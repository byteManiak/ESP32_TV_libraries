/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

#include <sound.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2s.h>
#include <driver/ledc.h>
#include <esp_timer.h>
#include <board.h>
#include <audio_hal.h>
#include <audio_element.h>
#include <audio_pipeline.h>

#if defined(CONFIG_AUDIO_ENABLE_HTTP)
#include <http_stream.h>
#endif
//#if defined(CONFIG_AUDIO_ENABLE_FS)
#include <spiffs_stream.h>
//#endif

#if defined(CONFIG_AUDIO_ENABLE_MP3)
#include <mp3_decoder.h>
#endif
#if defined(CONFIG_AUDIO_ENABLE_OGG)
#include <ogg_decoder.h>
#endif

#include <raw_stream.h>
#include <i2s_stream.h>

#include <downmix.h>

#include <memory/alloc.h>
#include <util/log.h>
#include <util/numeric.h>
#include <util/queues.h>

static const char *TAG = "sound";

QueueHandle_t audioQueue;

static audio_board_handle_t boardHandle;
static audio_pipeline_handle_t basePipeline = NULL, secondPipeline = NULL, downmixPipeline = NULL;
static audio_element_handle_t baseStreamer = NULL, baseDecoder = NULL, baseOut = NULL;
static audio_element_handle_t secondStreamer = NULL, secondDecoder = NULL, secondOut = NULL;
static audio_element_handle_t downmixer = NULL;
static audio_element_handle_t i2sDac = NULL;

static audio_event_iface_handle_t listener = NULL;
static audio_event_iface_handle_t sender = NULL;

const char *basePipelineStages[] = {"baseGet", "baseCvt", "baseOut"};
const char *secondPipelineStages[] = {"secondGet", "secondCvt", "secondOut"};
const char *downmixPipelineStages[] = {"downmix", "i2s"};

bool loopMusic = false;
bool mutedSounds = false;

#if defined(CONFIG_AUDIO_ENABLE_HTTP)
static int playlistHandler(http_stream_event_msg_t *message)
{
	switch(message->event_id)
	{
		case HTTP_STREAM_FINISH_TRACK: return http_stream_next_track(message->el);
		case HTTP_STREAM_FINISH_PLAYLIST: return http_stream_fetch_again(message->el);
		default: return ESP_OK;
	}
}

static esp_err_t initHttpStream()
{
	esp_err_t error = ESP_FAIL;

	http_stream_cfg_t httpStreamConfig = {};
	httpStreamConfig.type = AUDIO_STREAM_READER;
	httpStreamConfig.task_core = 1;
	httpStreamConfig.task_prio = HTTP_TASK_PRIORITY;
	httpStreamConfig.task_stack = 5120;
	httpStreamConfig.out_rb_size = 8192;
	httpStreamConfig.stack_in_ext = true;
	httpStreamConfig.enable_playlist_parser = true;
	httpStreamConfig.event_handle = playlistHandler;
	streamer = http_stream_init(&httpStreamConfig);
	LOG_FN_GOTO_IF_NULL(streamer, "http_stream_init", httpInitFail);

	error = audio_pipeline_register(pipeline, streamer, "get");
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_register:http", pipelineFail);

	return ESP_OK;

pipelineFail:
	audio_element_deinit(streamer);
httpInitFail:
	return error;
}
#endif

#if defined(CONFIG_AUDIO_ENABLE_FS)
static esp_err_t initFsStream()
{
	esp_err_t error = ESP_FAIL;

	spiffs_stream_cfg_t spiffsStreamConfig = {};
	spiffsStreamConfig.type = AUDIO_STREAM_READER;
	spiffsStreamConfig.task_core = 1;
	spiffsStreamConfig.task_prio = HTTP_TASK_PRIORITY;
	spiffsStreamConfig.task_stack = 5120;
	spiffsStreamConfig.buf_sz = 2048;
	spiffsStreamConfig.out_rb_size = 8192;

	baseStreamer = spiffs_stream_init(&spiffsStreamConfig);
	LOG_FN_GOTO_IF_NULL(baseStreamer, "spiffs_stream_init:1", baseSpiffsInitFail);
	error = audio_pipeline_register(basePipeline, baseStreamer, "baseGet");
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_register:fs:1", baseSpiffsRegFail);

	secondStreamer = spiffs_stream_init(&spiffsStreamConfig);
	LOG_FN_GOTO_IF_NULL(secondStreamer, "spiffs_stream_init:2", secondSpiffsInitFail);
	error = audio_pipeline_register(secondPipeline, secondStreamer, "secondGet");
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_register:fs:2", secondSpiffsRegFail);

	return ESP_OK;

secondSpiffsRegFail:
	audio_element_deinit(secondStreamer);
secondSpiffsInitFail:
	audio_pipeline_unregister(basePipeline, baseStreamer);
baseSpiffsRegFail:
	audio_element_deinit(baseStreamer);
baseSpiffsInitFail:
	return error;
}
#endif

#if defined(CONFIG_AUDIO_ENABLE_MP3)
static esp_err_t initMp3Decoder()
{
	esp_err_t error = ESP_FAIL;

	mp3_decoder_cfg_t mp3DecoderConfig = {};
	mp3DecoderConfig.task_core = 1;
	mp3DecoderConfig.task_prio = MP3_TASK_PRIORITY;
	mp3DecoderConfig.task_stack = 3072;
	mp3DecoderConfig.out_rb_size = 4096;
	mp3DecoderConfig.stack_in_ext = true;
	decoder = mp3_decoder_init(&mp3DecoderConfig);
	LOG_FN_GOTO_IF_NULL(decoder, "mp3_decoder_init", mp3InitFail);

	error = audio_pipeline_register(pipeline, decoder, "cvt");
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_register:mp3", pipelineFail);

	return ESP_OK;

pipelineFail:
	audio_element_deinit(decoder);
mp3InitFail:
	return error;
}
#endif

#if defined(CONFIG_AUDIO_ENABLE_OGG)
static esp_err_t initOggDecoder()
{
	esp_err_t error = ESP_FAIL;

	ogg_decoder_cfg_t oggDecoderConfig = {};
	oggDecoderConfig.task_core = 1;
	oggDecoderConfig.task_prio = MP3_TASK_PRIORITY;
	oggDecoderConfig.task_stack = 3072;
	oggDecoderConfig.out_rb_size = 4096;
	oggDecoderConfig.stack_in_ext = true;

	baseDecoder = ogg_decoder_init(&oggDecoderConfig);
	LOG_FN_GOTO_IF_NULL(baseDecoder, "ogg_decoder_init:1", baseOggInitFail);
	error = audio_pipeline_register(basePipeline, baseDecoder, "baseCvt");
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_register:ogg:1", baseOggRegFail);

	secondDecoder = ogg_decoder_init(&oggDecoderConfig);
	LOG_FN_GOTO_IF_NULL(secondDecoder, "ogg_decoder_init:2", secondOggInitFail);
	error = audio_pipeline_register(secondPipeline, secondDecoder, "secondCvt");
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_register:ogg:2", secondOggRegFail);

	return ESP_OK;

secondOggRegFail:
	audio_element_deinit(secondDecoder);
secondOggInitFail:
	audio_pipeline_unregister(basePipeline, baseDecoder);
baseOggRegFail:
	audio_element_deinit(baseDecoder);
baseOggInitFail:
	return error;
}
#endif

static esp_err_t initRawOut()
{
	esp_err_t error = ESP_FAIL;

	raw_stream_cfg_t rawConfig = {};
	rawConfig.type = AUDIO_STREAM_WRITER;
	rawConfig.out_rb_size = 6144;

	baseOut = raw_stream_init(&rawConfig);
	LOG_FN_GOTO_IF_NULL(baseOut, "raw_stream_init:1", baseOutInitFail);
	error = audio_pipeline_register(basePipeline, baseOut, "baseOut");
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_register:out:1", baseOutRegFail);

	secondOut = raw_stream_init(&rawConfig);
	LOG_FN_GOTO_IF_NULL(secondOut, "raw_stream_init:2", secondOutInitFail);
	error = audio_pipeline_register(secondPipeline, secondOut, "secondOut");
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_register:out:2", secondOutRegFail);

	return ESP_OK;

secondOutRegFail:
	audio_element_deinit(secondOut);
secondOutInitFail:
	audio_pipeline_unregister(basePipeline, baseOut);
baseOutRegFail:
	audio_element_deinit(baseOut);
baseOutInitFail:
	return error;
}

static esp_err_t initDownmixer()
{
	downmix_cfg_t downmixConfig = {};
	downmixConfig.downmix_info.mode = ESP_DOWNMIX_WORK_MODE_SWITCH_ON;
	downmixConfig.downmix_info.out_ctx = ESP_DOWNMIX_OUT_CTX_LEFT_RIGHT;
	downmixConfig.downmix_info.output_type = ESP_DOWNMIX_OUTPUT_TYPE_ONE_CHANNEL;
	downmixConfig.downmix_info.source_num = 2;
	downmixConfig.max_sample = 256;
	downmixConfig.task_core = 1;
	downmixConfig.task_prio = I2S_TASK_PRIORITY;
	downmixConfig.task_stack = 6144;
	downmixConfig.out_rb_size = 6144;
	downmixer = downmix_init(&downmixConfig);
	if (!downmixer) return ESP_FAIL;

	esp_downmix_input_info_t downmixInfo[2] = {};
	esp_downmix_input_info_t downmixParams = {};
	downmixParams.bits_num = 16;
	downmixParams.channel = 1;
	downmixParams.samplerate = 22050;
	downmixParams.transit_time = 10;
	downmixInfo[0] = downmixParams;
	downmixInfo[1] = downmixParams;

	source_info_init(downmixer, downmixInfo);

	downmix_set_input_rb_timeout(downmixer, 0, 0);
    downmix_set_input_rb_timeout(downmixer, 0, 1);

	return ESP_OK;
}

static esp_err_t initI2S()
{
	i2s_stream_cfg_t i2sDacConfig = {};
	i2sDacConfig.type = AUDIO_STREAM_WRITER;
	i2sDacConfig.i2s_port = I2S_NUM_0;
	i2sDacConfig.out_rb_size = 4096;
	i2sDacConfig.stack_in_ext = true;
	i2sDacConfig.task_core = 1;
	i2sDacConfig.task_prio = I2S_TASK_PRIORITY;
	i2sDacConfig.task_stack = 3072;
	i2sDacConfig.i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
	i2sDacConfig.i2s_config.sample_rate = 22050;
	i2sDacConfig.i2s_config.mode = (i2s_mode_t)(I2S_MODE_DAC_BUILT_IN | I2S_MODE_MASTER | I2S_MODE_TX);
	i2sDacConfig.i2s_config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
	i2sDacConfig.i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
	i2sDacConfig.i2s_config.dma_buf_count = 2;
	i2sDacConfig.i2s_config.dma_buf_len = 1024;
	i2sDacConfig.i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
	i2sDacConfig.i2s_config.tx_desc_auto_clear = true;
	i2sDac = i2s_stream_init(&i2sDacConfig);
	if (!i2sDac) return ESP_FAIL;

	return ESP_OK;
}

static void audioTask(void *arg);

static esp_err_t initPipeline(StreamType streamType, DecoderType decoderType)
{
	esp_err_t error = ESP_OK;
	ringbuf_handle_t baseOutBuf, secondOutBuf;
	audio_event_iface_cfg_t eventConfig = AUDIO_EVENT_IFACE_DEFAULT_CFG();

	audio_pipeline_cfg_t pipelineCfg = { .rb_size=6144 };
	basePipeline = audio_pipeline_init(&pipelineCfg);
	LOG_FN_GOTO_IF_NULL(basePipeline, "audio_pipeline_init:1", basePipelineFail);
	secondPipeline = audio_pipeline_init(&pipelineCfg);
	LOG_FN_GOTO_IF_NULL(secondPipeline, "audio_pipeline_init:2", secondPipelineFail);
	downmixPipeline = audio_pipeline_init(&pipelineCfg);
	LOG_FN_GOTO_IF_NULL(downmixPipeline, "audio_pipeline_init:3", downmixPipelineFail);

	switch (streamType)
	{
#if defined(CONFIG_AUDIO_ENABLE_HTTP)
		case STREAM_TYPE_HTTP: error = initHttpStream(); break;
#endif
#if defined(CONFIG_AUDIO_ENABLE_FS)
		case STREAM_TYPE_FS: error = initFsStream(); break;
#endif
		default: error = ESP_ERR_INVALID_ARG; break;
	}
	LOG_FN_GOTO_IF_ERR(error, "initStream", streamFail);

	switch (decoderType)
	{
#if defined(CONFIG_AUDIO_ENABLE_MP3)
		case DECODER_TYPE_MP3: error = initMp3Decoder(); break;
#endif
#if defined(CONFIG_AUDIO_ENABLE_OGG)
		case DECODER_TYPE_OGG: error = initOggDecoder(); break;
#endif
		default: error = ESP_ERR_INVALID_ARG; break;
	}
	LOG_FN_GOTO_IF_ERR(error, "initDecoder", decoderFail);

	error = initRawOut();
	LOG_FN_GOTO_IF_ERR(error, "initRawOut", rawOutFail);

	error = audio_pipeline_link(basePipeline, basePipelineStages, 3);
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_link:1", basePipelineLinkFail);
	error = audio_pipeline_link(secondPipeline, secondPipelineStages, 3);
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_link:2", secondPipelineLinkFail);

	error = initDownmixer();
	LOG_FN_GOTO_IF_ERR(error, "initDownmixer", downmixerFail);

	baseOutBuf = audio_element_get_input_ringbuf(baseOut);
	downmix_set_input_rb(downmixer, baseOutBuf, 0);
	secondOutBuf = audio_element_get_input_ringbuf(secondOut);
	downmix_set_input_rb(downmixer, secondOutBuf, 1);

	error = initI2S();
	LOG_FN_GOTO_IF_ERR(error, "initI2S", i2sDacFail);

	error = audio_pipeline_register(downmixPipeline, downmixer, "downmix");
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_register:downmix", downmixRegFail);
	error = audio_pipeline_register(downmixPipeline, i2sDac, "i2s");
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_register:i2s", i2sRegFail);

	error = audio_pipeline_link(downmixPipeline, downmixPipelineStages, 2);
	LOG_FN_GOTO_IF_ERR(error, "audio_pipeline_link:3", pipelineLinkFail);

	sender = audio_event_iface_init(&eventConfig);
	LOG_FN_GOTO_IF_NULL(sender, "audio_event_iface_init:1", senderInitFail);
	listener = audio_event_iface_init(&eventConfig);
	LOG_FN_GOTO_IF_NULL(listener, "audio_event_iface_init:2", listenerInitFail);

	audio_event_iface_set_listener(sender, listener);
	audio_pipeline_set_listener(basePipeline, listener);
	audio_pipeline_set_listener(secondPipeline, listener);
	audio_pipeline_set_listener(downmixPipeline, listener);

	return ESP_OK;

listenerInitFail:
	audio_event_iface_destroy(sender);
senderInitFail:
	audio_pipeline_unlink(downmixPipeline);
pipelineLinkFail:
	audio_pipeline_unregister(downmixPipeline, i2sDac);
i2sRegFail:
	audio_pipeline_unregister(downmixPipeline, downmixer);
downmixRegFail:
	audio_element_deinit(i2sDac);
i2sDacFail:
	audio_element_deinit(downmixer);
downmixerFail:
	audio_pipeline_unlink(secondPipeline);
secondPipelineLinkFail:
	audio_pipeline_unlink(basePipeline);
basePipelineLinkFail:
	audio_pipeline_unregister(basePipeline, baseOut);
	audio_element_deinit(baseOut);
	audio_pipeline_unregister(secondPipeline, secondOut);
	audio_element_deinit(secondOut);
rawOutFail:
	audio_pipeline_unregister(basePipeline, baseDecoder);
	audio_element_deinit(baseDecoder);
	audio_pipeline_unregister(secondPipeline, secondDecoder);
	audio_element_deinit(secondDecoder);
decoderFail:
	audio_pipeline_unregister(basePipeline, baseStreamer);
	audio_element_deinit(baseStreamer);
	audio_pipeline_unregister(secondPipeline, secondStreamer);
	audio_element_deinit(secondStreamer);
streamFail:
	audio_pipeline_deinit(downmixPipeline);
	downmixPipeline = NULL;
downmixPipelineFail:
	audio_pipeline_deinit(secondPipeline);
secondPipelineFail:
	audio_pipeline_deinit(basePipeline);
basePipelineFail:
	return error;
}

#define AUDIO_EVENT_SOURCE_VGA_APP 1

#define AUDIO_EVENT_PLAY_MUSIC_LOOP 1
#define AUDIO_EVENT_PLAY_MUSIC_ONCE 2
#define AUDIO_EVENT_PAUSE_MUSIC 3
#define AUDIO_EVENT_RESUME_MUSIC 4
#define AUDIO_EVENT_PLAY_SOUND 5
#define AUDIO_EVENT_MUTE_SOUND 6
#define AUDIO_EVENT_RESUME_SOUND 7

void playMusic(const char *path, bool loop)
{
	audio_event_iface_msg_t msg = {};
	msg.source_type = AUDIO_EVENT_SOURCE_VGA_APP;
	msg.data = strdup(path);
	msg.data_len = strlen(path);
	msg.cmd = loop ? AUDIO_EVENT_PLAY_MUSIC_LOOP : AUDIO_EVENT_PLAY_MUSIC_ONCE;
	msg.need_free_data = true;
	audio_event_iface_sendout(sender, &msg);
}

void pauseMusic()
{
	audio_event_iface_msg_t msg = {};
	msg.source_type = AUDIO_EVENT_SOURCE_VGA_APP;
	msg.cmd = AUDIO_EVENT_PAUSE_MUSIC;
	audio_event_iface_sendout(sender, &msg);
}

void resumeMusic()
{
	audio_event_iface_msg_t msg = {};
	msg.source_type = AUDIO_EVENT_SOURCE_VGA_APP;
	msg.cmd = AUDIO_EVENT_RESUME_MUSIC;
	audio_event_iface_sendout(sender, &msg);
}

void muteMusic(bool mute)
{
	if (mute) pauseMusic();
	else resumeMusic();
}

void playSound(const char *path)
{
	audio_event_iface_msg_t msg = {};
	msg.source_type = AUDIO_EVENT_SOURCE_VGA_APP;
	msg.data = strdup(path);
	msg.data_len = strlen(path);
	msg.cmd = AUDIO_EVENT_PLAY_SOUND;
	msg.need_free_data = true;
	audio_event_iface_sendout(sender, &msg);
}

void muteSound(bool mute)
{
	audio_event_iface_msg_t msg = {};
	msg.source_type = AUDIO_EVENT_SOURCE_VGA_APP;
	msg.cmd = mute ? AUDIO_EVENT_MUTE_SOUND : AUDIO_EVENT_RESUME_SOUND;
	audio_event_iface_sendout(sender, &msg);
}

static void audioTask(void *arg)
{
	for (;;)
	{
		audio_event_iface_msg_t msg;
		esp_err_t error = audio_event_iface_listen(listener, &msg, portMAX_DELAY);

		if (error == ESP_OK)
		{
			//audio_element_state_t downmixState = audio_element_get_state(downmixer);
			//audio_element_state_t i2sState = audio_element_get_state(i2sDac);
			//audio_element_state_t baseState = audio_element_get_state(baseOut);
			//audio_element_state_t secondState = audio_element_get_state(secondOut);
			//ESP_LOGE("sound", "%u %u  %u  %u", downmixState, i2sState, baseState, secondState);

			ESP_LOGE("sound", "%p %p", baseDecoder, secondDecoder);

			if (loopMusic && msg.source == (void*)baseDecoder && msg.cmd == AEL_MSG_CMD_REPORT_STATUS && (int)msg.data == AEL_STATUS_STATE_FINISHED)
			{
				audio_pipeline_stop(basePipeline);
				audio_pipeline_wait_for_stop(basePipeline);

				audio_pipeline_reset_elements(basePipeline);
				audio_pipeline_reset_ringbuffer(basePipeline);

				audio_pipeline_run(basePipeline);
			}
			//if (msg.source == (void*)downmixer && msg.cmd == AEL_MSG_CMD_REPORT_STATUS && (int)msg.data == AEL_STATUS_STATE_FINISHED)
			//{
			//	audio_element_reset_state(downmixer);
			//}
			else if (msg.source_type == AUDIO_EVENT_SOURCE_VGA_APP)
			{
				switch(msg.cmd)
				{
					case AUDIO_EVENT_PLAY_MUSIC_ONCE:
					case AUDIO_EVENT_PLAY_MUSIC_LOOP:
					{
						if (msg.cmd == AUDIO_EVENT_PLAY_MUSIC_LOOP) loopMusic = true;
						else loopMusic = false;

						audio_pipeline_stop(basePipeline);
						audio_pipeline_wait_for_stop(basePipeline);

						audio_pipeline_reset_elements(basePipeline);
						audio_pipeline_reset_ringbuffer(basePipeline);

						audio_element_set_uri(baseStreamer, (char*)msg.data);

						/*if (audio_element_get_state(downmixer) == AEL_STATE_FINISHED)
						{
							audio_element_reset_output_ringbuf(i2sDac);
							audio_pipeline_reset_elements(downmixPipeline);
							audio_pipeline_reset_ringbuffer(downmixPipeline);
							audio_pipeline_change_state(downmixPipeline, AEL_STATE_INIT);
						}*/
						audio_pipeline_run(downmixPipeline);
						audio_pipeline_run(basePipeline);

						break;
					}

					case AUDIO_EVENT_PLAY_SOUND:
					{
						if (mutedSounds) break;

						audio_pipeline_stop(secondPipeline);
						audio_pipeline_wait_for_stop(secondPipeline);

						audio_pipeline_reset_elements(secondPipeline);
						audio_pipeline_reset_ringbuffer(secondPipeline);

						audio_element_set_uri(secondStreamer, (char*)msg.data);

						/*if (audio_element_get_state(downmixer) == AEL_STATE_FINISHED)
						{
							audio_element_reset_output_ringbuf(i2sDac);
							audio_pipeline_reset_elements(downmixPipeline);
							audio_pipeline_reset_ringbuffer(downmixPipeline);
							audio_pipeline_change_state(downmixPipeline, AEL_STATE_INIT);
						}*/

						audio_pipeline_run(secondPipeline);

						break;
					}

					case AUDIO_EVENT_PAUSE_MUSIC:
					{
						audio_pipeline_pause(basePipeline);
						break;
					}

					case AUDIO_EVENT_RESUME_MUSIC:
					{
						audio_pipeline_resume(basePipeline);
						break;
					}

					case AUDIO_EVENT_MUTE_SOUND:
					{
						mutedSounds = true;
						downmix_set_work_mode(downmixer, ESP_DOWNMIX_WORK_MODE_BYPASS);
						break;
					}

					case AUDIO_EVENT_RESUME_SOUND:
					{
						mutedSounds = false;
						downmix_set_work_mode(downmixer, ESP_DOWNMIX_WORK_MODE_SWITCH_ON);
						break;
					}

					default:
					{
						break;
					}
				}
			}
		}

		if (msg.need_free_data) heap_caps_free(msg.data);
	}
}

void initSound(StreamType streamType, DecoderType decoderType)
{
	boardHandle = audio_board_init();
	audio_hal_ctrl_codec(boardHandle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);
	audioQueue = xQueueCreate(4, sizeof(queue_message*));

	initPipeline(streamType, decoderType);

	audio_pipeline_run(downmixPipeline);

	xTaskCreatePinnedToCore(audioTask, "audioTask", 3072, NULL, WIFI_TASK_PRIORITY, NULL, 1);
}

void initBuzzers()
{
	ledc_timer_config_t pwm1Config = {};
	pwm1Config.speed_mode = LEDC_HIGH_SPEED_MODE;
	pwm1Config.duty_resolution = LEDC_TIMER_12_BIT;
	pwm1Config.clk_cfg = LEDC_USE_APB_CLK;
	pwm1Config.freq_hz = 5000;
	pwm1Config.timer_num = LEDC_TIMER_0;

	ledc_timer_config_t pwm2Config = {};
	pwm2Config.speed_mode = LEDC_HIGH_SPEED_MODE;
	pwm2Config.duty_resolution = LEDC_TIMER_12_BIT;
	pwm2Config.clk_cfg = LEDC_USE_APB_CLK;
	pwm2Config.freq_hz = 5000;
	pwm2Config.timer_num = LEDC_TIMER_1;

	ledc_timer_config(&pwm1Config);
	ledc_timer_config(&pwm2Config);

	ledc_channel_config_t channel1Config = {};
	channel1Config.channel = LEDC_CHANNEL_0;
	channel1Config.gpio_num = 18; //25;
	channel1Config.speed_mode = LEDC_HIGH_SPEED_MODE;
	channel1Config.timer_sel = LEDC_TIMER_0;
	channel1Config.duty = 0;

	ledc_channel_config_t channel2Config = {};
	channel2Config.channel = LEDC_CHANNEL_1;
	channel2Config.gpio_num = 19; //26;
	channel2Config.speed_mode = LEDC_HIGH_SPEED_MODE;
	channel2Config.timer_sel = LEDC_TIMER_1;
	channel2Config.duty = 0;

	// In case a PWM driver was already installed
	ledc_fade_func_uninstall();

	ledc_channel_config(&channel1Config);
	ledc_channel_config(&channel2Config);
}

static unsigned short frequencyTable[] =
{
	31, 33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62, 65, 69, 73, 78, 82, 87, 93, 98, 104, 110, 117, 123,
	131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392, 415,
	440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988, 1047, 1109, 1175, 1245, 1319,
	1397, 1480, 1568, 1661, 1760, 1865, 1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520,
	3729, 3951, 4186, 4435, 4699, 4978
};

static BuzzerMusic *musicHandle;

static void soundLoop(void*);

static esp_timer_handle_t soundLoopHandle = NULL;

void playBuzzerMusic(BuzzerMusic *music)
{
	// No point playing an empty piece of music
	if (!music || music->data_length == 0) return;

	musicHandle = music;

	// Replace the timer interrupt if it existed already
	if (soundLoopHandle) esp_timer_delete(soundLoopHandle);

	esp_timer_create_args_t isrTimerConfig = {};
	isrTimerConfig.callback = soundLoop;
	isrTimerConfig.name = "BuzzerMusic";

	// Create a timer with the tick rate specified in the music piece
	esp_timer_create(&isrTimerConfig, &soundLoopHandle);
	esp_timer_start_periodic(soundLoopHandle, musicHandle->tick_period);
	LOG_INFO("m");
}

static uint8_t currentTick = 0;
static uint32_t currentIndex = 0;
static BuzzerInstrument *leftChannelPtr, *rightChannelPtr;

static void soundLoop(void*)
{
	// If the tick counter has just been reset, advance the note pointer and play the corresponding next note
	if (currentTick == 0)
	{
		leftChannelPtr = &musicHandle->left_ch_data[currentIndex];
		rightChannelPtr = &musicHandle->right_ch_data[currentIndex];

		if (leftChannelPtr->note > 0)
		{
			// Prepare the frequency and duty cycle to apply to the left channel buzzer
			ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, frequencyTable[leftChannelPtr->note]);

			if (leftChannelPtr->duty > 0) ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, leftChannelPtr->duty);
			// If no duty has been specified, apply 50% duty
			else ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 2048);
		}
		// If the note is NONE, don't play any sound
		else ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);

		if (rightChannelPtr->note > 0)
		{
			// Prepare the frequency and duty cycle to apply to the right channel buzzer
			ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_1, frequencyTable[rightChannelPtr->note]);

			if (rightChannelPtr->duty > 0) ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, rightChannelPtr->duty);
			// If no duty has been specified, apply 50% duty
			else ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 2048);
		}
		// If the note is NONE, don't play any sound
		else ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 0);

		// Call ledc_update_duty to actually write the new PWM duty to the buzzers.
		ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
		ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
	}

	currentTick = getNextInt(currentTick, musicHandle->ticks_per_note);
	if (currentTick == 0) currentIndex = getNextInt(currentIndex, musicHandle->data_length);
}
