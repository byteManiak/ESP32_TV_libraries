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
#include <http_stream.h>
#include <mp3_decoder.h>
#include <i2s_stream.h>

#include <memory/alloc.h>
#include <util/log.h>
#include <util/numeric.h>
#include <util/queues.h>

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
