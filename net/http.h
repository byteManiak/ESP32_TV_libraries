#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <esp_err.h>
#include <esp_http_client.h>

#define HTTP_SERVER_ADDRESS CONFIG_HTTP_SERVER_URL ":" CONFIG_HTTP_SERVER_PORT

extern esp_http_client_handle_t httpClient;

/**
 * @brief Initialize HTTP client for user applications. The client can then accept get requests by sending
 *        data to httpQueueRx. NOTE: Users must implement their own event handler to make proper use of the client.
 *
 * @param httpEventHandler Function pointer to the event handler of the HTTP client.
 */
esp_err_t initHTTP(esp_err_t (*httpEventHandler)(esp_http_client_event_t* event));
