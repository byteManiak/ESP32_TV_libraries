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
	if (!httpQueueRx) return ESP_ERR_NO_MEM;
	return ESP_OK;
}

static char *replaceSpaces(char *str)
{
	if (!strstr(str, " ")) return strdup(str);

	char *buf = heap_caps_malloc_cast<char>(MALLOC_CAP_PREFERRED, 512);
	char *cursor = buf;

	char *tok = strtok(str, " ");
	while(tok)
	{
		cursor += sprintf(cursor, "%s", tok);
		tok = strtok(NULL, " ");
		if (tok) cursor += sprintf(cursor, "%%20");
	}

	LOG_INFO("%s", buf);

	return buf;
}

void httpServer(void *arg)
{
	for(;;)
	{
		queue_message *rxMessage;
		char *urlNoSpaces;
		xQueueReceive(httpQueueRx, &rxMessage, portMAX_DELAY);

		urlNoSpaces = replaceSpaces(rxMessage->msg_text);

		esp_http_client_set_url(httpClient, urlNoSpaces);
		esp_err_t err = esp_http_client_perform(httpClient);
		if (err == ESP_ERR_HTTP_CONNECT) sendQueueData(wifiQueueTx, WIFI_QUEUE_RX_HTTP_SERVER_ERROR, NULL, 0);

		heap_caps_free(urlNoSpaces);
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