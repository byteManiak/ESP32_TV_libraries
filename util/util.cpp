#include <util.h>

#include <string.h>
#include <sys/time.h>
#include <esp_timer.h>

#include <alloc.h>

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
	// Scale the scroll speed with the distance for a smooth animation
	double distance = abs(from-to);
	double scrollSpeed = distance * timeDelta * lerpFactor;

	// Move radial menu towards the new destination
	if (from > to) from -= scrollSpeed;
	else from += scrollSpeed;
}

esp_err_t sendQueueData(QueueHandle_t &queue, uint8_t msg_flags, const char *msg_text, uint32_t delay)
{
	if (!queue) return ESP_ERR_INVALID_ARG;

	queue_message *txMessage = heap_caps_malloc_cast<queue_message>(MALLOC_CAP_PREFERRED);
	if (!txMessage)
	{
		LOG_PTR(txMessage, "heap_caps_malloc_cast");
		return ESP_ERR_NO_MEM;
	}

	txMessage->msg_flags = msg_flags;
	if (msg_text) strlcpy(txMessage->msg_text, msg_text, 256);
	else txMessage->msg_text[0] = '\0';

	LOG_INFO("Sending %p through queue %p", txMessage, queue);
	BaseType_t ret = xQueueSend(queue, &txMessage, delay);
	return ret == pdPASS ? ESP_OK : ESP_ERR_TIMEOUT;
}