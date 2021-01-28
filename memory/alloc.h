#pragma once

#include <esp_heap_caps.h>
#include <textview.h>

void *heap_caps_malloc_perror(TextView *textbuf, size_t size, uint32_t heap_caps);
