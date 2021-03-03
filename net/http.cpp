#include <net/http.h>

#include <freertos/queue.h>
#include <freertos/task.h>
#include <esp_http_client.h>

#include <alloc.h>
#include <util.h>
#include <sound.h>

#include <string.h>

static const char *TAG = "http";

QueueHandle_t httpQueueRx = NULL;
QueueHandle_t radioQueueTx = NULL, otaQueueTx = NULL;

esp_http_client_handle_t httpClient = NULL;

static char *buf = NULL;
static int contentSize = 0;
static int writtenBytes = 0;
static int lastResponse = 0;

static char url[256] = {};
static char requestType[16] = {};
static int requestValues[2];

void parseURL()
{
	char *tmp = url + strlen(CONFIG_HTTP_SERVER_URL) + 1;
	char *req = strtok(tmp, "=");
	strcpy(requestType, req);
	char *val1 = strtok(NULL, ",");
	requestValues[0] = atoi(val1);
	char *val2 = strtok(NULL, ",");
	if (val2) requestValues[1] = atoi(val2);

	LOG_INFO("GET request is %s", tmp);
	LOG_INFO("VALUE 1 is %d", requestValues[0]);
	if (val2) LOG_INFO("VALUE 2 is %d", requestValues[1]);
}
static esp_err_t httpEventHandler(esp_http_client_event_t *event)
{
	switch(event->event_id)
	{
		case HTTP_EVENT_ON_CONNECTED:
		{
			LOG_INFO("HTTP connected");
			break;
		}
		case HTTP_EVENT_ON_HEADER:
		{
			LOG_INFO("HTTP header received: %s = %s", event->header_key, event->header_value);
			esp_http_client_get_url(httpClient, url, 256);
			LOG_INFO("%s", url);

			if (!strcmp(event->header_key, "Content-Length"))
			{
				contentSize = atoi(event->header_value);
				// In case the input data is parsed as a string, 
				// allocate an extra byte to be able to then NUL-terminate it
				buf = heap_caps_malloc_cast<char>(MALLOC_CAP_PREFERRED, contentSize+1);

				parseURL();
			}
			break;
		}
		case HTTP_EVENT_ON_DATA:
		{
			LOG_INFO("HTTP data received");

			lastResponse = esp_http_client_get_status_code(httpClient);

			LOG_INFO("Status code: %d", lastResponse);
			LOG_INFO("Size of data: %d", contentSize);
			if (lastResponse == 200 && contentSize > 0 && !esp_http_client_is_chunked_response(event->client))
			{
				memcpy(&buf[writtenBytes], event->data, event->data_len);
				writtenBytes += event->data_len;
			}
			break;
		}
		case HTTP_EVENT_ON_FINISH:
		{
			writtenBytes = 0;
			// NUL-terminate the buffer
			buf[contentSize] = '\0';

			if (!strcmp(requestType, "radio"))
			{
				char *stationName = strtok(buf, ";");
				for(int i = requestValues[0]; i <= requestValues[1]; i++)
				{
					if (!stationName) break;
					LOG_INFO("%s", stationName);
					sendQueueData(radioQueueTx, RADIO_QUEUE_RX_RADIO_STATION, stationName, portMAX_DELAY);
					stationName = strtok(NULL, ";");
				}
				sendQueueData(radioQueueTx, RADIO_QUEUE_RX_FINISHED_OP, NULL, portMAX_DELAY);
			}
			else if (!strcmp(requestType, "station"))
			{
				char *audioURL = heap_caps_malloc_cast<char>(MALLOC_CAP_PREFERRED, 256);
				strlcpy(audioURL, buf, 256);
				xTaskCreatePinnedToCore(audioDispatchTask, "audioTask", 3072, audioURL, tskIDLE_PRIORITY, NULL, 1);
			}
			break;
		}
		default: break;
	}
	return ESP_OK;
}

static esp_err_t createHTTPQueues()
{
	httpQueueRx = xQueueCreate(8, sizeof(queue_message*));
	LOG_FN_GOTO_IF_NULL(httpQueueRx, "xQueueCreate:1", rxQueueFail);

	radioQueueTx = xQueueCreate(16, sizeof(queue_message*));
	LOG_FN_GOTO_IF_NULL(radioQueueTx, "xQueueCreate:2", radioQueueFail);

	otaQueueTx = xQueueCreate(4, sizeof(queue_message*));
	LOG_FN_GOTO_IF_NULL(otaQueueTx, "xQueueCreate:3", otaQueueFail);

	return ESP_OK;

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

esp_err_t initHTTP()
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

void sendRadioQueueData()
{

}

void sendOTAQueueData()
{

}