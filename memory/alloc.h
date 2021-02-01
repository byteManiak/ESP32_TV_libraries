#pragma once

#include <vector>
#include <esp_heap_caps.h>

template <class T, class... Args>
T *heap_caps_malloc_construct(uint32_t caps, Args... args)
{
	T *obj = (T*)heap_caps_malloc(sizeof(T), caps);
	if (obj)
	{
		new (obj) T(args...);
	}
	else
	{
		//LOG_PTR(obj, "heap_caps_malloc");
	}

	return obj;
}

template <typename T>
T *heap_caps_malloc_cast(uint32_t caps, size_t n = 1)
{
	T *obj = (T*)heap_caps_malloc(sizeof(T) * n, caps);
	return obj;
}

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

