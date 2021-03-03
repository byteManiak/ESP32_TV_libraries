#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <esp_log.h>

// PI*2
constexpr double pi2 = 3.14159 * 2;

/**
 * @brief Get microseconds since the ESP started.
 */
int64_t getMicros();

/**
 * @brief Get milliseconds since the ESP started.
 */
int64_t getMillis();

/**
 * @brief Get the next integer between [0,max), wrapping around to 0 if >= max.
 */
uint32_t getNextInt(uint32_t s, uint32_t max);

/**
 * @brief Get the previous integer between [0,max), wrapping around if < 0.
 */
uint32_t getPrevInt(uint32_t s, uint32_t max);

// Result of calculateTimeDelta().
extern double timeDelta;
/**
 * @brief Calculate the time delta between this call and the previous call to calculateTimeDelta()
 * NOTE: Do not call more than once per frame, and make sure to always call this from the same one task.
 * The result is stored in the global variable "timeDelta".
 */
void calculateTimeDelta();

/**
 * @brief Interpolate from a double to the other, storing the result in the first parameter.
 * @param from Double to interpolate from.
 * @param to Destination double to interpolate to.
 */
void smoothLerp(double &from, double &to);

// Message to be passed around in RTOS queues with sendQueueData()
struct queue_message
{
    char msg_text[257];
    uint8_t msg_flags;
};

/**
 * @brief Send data for a queue_message to a queue
 * 
 * @param queue Queue to send data to
 * @param msg_flags Flags to indicate action to take
 * @param msg_text Text to accompany certain flags
 */
esp_err_t sendQueueData(QueueHandle_t &queue, uint8_t msg_flags, const char *msg_text = NULL, uint32_t delay = 10 / portTICK_PERIOD_MS);

#define WIFI_TASK_PRIORITY 4
#define HTTP_TASK_PRIORITY 3
#define MP3_TASK_PRIORITY 2
#define I2S_TASK_PRIORITY 1

#if defined(CONFIG_DEBUG_VGA_PROJ)
#define LOG_INFO(msg, ...) { ESP_LOGI(TAG, "%s(%d): " msg, __func__, __LINE__, ##__VA_ARGS__); }
#define LOG_ERR(msg, ...) { ESP_LOGE(TAG, "%s(%d): " msg, __func__, __LINE__, ##__VA_ARGS__); }
#define LOG_FN(e, fn) { if (e != ESP_OK) ESP_LOGE(TAG, "%s(%d): %s returned 0x%0x", __func__, __LINE__, fn, e); }
#define LOG_PTR(ptr, fn) { if (!ptr) ESP_LOGE(TAG, "%s(%d): %s returned null", __func__, __LINE__, fn); }
#else
#define LOG_INFO(msg, ...)
#define LOG_ERR(msg, ...)
#define LOG_FN(e, fn)
#define LOG_PTR(ptr, fn)
#endif

#define LOG_FN_GOTO_IF_ERR(e, fn, label) { LOG_FN(e, fn); if (e != ESP_OK) goto label; }
#define LOG_FN_GOTO_IF_NULL(e, fn, label) { if (!e) { LOG_PTR(e, fn); goto label; } }