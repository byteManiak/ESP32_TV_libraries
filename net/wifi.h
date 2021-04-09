/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

/**
 * @file wifi.h
 * @brief Contains objects and functions used to provide WiFi functionality.
 **/

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include <esp_err.h>

/**
 * @brief Initializes Wi-Fi structures. Must be called before VGA code starts.
 */
esp_err_t initWifi();