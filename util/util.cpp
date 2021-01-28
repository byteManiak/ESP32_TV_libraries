#include <sys/time.h>

int64_t getTimeus()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return (int64_t)t.tv_sec * 1000000L + (int64_t)t.tv_usec;
}

double timeDelta;
void calculateTimeDelta()
{
	static double timePrev = getTimeus(), timeNext = getTimeus();
	timePrev = timeNext;
	timeNext = getTimeus();
	timeDelta = (timeNext - timePrev)/1000000.f;
}
