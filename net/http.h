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

#define MAKE_REQUEST_URL(getRequest, ...) \
	char *url = heap_caps_malloc_cast<char>(MALLOC_CAP_PREFERRED, 256); \
	snprintf(url, 256, HTTP_SERVER_ADDRESS "/" getRequest, __VA_ARGS__);

#define GET_REQUEST_URL() (url)

#define FREE_REQUEST_URL() {heap_caps_free(url); }