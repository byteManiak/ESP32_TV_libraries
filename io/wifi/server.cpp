#include <wifi/common.h>
#include <wifi/server.h>
#include <alloc.h>
#include <util.h>
#include <net/http.h>

#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <esp_event.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

#include <esp_netif_ip_addr.h>
#include <esp_http_client.h>

static const char *TAG = "wifi-server";

static wifi_config_t wifiConfig;

static TaskHandle_t wifiTask;
static EventGroupHandle_t wifiEventGroup;

#define MAX_CONNECT_RETRY_COUNT 2
static uint8_t connectRetryCount = 0;

static void wifiEventLoop(void*, esp_event_base_t, int32_t, void*);
static void wifiStateMachine(void *pvParams);

esp_err_t createWifiTasks()
{
	BaseType_t error = xTaskCreatePinnedToCore(wifiStateMachine, "WiFi", 4096, NULL, WIFI_TASK_PRIORITY, &wifiTask, 1);
	if (error != pdPASS) return ESP_ERR_NO_MEM;

	return ESP_OK;
}

esp_err_t initWifi()
{
	esp_event_handler_instance_t wifiEvent, ipEvent;
	esp_netif_t *interface;
	wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();

	esp_err_t error = createWifiQueues();
	LOG_FN_GOTO_IF_ERR(error, "createWifiQueues", queuesFail);

	error = nvs_flash_init();
	LOG_FN_GOTO_IF_ERR(error, "nvs_flash_init", nvsFail);

	error = esp_netif_init();
	LOG_FN_GOTO_IF_ERR(error, "esp_netif_init", netifFail);

	interface = esp_netif_create_default_wifi_sta();
	if (!interface)
	{
		error = ESP_ERR_NO_MEM;
		LOG_FN_GOTO_IF_NULL(interface, "esp_netif_create_default_wifi_sta", netifStaFail);
	}

	error = esp_wifi_init(&initConfig);
	LOG_FN_GOTO_IF_ERR(error, "esp_wifi_init", wifiInitFail);

	error = esp_wifi_set_mode(WIFI_MODE_STA);
	LOG_FN_GOTO_IF_ERR(error, "esp_wifi_set_mode", wifiModeFail);

	error = createWifiTasks();
	LOG_FN_GOTO_IF_ERR(error, "createWifiTasks", wifiTaskFail);

	wifiEventGroup = xEventGroupCreate();
	if (!wifiEventGroup)
	{
		error = ESP_ERR_NO_MEM;
		LOG_FN_GOTO_IF_NULL(wifiEvent, "xEventGroupCreate", eventGroupFail);	
	}

	error = esp_wifi_start();
	LOG_FN_GOTO_IF_ERR(error, "esp_wifi_start", wifiStartFail);

	error = esp_event_handler_instance_register(WIFI_EVENT,
											WIFI_EVENT_STA_CONNECTED | WIFI_EVENT_STA_DISCONNECTED,
											&wifiEventLoop,
											NULL,
											&wifiEvent);
	LOG_FN_GOTO_IF_ERR(error, "esp_event_handler_instance_register:1", wifiEventRegFail);
	error = esp_event_handler_instance_register(IP_EVENT,
											IP_EVENT_STA_GOT_IP,
											&wifiEventLoop,
											NULL,
											&ipEvent);
	LOG_FN_GOTO_IF_ERR(error, "esp_event_handler_instance_register:1", ipEventRegFail);

success:
	return ESP_OK;

ipEventRegFail:
	esp_event_handler_instance_unregister(WIFI_EVENT, 
										  WIFI_EVENT_STA_CONNECTED | WIFI_EVENT_STA_DISCONNECTED,
										  wifiEvent);
wifiEventRegFail:
	esp_wifi_stop();
wifiStartFail:
	vEventGroupDelete(wifiEventGroup);
eventGroupFail:
	vTaskDelete(wifiTask);
	wifiTask = NULL;
wifiTaskFail:
wifiModeFail:
	esp_wifi_deinit();
wifiInitFail:
netifStaFail:
	esp_netif_deinit();
netifFail:
nvsFail:
	destroyWifiQueues();
queuesFail:
	return error;
}

static void wifiEventLoop(void *args, esp_event_base_t eventBase, int32_t eventId, void *eventData)
{
	if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED)
	{
		// Notify the wifi task that a disconnect happened
		sendQueueData(wifiQueueRx, WIFI_QUEUE_TX_EVENT_DISCONNECTED);
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
		sendQueueData(wifiQueueTx, WIFI_QUEUE_RX_CONNECTED);
		sendQueueData(wifiQueueTx, WIFI_QUEUE_RX_IP_ADDRESS, ipAddress);
		sendQueueData(wifiQueueTx, WIFI_QUEUE_RX_GATEWAY_ADDRESS, gatewayAddress);

		sendQueueData(radioQueueTx, RADIO_QUEUE_RX_WIFI_CONNECTED, NULL);
	}
}


void wifiStateMachine(void *pvParams)
{
	for(;;)
	{
		esp_err_t error;

		if (!(wifiQueueRx && wifiQueueTx)) continue;

		queue_message *rxMessage;
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

					error = esp_wifi_scan_start(&scanConfig, true);
					LOG_FN(error, "esp_wifi_scan_start");

					uint16_t apScanResultsMax = 8;
					wifi_ap_record_t apScanResults[8];
					error = esp_wifi_scan_get_ap_records(&apScanResultsMax, apScanResults);
					LOG_FN(error, "esp_wifi_scan_get_ap_records");

					for(int i = 0; i < apScanResultsMax; i++)	
					{
						ESP_LOGI(TAG, "Found network %d: %s", i+1, apScanResults[i].ssid);

						sendQueueData(wifiQueueTx, WIFI_QUEUE_RX_SCAN_RESULT, (char*)apScanResults[i].ssid);
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
					sendQueueData(wifiQueueTx, WIFI_QUEUE_RX_DISCONNECTED, NULL);
				}
			}
			heap_caps_free(rxMessage);
		}

		vTaskDelay(10 / portTICK_RATE_MS);
	}
}
