#include <string.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_http_client.h>
#include <nvs_flash.h>
#include <wifi.h>
#include <util.h>
#include <alloc.h>

EventGroupHandle_t wifiEventGroup;

// Queues used to communicate between VGA thread and wifi task
QueueHandle_t wifiQueueTx, wifiQueueRx;

static const char *TAG = "WiFi";

void createWifiQueues()
{
	wifiQueueTx = xQueueCreate(8, sizeof(wifi_queue_message*));
	LOG_PTR(wifiQueueTx, "xQueueCreate:1");
	wifiQueueRx = xQueueCreate(32, sizeof(wifi_queue_message*));
	LOG_PTR(wifiQueueRx, "xQueueCreate:2");
}

TaskHandle_t wifiTask;

void createWifiTasks()
{
	xTaskCreatePinnedToCore(connectWifi, "WiFi", 4096, NULL, tskIDLE_PRIORITY, &wifiTask, 1);
}

static void wifiEventHandler(void *args, esp_event_base_t eventBase, int32_t eventId, void *eventData)
{
	if (eventBase == WIFI_EVENT)
	{
		switch (eventId)
		{
			wifi_queue_message *txMessage;

			case WIFI_EVENT_STA_CONNECTED:
			{
				// Notify the wifi task that wifi connected to AP
				txMessage = heap_caps_malloc_cast<wifi_queue_message>(MALLOC_CAP_PREFERRED);
				txMessage->msg_flags = WIFI_QUEUE_TX_EVENT_CONNECTED;
				xQueueSend(wifiQueueRx, &txMessage, 10 / portTICK_PERIOD_MS);
				break;
			}
			case WIFI_EVENT_STA_DISCONNECTED:
			{
				// Notify the wifi task that a disconnect happened
				txMessage = heap_caps_malloc_cast<wifi_queue_message>(MALLOC_CAP_PREFERRED);
				txMessage->msg_flags = WIFI_QUEUE_TX_EVENT_DISCONNECTED;
				xQueueSend(wifiQueueRx, &txMessage, 10 / portTICK_PERIOD_MS);
				break;
			}
		}
	}
	else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP)
	{
		// Send out the IP and gateway addresses to the VGA thread
		ip_event_got_ip_t *ipInfo = (ip_event_got_ip_t*)eventData;
		uint32_t ipAddress = ipInfo->ip_info.ip.addr;
		uint32_t gatewayAddress = ipInfo->ip_info.gw.addr;

		wifi_queue_message *ipMessage, *gatewayMessage;

		ipMessage = heap_caps_malloc_cast<wifi_queue_message>(MALLOC_CAP_PREFERRED);
		ipMessage->msg_flags = WIFI_QUEUE_RX_IP_ADDRESS;
		sprintf(ipMessage->msg_text, "%u.%u.%u.%u", 
		                             ipAddress & 0xFF,
		                             (ipAddress & 0xFF00) >> 8,
		                             (ipAddress & 0xFF0000) >> 16,
		                             (ipAddress & 0xFF000000) >> 24 );
		xQueueSend(wifiQueueTx, &ipMessage, 10 / portTICK_PERIOD_MS);

		gatewayMessage = heap_caps_malloc_cast<wifi_queue_message>(MALLOC_CAP_PREFERRED);
		gatewayMessage->msg_flags = WIFI_QUEUE_RX_GATEWAY_ADDRESS;
		sprintf(gatewayMessage->msg_text, "%u.%u.%u.%u", 
		                 	              gatewayAddress & 0xFF,
		                      	          (gatewayAddress & 0xFF00) >> 8,
		                          	      (gatewayAddress & 0xFF0000) >> 16,
		                             	  (gatewayAddress & 0xFF000000) >> 24 );
		xQueueSend(wifiQueueTx, &gatewayMessage, 10 / portTICK_PERIOD_MS);
	}
}

void initWifi()
{
	esp_event_handler_instance_t wifiEvent, ipEvent;

	esp_err_t e = nvs_flash_init();
	LOG_FN(e, "nvs_flash_init");

	e = esp_netif_init();
	LOG_FN(e, "esp_netif_init");

	esp_netif_t *interface = esp_netif_create_default_wifi_sta();
	LOG_PTR(interface, "esp_netif_create_default_wifi_sta");

	createWifiQueues();

	wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();
	e = esp_wifi_init(&initConfig);
	LOG_FN(e, "esp_wifi_init");

	e = esp_wifi_set_mode(WIFI_MODE_STA);
	LOG_FN(e, "esp_wifi_set_mode");

	createWifiTasks();

	wifiEventGroup = xEventGroupCreate();

	e = esp_wifi_start();
	LOG_FN(e, "esp_wifi_start");

	e = esp_event_handler_instance_register(WIFI_EVENT,
											WIFI_EVENT_STA_DISCONNECTED,
											&wifiEventHandler,
											NULL,
											&wifiEvent);
	LOG_FN(e, "esp_event_handler_instance_register:1");
	e = esp_event_handler_instance_register(IP_EVENT,
											IP_EVENT_STA_GOT_IP,
											&wifiEventHandler,
											NULL,
											&ipEvent);
	LOG_FN(e, "esp_event_handler_instance_register:1");
}

static wifi_config_t wifiConfig;

#define MAX_RETRY_COUNT 5
static uint8_t connectRetryCount = 0;

void connectWifi(void *pvParams)
{
	for(;;)
	{
		esp_err_t e;

		if (!(wifiQueueRx && wifiQueueTx)) continue;

		wifi_queue_message *rxMessage;
		if (xQueueReceive(wifiQueueRx, &rxMessage, 0) == pdTRUE)
		{
			switch(rxMessage->msg_flags)
			{
				case WIFI_QUEUE_TX_USER_BEGIN_SCAN:
				{
					wifi_scan_config_t scanConfig = {};
					scanConfig.scan_type = WIFI_SCAN_TYPE_ACTIVE;
					scanConfig.scan_time.active.max = 250;

					ESP_LOGI("wifi", "Received scan event from queue %p", wifiQueueRx);

					e = esp_wifi_scan_start(&scanConfig, true);
					LOG_FN(e, "esp_wifi_scan_start");

					uint16_t apScanResultsMax = 8;
					wifi_ap_record_t apScanResults[8];
					e = esp_wifi_scan_get_ap_records(&apScanResultsMax, apScanResults);
					LOG_FN(e, "esp_wifi_scan_get_ap_records");

					for(int i = 0; i < apScanResultsMax; i++)	
					{
						ESP_LOGI(TAG, "Found network %d: %s", i+1, apScanResults[i].ssid);

						wifi_queue_message *txMessage = heap_caps_malloc_cast<wifi_queue_message>(MALLOC_CAP_PREFERRED);
						txMessage->msg_flags = WIFI_QUEUE_RX_SCAN_RESULT;
						strlcpy(txMessage->msg_text, (char*)apScanResults[i].ssid, 33);

						ESP_LOGI(TAG, "Sending %p through queue %p", txMessage, wifiQueueTx);
						xQueueSend(wifiQueueTx, &txMessage, 10);
					}
					break;
				}
				case WIFI_QUEUE_TX_USER_SSID:
				{
					ESP_LOGI(TAG, "Received ssid %s through queue %p", rxMessage->msg_text, wifiQueueRx);
					strlcpy((char*)wifiConfig.sta.ssid, rxMessage->msg_text, 33);
					break;
				}
				case WIFI_QUEUE_TX_USER_PSK:
				{
					ESP_LOGI(TAG, "Received psk %s through queue %p", rxMessage->msg_text, wifiQueueRx);
					strlcpy((char*)wifiConfig.sta.password, rxMessage->msg_text, 65);

					esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
					esp_wifi_connect();
					break;
				}
				case WIFI_QUEUE_TX_EVENT_CONNECTED:
				{
					connectRetryCount = 0;
					break;
				}
				case WIFI_QUEUE_TX_EVENT_DISCONNECTED:
				{
					if (connectRetryCount < MAX_RETRY_COUNT)
					{
						connectRetryCount++;
						esp_wifi_connect();
						break;
					}
				}
			}
			heap_caps_free(rxMessage);
		}

		vTaskDelay(10 / portTICK_RATE_MS);
	}

}
