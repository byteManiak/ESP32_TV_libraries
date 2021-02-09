#include <wifi/common.h>
#include <alloc.h>
#include <util.h>

#include <string.h>

#include <esp_log.h>

static const char *TAG = "wifi-common";

QueueHandle_t wifiQueueTx, wifiQueueRx;

void createWifiQueues()
{
	wifiQueueTx = xQueueCreate(8, sizeof(wifi_queue_message*));
	LOG_PTR(wifiQueueTx, "xQueueCreate:1");
	wifiQueueRx = xQueueCreate(32, sizeof(wifi_queue_message*));
	LOG_PTR(wifiQueueRx, "xQueueCreate:2");
}

BaseType_t sendWifiQueueData(QueueHandle_t &queue, uint8_t msg_flags, const char *msg_text)
{
	if (!queue) return pdFAIL;
	wifi_queue_message *txMessage = heap_caps_malloc_cast<wifi_queue_message>(MALLOC_CAP_PREFERRED);

	txMessage->msg_flags = msg_flags;
	if (msg_text) strlcpy(txMessage->msg_text, msg_text, 65);
	else txMessage->msg_text[0] = '\0';

	ESP_LOGI(TAG, "Sending %p through queue %p", txMessage, wifiQueueTx);
	return xQueueSend(queue, &txMessage, 10 / portTICK_PERIOD_MS);
}