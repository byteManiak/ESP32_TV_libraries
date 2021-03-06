/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#define WIFI_TASK_PRIORITY 4
#define HTTP_TASK_PRIORITY 3
#define MP3_TASK_PRIORITY 2
#define I2S_TASK_PRIORITY 1

// Message to be passed around in RTOS queues with sendQueueData()
struct queue_message
{
	char msg_text[257];
	uint8_t msg_flags;
};

// Queues used to communicate between VGA thread and wifi task.
extern QueueHandle_t wifiQueueTx, wifiQueueRx;

// Flags to send from user to Wi-Fi task
enum wifi_queue_tx_flag {
	WIFI_QUEUE_TX_UNKNOWN,
	WIFI_QUEUE_TX_USER_BEGIN_SCAN,
	WIFI_QUEUE_TX_USER_SSID,
	WIFI_QUEUE_TX_USER_PSK,
	WIFI_QUEUE_TX_EVENT_CONNECTED,
	WIFI_QUEUE_TX_EVENT_DISCONNECTED
};

// Flags to send from Wi-Fi task to user
enum wifi_queue_rx_flag {
	WIFI_QUEUE_RX_UNKNOWN,
	WIFI_QUEUE_RX_SCAN_RESULT,
	WIFI_QUEUE_RX_IP_ADDRESS,
	WIFI_QUEUE_RX_GATEWAY_ADDRESS,
	WIFI_QUEUE_RX_CONNECTED,
	WIFI_QUEUE_RX_DISCONNECTED,
	WIFI_QUEUE_RX_HTTP_SERVER_ERROR
};

// Queue used to communicate GET requests to the HTTP task
extern QueueHandle_t httpQueueRx;

/**
 * @brief Send data for a queue_message to a queue
 * 
 * @param queue Queue to send data to
 * @param msg_flags Flags to indicate action to take
 * @param msg_text Text to accompany certain flags
 */
esp_err_t sendQueueData(QueueHandle_t &queue, uint8_t msg_flags, const char *msg_text = NULL, uint32_t delay = 10 / portTICK_PERIOD_MS);