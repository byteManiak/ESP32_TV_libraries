#pragma once

#include <vector>
#include <esp_heap_caps.h>
#include <textview.h>

void *heap_caps_malloc_perror(TextView *textbuf, size_t size, uint32_t heap_caps);

#if defined(CONFIG_ESP32_SPIRAM_SUPPORT)
#define MALLOC_CAP_PREFERRED MALLOC_CAP_SPIRAM
template <typename T>
struct PSRAM_Allocator
{
	typedef T value_type;
	PSRAM_Allocator() {}

	template <typename U> PSRAM_Allocator(const PSRAM_Allocator<U> &) {}

	T *allocate(size_t n)
	{
		return (T*)heap_caps_malloc(sizeof(T) * n, MALLOC_CAP_SPIRAM);
	}

	void construct(T *p, const T &c)
	{
		new (p) T(c);
	}

	void deallocate(T *p, size_t n)
	{
		heap_caps_free(p);
	}
};
#else
#define MALLOC_CAP_PREFERRED MALLOC_CAP_DEFAULT
#endif

template <typename T>
#if defined(CONFIG_ESP32_SPIRAM_SUPPORT)
using heap_caps_vector = std::vector<T, PSRAM_Allocator<T>>;
#else
using heap_caps_vector = std::vector<T, std::allocator<T>>;
#endif

