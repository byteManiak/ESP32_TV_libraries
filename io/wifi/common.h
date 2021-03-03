#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include <esp_err.h>

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

// Queues used to communicate between VGA thread and wifi task.
extern QueueHandle_t wifiQueueTx, wifiQueueRx;

/**
 * @brief Creates the queues that will be used to receive and send Wi-Fi state data.
 */
esp_err_t createWifiQueues();

/**
 * @brief Destroys the queues used to receive and send Wi-Fi state data.
 */
void destroyWifiQueues();
