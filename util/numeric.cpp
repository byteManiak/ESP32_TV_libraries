#include <util/numeric.h>

#include <string.h>
#include <sys/time.h>
#include <esp_timer.h>

#include <memory/alloc.h>

static const char *TAG = "util";

int64_t getMicros()
{
	return esp_timer_get_time();
}

int64_t getMillis()
{
	int64_t timeSinceStartup = esp_timer_get_time();
	return timeSinceStartup/1000;
}

uint32_t getNextInt(uint32_t s, uint32_t max)
{
	return (s < max-1) ? s+1 : 0;
}

uint32_t getPrevInt(uint32_t s, uint32_t max)
{
	return (s > 0) ? s-1 : max-1;
}

double timeDelta;
void calculateTimeDelta()
{
	static double timePrev = getMicros(), timeNext = getMicros();
	timePrev = timeNext;
	timeNext = getMicros();
	timeDelta = (timeNext - timePrev)/1000000.f;
}

static constexpr double lerpFactor = 3;
void smoothLerp(double &from, double &to)
{
	// Scale the lerp speed with the distance for a smooth animation
	double distance = abs(from-to);
	double lerpSpeed = distance * timeDelta * lerpFactor;

	// Move radial menu towards the new destination
	if (from > to) from -= lerpSpeed;
	else from += lerpSpeed;
}