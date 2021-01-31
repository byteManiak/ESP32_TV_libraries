#pragma once

#include <esp_log.h>

const double pi2 = 3.14159 * 2;

int64_t getTimeus();
int64_t getMillis();

extern double timeDelta;
void calculateTimeDelta();

#if defined(CONFIG_DEBUG_VGA_PROJ)
#define LOG_FN(e, fn) { ESP_LOGI(TAG, "%s(%d): %s returned 0x%0x", __func__, __LINE__, fn, e); }
#define LOG_PTR(ptr, fn) { ESP_LOGI(TAG, "%s(%d): %s returned %p", __func__, __LINE__, fn, ptr); }
#else
#define LOG_FN(e, fn)
#define LOG_PTR(ptr, fn)
#endif

