#pragma once

/**
 * @brief Initializes Wi-Fi structures. Must be called before VGA code starts.
 */
void initWifi();

/**
 * @brief Creates the tasks that will provide Wi-Fi handling.
 */
void createWifiTasks();