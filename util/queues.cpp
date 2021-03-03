#include <util/queues.h>

#include <string.h>

#include <memory/alloc.h>
#include <util/log.h>

static const char *TAG = "queue";

QueueHandle_t wifiQueueTx = NULL;
QueueHandle_t wifiQueueRx = NULL;

QueueHandle_t httpQueueRx = NULL;

QueueHandle_t otaQueueTx = NULL;

QueueHandle_t radioQueueTx = NULL;

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
