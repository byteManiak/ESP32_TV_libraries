#pragma once

#include <esp_heap_caps.h>
#include <textview.h>

void *heap_caps_malloc_perror(TextView *textbuf, size_t size, uint32_t heap_caps);

template <class T>
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
