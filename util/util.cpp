#include <sys/time.h>
#include <esp_timer.h>

int64_t getTimeus()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return (int64_t)t.tv_sec * 1000000L + (int64_t)t.tv_usec;
}

int64_t getMillis()
{
	int64_t timeSinceStartup = esp_timer_get_time();
	return timeSinceStartup/1000;
}

double timeDelta;
void calculateTimeDelta()
{
	static double timePrev = getTimeus(), timeNext = getTimeus();
	timePrev = timeNext;
	timeNext = getTimeus();
	timeDelta = (timeNext - timePrev)/1000000.f;
}
