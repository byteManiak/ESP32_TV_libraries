#include <wifi/common.h>
#include <wifi/server.h>
#include <alloc.h>
#include <util.h>

#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <esp_event.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

#include <esp_netif_ip_addr.h>

static const char *TAG = "wifi-server";

static wifi_config_t wifiConfig;

static TaskHandle_t wifiTask;
static EventGroupHandle_t wifiEventGroup;

#define MAX_CONNECT_RETRY_COUNT 2
static uint8_t connectRetryCount = 0;

static void wifiEventLoop(void*, esp_event_base_t, int32_t, void*);
static void wifiStateMachine(void *pvParams);

void createWifiTasks()
{
	xTaskCreatePinnedToCore(wifiStateMachine, "WiFi", 4096, NULL, tskIDLE_PRIORITY, &wifiTask, 1);
}

void initWifi()
{
	esp_event_handler_instance_t wifiEvent, ipEvent;

	createWifiQueues();

	esp_err_t e = nvs_flash_init();
	LOG_FN(e, "nvs_flash_init");

	e = esp_netif_init();
	LOG_FN(e, "esp_netif_init");

	esp_netif_t *interface = esp_netif_create_default_wifi_sta();
	LOG_PTR(interface, "esp_netif_create_default_wifi_sta");

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
											WIFI_EVENT_STA_CONNECTED | WIFI_EVENT_STA_DISCONNECTED,
											&wifiEventLoop,
											NULL,
											&wifiEvent);
	LOG_FN(e, "esp_event_handler_instance_register:1");
	e = esp_event_handler_instance_register(IP_EVENT,
											IP_EVENT_STA_GOT_IP,
											&wifiEventLoop,
											NULL,
											&ipEvent);
	LOG_FN(e, "esp_event_handler_instance_register:1");
}

static void wifiEventLoop(void *args, esp_event_base_t eventBase, int32_t eventId, void *eventData)
{
	if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED)
	{
		// Notify the wifi task that a disconnect happened
		sendWifiQueueData(wifiQueueRx, WIFI_QUEUE_TX_EVENT_DISCONNECTED);
	}
	else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP)
	{
		// Send out the IP and gateway addresses to the VGA thread
		ip_event_got_ip_t *ipInfo = (ip_event_got_ip_t*)eventData;
		uint32_t ipAddressInt = ipInfo->ip_info.ip.addr;
		uint32_t gatewayAddressInt = ipInfo->ip_info.gw.addr;
		char ipAddress[16], gatewayAddress[16];
		sprintf(ipAddress, "%u.%u.%u.%u", 
							ipAddressInt & 0xFF,
							(ipAddressInt & 0xFF00) >> 8,
							(ipAddressInt & 0xFF0000) >> 16,
							(ipAddressInt & 0xFF000000) >> 24);

		sprintf(gatewayAddress, "%u.%u.%u.%u", 
								gatewayAddressInt & 0xFF,
								(gatewayAddressInt & 0xFF00) >> 8,
								(gatewayAddressInt & 0xFF0000) >> 16,
								(gatewayAddressInt & 0xFF000000) >> 24 );
		sendWifiQueueData(wifiQueueTx, WIFI_QUEUE_RX_CONNECTED);
		sendWifiQueueData(wifiQueueTx, WIFI_QUEUE_RX_IP_ADDRESS, ipAddress);
		sendWifiQueueData(wifiQueueTx, WIFI_QUEUE_RX_GATEWAY_ADDRESS, gatewayAddress);
	}
}


void wifiStateMachine(void *pvParams)
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

					ESP_LOGI(TAG, "Received scan event from queue %p", wifiQueueRx);

					e = esp_wifi_scan_start(&scanConfig, true);
					LOG_FN(e, "esp_wifi_scan_start");

					uint16_t apScanResultsMax = 8;
					wifi_ap_record_t apScanResults[8];
					e = esp_wifi_scan_get_ap_records(&apScanResultsMax, apScanResults);
					LOG_FN(e, "esp_wifi_scan_get_ap_records");

					for(int i = 0; i < apScanResultsMax; i++)	
					{
						ESP_LOGI(TAG, "Found network %d: %s", i+1, apScanResults[i].ssid);

						sendWifiQueueData(wifiQueueTx, WIFI_QUEUE_RX_SCAN_RESULT, (char*)apScanResults[i].ssid);
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

				case WIFI_QUEUE_TX_EVENT_DISCONNECTED:
				{
					// Attempt to connect until the retry count is reached
					if (connectRetryCount < MAX_CONNECT_RETRY_COUNT)
					{
						connectRetryCount++;
						ESP_LOGI(TAG, "Failed to connect %d times", connectRetryCount);
						esp_wifi_connect();
						break;
					}
					sendWifiQueueData(wifiQueueTx, WIFI_QUEUE_RX_DISCONNECTED, nullptr);
				}
			}
			heap_caps_free(rxMessage);
		}

		vTaskDelay(10 / portTICK_RATE_MS);
	}
}
