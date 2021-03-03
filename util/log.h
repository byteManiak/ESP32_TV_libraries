#pragma once

#include <esp_log.h>

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