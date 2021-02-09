#include <sound.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/ledc.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <util.h>

#define SOUND_TICK_PERIOD_US 83000
#define SOUND_TICKS_PER_NOTE 5

static esp_timer_handle_t soundTimer;

static uint32_t notes[2][192] = {
	{131, 147, 155, 196, 131, 147, 155, 196, 131, 147, 155, 196, 131, 147, 155, 196, 
	 131, 147, 155, 196, 131, 147, 155, 196, 131, 147, 155, 196, 131, 147, 155, 196, 
	 131, 147, 155, 196, 131, 147, 155, 196, 131, 147, 155, 196, 131, 147, 155, 196, 
	 131, 147, 155, 196, 131, 147, 155, 196, 131, 147, 155, 196, 131, 147, 155, 196, 
	 116, 131, 147, 175, 116, 131, 147, 175, 116, 131, 147, 175, 116, 131, 147, 175, 
	 116, 131, 147, 175, 116, 131, 147, 175, 116, 131, 147, 175, 116, 131, 147, 175, 
	 116, 131, 147, 175, 116, 131, 147, 175, 116, 131, 147, 175, 116, 131, 147, 175, 
	 116, 131, 147, 175, 116, 131, 147, 175, 116, 131, 147, 175, 116, 131, 147, 175, 
	 104, 116, 131, 155, 104, 116, 131, 155, 104, 116, 131, 155, 104, 116, 131, 155, 
	 104, 116, 131, 155, 104, 116, 131, 155, 104, 116, 131, 155, 104, 116, 131, 155, 
	 104, 116, 131, 155, 104, 116, 131, 155, 104, 116, 131, 155, 104, 116, 131, 155, 
	 104, 116, 131, 155, 104, 116, 131, 155, 104, 116, 131, 155, 104, 116, 131, 155},

	{131, 0,   0,   0,   131, 0,   0,   0,   131, 0,   0,   0,   131, 0,   131, 0,   
	 0,   0,   131, 0,   0,   0,   131, 0,   131, 0,   0,   0,   0,   0,   0,   0,
	 131, 0,   0,   0,   131, 0,   0,   0,   131, 0,   0,   0,   131, 0,   131, 0,   
	 0,   0,   131, 0,   0,   0,   131, 0,   131, 0,   0,   0,   0,   0,   0,   0,
	 131, 0,   0,   0,   131, 0,   0,   0,   131, 0,   0,   0,   131, 0,   131, 0,   
	 0,   0,   131, 0,   0,   0,   131, 0,   131, 0,   0,   0,   0,   0,   0,   0,
	 131, 0,   0,   0,   131, 0,   0,   0,   131, 0,   0,   0,   131, 0,   131, 0,   
	 0,   0,   131, 0,   0,   0,   131, 0,   131, 0,   0,   0,   0,   0,   0,   0,
	 131, 0,   0,   0,   131, 0,   0,   0,   131, 0,   0,   0,   131, 0,   131, 0,   
	 0,   0,   131, 0,   0,   0,   131, 0,   131, 0,   0,   0,   0,   0,   0,   0,
	 131, 0,   0,   0,   131, 0,   0,   0,   131, 0,   0,   0,   131, 0,   131, 0,   
	 0,   0,   131, 0,   0,   0,   131, 0,   131, 0,   0,   0,   0,   0,   0,   0}
};

static uint8_t currentNoteTick = 0;
static uint32_t i = 0;
static uint16_t duty = 0;

bool enableSound = true;

#define len(a) sizeof(a)/sizeof(a[0])
static void soundLoop(void *arg)
{
	currentNoteTick = getNextInt(currentNoteTick, SOUND_TICKS_PER_NOTE);
	if (currentNoteTick == 0)
	{
		setNote(0, duty%4096, notes[0][i%192]*2);
		setNote(1, 3072, notes[1][i%192]/2);
		i++;
	}
	duty++;
}

void initSound()
{
	ledc_timer_config_t timer1Config = {};
	timer1Config.speed_mode = LEDC_HIGH_SPEED_MODE;
	timer1Config.duty_resolution = LEDC_TIMER_12_BIT;
	timer1Config.freq_hz = 5000;
	timer1Config.timer_num = LEDC_TIMER_0;
	timer1Config.clk_cfg = LEDC_USE_APB_CLK;

	ledc_timer_config_t timer2Config = {};
	timer2Config.speed_mode = LEDC_HIGH_SPEED_MODE;
	timer2Config.duty_resolution = LEDC_TIMER_12_BIT;
	timer2Config.freq_hz = 5000;
	timer2Config.timer_num = LEDC_TIMER_1;
	timer2Config.clk_cfg = LEDC_USE_APB_CLK;

	ledc_timer_config(&timer1Config);
	ledc_timer_config(&timer2Config);

	ledc_channel_config_t sound1Config = {};
	sound1Config.gpio_num = 18;
	sound1Config.channel = LEDC_CHANNEL_0;
	sound1Config.speed_mode = LEDC_HIGH_SPEED_MODE;
	sound1Config.timer_sel = LEDC_TIMER_0;
	sound1Config.duty = 0;

	ledc_channel_config_t sound2Config = {};
	sound2Config.gpio_num = 19;
	sound2Config.channel = LEDC_CHANNEL_1;
	sound2Config.speed_mode = LEDC_HIGH_SPEED_MODE;
	sound2Config.timer_sel = LEDC_TIMER_1;
	sound2Config.duty = 0;

	ledc_fade_func_uninstall();

	ledc_channel_config(&sound1Config);
	ledc_channel_config(&sound2Config);

	ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
	ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, 110);

	esp_timer_create_args_t isrTimerConfig;
	isrTimerConfig.callback = soundLoop;
	isrTimerConfig.name = "Sound";
	esp_timer_create(&isrTimerConfig, &soundTimer);
	esp_timer_start_periodic(soundTimer, SOUND_TICK_PERIOD_US / SOUND_TICKS_PER_NOTE);
}

//#define QUIET

static uint32_t prevDuty[2] = {0, 0};
void setDuty(uint8_t channel, uint32_t duty)
{
	if (duty != prevDuty[channel])
	{
		ledc_set_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)(LEDC_CHANNEL_0+channel), duty);
		ledc_update_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)(LEDC_CHANNEL_0+channel));
		prevDuty[channel] = duty;
	}
}

static uint32_t prevFrequency[2] = {0, 0};
void setFrequency(uint8_t channel, uint32_t frequency)
{
	if (frequency != prevFrequency[channel])
	{
		ledc_set_freq(LEDC_HIGH_SPEED_MODE, (ledc_timer_t)(LEDC_TIMER_0+channel), frequency);
		prevFrequency[channel] = frequency;
	}
}

void setNote(uint8_t channel, uint32_t duty, uint32_t frequency)
{
#if defined(QUIET)
	duty = 5;
#endif
	if (frequency == 0 || !enableSound)
	{
		setDuty(channel, 0);
		return;
	}
	setDuty(channel, duty);
	setFrequency(channel, frequency);
}