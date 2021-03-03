#pragma once

#include <esp_err.h>

/**
 * @brief Initializes Wi-Fi structures. Must be called before VGA code starts.
 */
esp_err_t initWifi();

/**
 * @brief Creates the tasks that will provide Wi-Fi handling.
 */
esp_err_t createWifiTasks();