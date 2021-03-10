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
	HTTP_QUEUE_TX_REQUEST_RADIO_LIST,    // Signal the HTTP task to return the list of radio stations
	HTTP_QUEUE_TX_REQUEST_RADIO_STATION, // Signal the HTTP task to return the URL of the selected radio station
    HTTP_QUEUE_TX_REQUEST_APP_LIST,      // Signal the HTTP task to return the list of user-downloadable applications
    HTTP_QUEUE_TX_REQUEST_APP,           // Signal the HTTP task to download the selected application
    HTTP_QUEUE_TX_REQUEST_NEWS_LIST      // Signal the HTTP task to return the list of news headlines from the current RSS feed
};

extern QueueHandle_t radioQueueTx;

// Queue flags to be used with the radio queues
enum radio_queue_rx_flag {
	RADIO_QUEUE_RX_WIFI_CONNECTED, // Signal the radio menu that wifi is connected
	RADIO_QUEUE_RX_RADIO_STATION,  // Signal the radio menu that a radio station was sent to the list
	RADIO_QUEUE_RX_FINISHED_OP     // Signal the radio menu that the server finished sending the list of radio stations
};

extern QueueHandle_t appQueueTx;

// Queue flags to be used with the app queues
enum app_queue_rx_flag {
    APP_QUEUE_RX_WIFI_CONNECTED, // Signal the app menu that wifi is connected
    APP_QUEUE_RX_APP_NAME,       // Signal the app menu that an application was sent to the list
    APP_QUEUE_RX_FINISHED_OP // Signal the app menu that the server finished sending the list of application
};

extern QueueHandle_t newsQueueTx;

// Queue flags to be used with the news queues
enum news_queue_rx_flag {
    NEWS_QUEUE_RX_WIFI_CONNECTED,
    NEWS_QUEUE_RX_RSS_FEED_NAME,
    NEWS_QUEUE_RX_HEADLINE_NAME,
    NEWS_QUEUE_RX_FINISHED_OP
};

/**
 * @brief Send data for a queue_message to a queue
 * 
 * @param queue Queue to send data to
 * @param msg_flags Flags to indicate action to take
 * @param msg_text Text to accompany certain flags
 */
esp_err_t sendQueueData(QueueHandle_t &queue, uint8_t msg_flags, const char *msg_text = NULL, uint32_t delay = 10 / portTICK_PERIOD_MS);