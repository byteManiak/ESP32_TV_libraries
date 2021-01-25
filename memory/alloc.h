#pragma once

#include <esp_heap_caps.h>
#include <textbuf.h>

void *heap_caps_malloc_perror(TextBuf *textbuf, size_t size, uint32_t heap_caps);
