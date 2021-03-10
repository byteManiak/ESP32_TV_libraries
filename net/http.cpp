#include <net/http.h>

#include <freertos/queue.h>
#include <freertos/task.h>
#include <esp_http_client.h>

#include <memory/alloc.h>
#include <util/queues.h>
#include <util/log.h>
#include <io/sound.h>

#include <string.h>

static const char *TAG = "http";

esp_http_client_handle_t httpClient = NULL;

static esp_err_t createHTTPQueues()
{
	httpQueueRx = xQueueCreate(8, sizeof(queue_message*));
	LOG_FN_GOTO_IF_NULL(httpQueueRx, "xQueueCreate:1", rxQueueFail);

	radioQueueTx = xQueueCreate(16, sizeof(queue_message*));
	LOG_FN_GOTO_IF_NULL(radioQueueTx, "xQueueCreate:2", radioQueueFail);

	appQueueTx = xQueueCreate(4, sizeof(queue_message*));
	LOG_FN_GOTO_IF_NULL(appQueueTx, "xQueueCreate:3", otaQueueFail);

	newsQueueTx = xQueueCreate(4, sizeof(queue_message*));
	LOG_FN_GOTO_IF_NULL(newsQueueTx, "xQueueCreate:4", newsQueueFail);

	return ESP_OK;

newsQueueFail:
	vQueueDelete(appQueueTx);
	appQueueTx = NULL;
otaQueueFail:
	vQueueDelete(radioQueueTx);
	radioQueueTx = NULL;
radioQueueFail:
	vQueueDelete(httpQueueRx);
	httpQueueRx = NULL;
rxQueueFail:
	return ESP_ERR_NO_MEM;
}

void httpServer(void *arg)
{
	for(;;)
	{
		queue_message *rxMessage;
		xQueueReceive(httpQueueRx, &rxMessage, portMAX_DELAY);

		esp_http_client_set_url(httpClient, rxMessage->msg_text);
		esp_http_client_perform(httpClient);

		heap_caps_free(rxMessage);
	}
}

esp_err_t initHTTP(esp_err_t (*httpEventHandler)(esp_http_client_event_t* event))
{
	esp_err_t error = ESP_OK;
	esp_http_client_config_t httpConfig = {};

	error = createHTTPQueues();
	LOG_FN_GOTO_IF_ERR(error, "createHTTPQueues", ret);

	httpConfig.event_handler = httpEventHandler;
	// URL is actually a placeholder as the WiFi network
	// might be unavailable when the HTTP client is initialized
	httpConfig.url = "http://127.0.0.1";
	httpClient = esp_http_client_init(&httpConfig);
	if (!httpClient) error = ESP_ERR_NO_MEM;

	xTaskCreatePinnedToCore(httpServer, "httpServer", 3072, NULL, HTTP_TASK_PRIORITY, NULL, 1);

ret:
	return error;
}