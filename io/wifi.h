#pragma once

#include <freertos/event_groups.h>
#include <esp_netif_ip_addr.h>

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
extern EventGroupHandle_t wifiEventGroup;

enum wifi_queue_tx_flag {
    WIFI_QUEUE_TX_UNKNOWN,
    WIFI_QUEUE_TX_USER_BEGIN_SCAN,
    WIFI_QUEUE_TX_USER_SSID,
    WIFI_QUEUE_TX_USER_PSK,
    WIFI_QUEUE_TX_EVENT_CONNECTED,
    WIFI_QUEUE_TX_EVENT_DISCONNECTED
};

enum wifi_queue_rx_flag {
    WIFI_QUEUE_RX_UNKNOWN,
    WIFI_QUEUE_RX_SCAN_RESULT,
    WIFI_QUEUE_RX_IP_ADDRESS,
    WIFI_QUEUE_RX_GATEWAY_ADDRESS,
    WIFI_QUEUE_RX_CONNECTED,
    WIFI_QUEUE_RX_DISCONNECTED
};

struct wifi_queue_message
{
    char msg_text[65];
    uint8_t msg_flags;
};

extern QueueHandle_t wifiQueueTx, wifiQueueRx;

/**
 * @brief Initialize structures that will be used for Wi-Fi connectivity
 */
void initWifi();

void connectWifi(void *pvParams);