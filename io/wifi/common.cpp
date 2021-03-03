#include <wifi/common.h>
#include <alloc.h>
#include <util.h>

#include <string.h>

#include <esp_log.h>

static const char *TAG = "wifi-common";

QueueHandle_t wifiQueueTx = NULL, wifiQueueRx = NULL;

esp_err_t createWifiQueues()
{
	wifiQueueTx = xQueueCreate(8, sizeof(queue_message*));
	LOG_FN_GOTO_IF_NULL(wifiQueueTx, "xQueueCreate:1", txQueueFail);
	wifiQueueRx = xQueueCreate(32, sizeof(queue_message*));
	LOG_FN_GOTO_IF_NULL(wifiQueueRx, "xQueueCreate:2", rxQueueFail);

	return ESP_OK;

rxQueueFail:
	vQueueDelete(wifiQueueTx);
	wifiQueueTx = NULL;
txQueueFail:
	return ESP_ERR_NO_MEM;
}

void destroyWifiQueues()
{
	vQueueDelete(wifiQueueTx);
	vQueueDelete(wifiQueueRx);
}
