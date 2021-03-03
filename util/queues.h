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
    WIFI_QUEUE_RX_DISCONNECTED
};

extern QueueHandle_t httpQueueRx;

// Queue flags to be used by any user-defined HTTP clients
enum http_queue_tx_flag {
	HTTP_QUEUE_TX_REQUEST_RADIO_LIST,
	HTTP_QUEUE_TX_REQUEST_RADIO_STATION
};

extern QueueHandle_t radioQueueTx, otaQueueTx;

// Queue flags to be used with the radio queues
enum radio_queue_rx_flag {
	RADIO_QUEUE_RX_WIFI_CONNECTED,
	RADIO_QUEUE_RX_RADIO_STATION,
	RADIO_QUEUE_RX_FINISHED_OP
};

/**
 * @brief Send data for a queue_message to a queue
 * 
 * @param queue Queue to send data to
 * @param msg_flags Flags to indicate action to take
 * @param msg_text Text to accompany certain flags
 */
esp_err_t sendQueueData(QueueHandle_t &queue, uint8_t msg_flags, const char *msg_text = NULL, uint32_t delay = 10 / portTICK_PERIOD_MS);