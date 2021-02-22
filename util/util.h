#pragma once

#include <esp_log.h>

constexpr double pi2 = 3.14159 * 2;

int64_t getMicros();
int64_t getMillis();
uint32_t getNextInt(uint32_t s, uint32_t max);
uint32_t getPrevInt(uint32_t s, uint32_t max);

extern double timeDelta;
void calculateTimeDelta();

void smoothLerp(double &from, double &to);

#define WIFI_TASK_PRIORITY 4
#define HTTP_TASK_PRIORITY 3
#define MP3_TASK_PRIORITY 2
#define I2S_TASK_PRIORITY 1

#if defined(CONFIG_DEBUG_VGA_PROJ)
#define LOG_INFO(msg, ...) { ESP_LOGI(TAG, "%s(%d): " msg, __func__, __LINE__, __VA_ARGS__); }
#define LOG_ERR(msg, ...) { ESP_LOGE(TAG, "%s(%d): " msg, __func__, __LINE__, __VA_ARGS__); }
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