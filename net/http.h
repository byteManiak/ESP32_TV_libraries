#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <esp_err.h>
#include <esp_http_client.h>

#define HTTP_SERVER_ADDRESS CONFIG_HTTP_SERVER_URL ":" CONFIG_HTTP_SERVER_PORT

extern QueueHandle_t httpQueueRx;
extern QueueHandle_t radioQueueTx, otaQueueTx;

extern esp_http_client_handle_t httpClient;

esp_err_t initHTTP();

enum http_queue_tx_flag {
	HTTP_QUEUE_TX_REQUEST_RADIO_LIST,
	HTTP_QUEUE_TX_REQUEST_RADIO_STATION
};

enum radio_queue_rx_flag {
	RADIO_QUEUE_RX_WIFI_CONNECTED,
	RADIO_QUEUE_RX_RADIO_STATION,
	RADIO_QUEUE_RX_FINISHED_OP
};

void httpGet(char *url);
