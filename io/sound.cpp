#include <sound.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2s.h>
#include <board.h>
#include <audio_hal.h>
#include <audio_element.h>
#include <audio_pipeline.h>
#include <http_stream.h>
#include <mp3_decoder.h>
#include <i2s_stream.h>

#include <alloc.h>
#include <util.h>

static const char *TAG = "sound";

QueueHandle_t audioQueue;

static audio_board_handle_t boardHandle;
static audio_pipeline_handle_t pipeline;
static audio_element_handle_t httpStream, mp3Decoder, i2sDac;

static int playlistHandler(http_stream_event_msg_t *message)
{
	switch(message->event_id)
	{
		case HTTP_STREAM_FINISH_TRACK: return http_stream_next_track(message->el);
		case HTTP_STREAM_FINISH_PLAYLIST: return http_stream_fetch_again(message->el);
		default: return ESP_OK;
	}
}

static esp_err_t initPipeline()
{
	esp_err_t error = ESP_OK;

	const char *pipelineStages[] = {"get", "cvt", "out"};

	http_stream_cfg_t httpStreamConfig = {};
	mp3_decoder_cfg_t mp3DecoderConfig = {};
	i2s_stream_cfg_t i2sDacConfig = {};

	audio_pipeline_cfg_t pipelineCfg = { DEFAULT_PIPELINE_RINGBUF_SIZE };
	pipeline = audio_pipeline_init(&pipelineCfg);

	httpStreamConfig.type = AUDIO_STREAM_READER;
	httpStreamConfig.task_core = 1;
	httpStreamConfig.task_prio = HTTP_TASK_PRIORITY;
	httpStreamConfig.task_stack = 5120;
	httpStreamConfig.out_rb_size = 8192;
	httpStreamConfig.stack_in_ext = true;
	httpStreamConfig.enable_playlist_parser = true;
	httpStreamConfig.event_handle = playlistHandler;
	httpStream = http_stream_init(&httpStreamConfig);
	LOG_FN_GOTO_IF_NULL(httpStream, "http_stream_init", httpStreamFail);

	mp3DecoderConfig.task_core = 1;
	mp3DecoderConfig.task_prio = MP3_TASK_PRIORITY;
	mp3DecoderConfig.stack_in_ext = true;
	mp3DecoderConfig.task_stack = 3072;
	mp3DecoderConfig.out_rb_size = 4096;
	mp3Decoder = mp3_decoder_init(&mp3DecoderConfig);
	LOG_FN_GOTO_IF_NULL(mp3Decoder, "mp3_decoder_init", mp3DecoderFail);

	i2sDacConfig.type = AUDIO_STREAM_WRITER;
	i2sDacConfig.i2s_port = I2S_NUM_0;
	i2sDacConfig.out_rb_size = 4096;
	i2sDacConfig.stack_in_ext = true;
	i2sDacConfig.task_core = 1;
	i2sDacConfig.task_prio = I2S_TASK_PRIORITY;
	i2sDacConfig.task_stack = 3072;
	i2sDacConfig.i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
	i2sDacConfig.i2s_config.sample_rate = 44100;
	i2sDacConfig.i2s_config.mode = (i2s_mode_t)(I2S_MODE_DAC_BUILT_IN | I2S_MODE_MASTER | I2S_MODE_TX);
	i2sDacConfig.i2s_config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
	i2sDacConfig.i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
	i2sDacConfig.i2s_config.dma_buf_count = 2;
	i2sDacConfig.i2s_config.dma_buf_len = 1024;
	i2sDacConfig.i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
	i2sDacConfig.i2s_config.tx_desc_auto_clear = true;
	i2sDac = i2s_stream_init(&i2sDacConfig);
	LOG_FN_GOTO_IF_NULL(i2sDac, "is2_stream_init", i2sDacFail);

	error = audio_pipeline_register(pipeline, httpStream, "get");
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_register:1", registerHttpFail);
	error = audio_pipeline_register(pipeline, mp3Decoder, "cvt");
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_register:2", registerMp3Fail);
	error = audio_pipeline_register(pipeline, i2sDac, "out");
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_register:3", registerI2sFail);

	error = audio_pipeline_link(pipeline, pipelineStages, 3);
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_link", pipelineLinkFail);

success:
	goto exit;

pipelineLinkFail:
	audio_pipeline_unregister(pipeline, i2sDac);
registerI2sFail:
	audio_pipeline_unregister(pipeline, mp3Decoder);
registerMp3Fail:
	audio_pipeline_unregister(pipeline, httpStream);
registerHttpFail:
	audio_element_deinit(i2sDac);
i2sDacFail:
	audio_element_deinit(mp3Decoder);
mp3DecoderFail:
	audio_element_deinit(httpStream);
httpStreamFail:
	audio_pipeline_deinit(pipeline);

exit:
	return error;
}

void audioDispatchTask(void *arg)
{
	char *streamURL = (char*)arg;
	if (!pipeline) initPipeline();

	audio_element_pause(httpStream);
	audio_element_pause(mp3Decoder);
	audio_pipeline_stop(pipeline);
	audio_pipeline_wait_for_stop(pipeline);
	audio_pipeline_terminate(pipeline);

	audio_element_set_uri(httpStream, streamURL);
	heap_caps_free(streamURL);

	audio_pipeline_reset_ringbuffer(pipeline);
	audio_pipeline_reset_elements(pipeline);

	audio_pipeline_run(pipeline);
exit:
	vTaskDelete(NULL);
}

void initSound()
{
	boardHandle = audio_board_init();
	audio_hal_ctrl_codec(boardHandle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);
	audioQueue = xQueueCreate(4, sizeof(queue_message*));
}
