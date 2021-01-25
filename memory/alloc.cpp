#include "alloc.h"

void *heap_caps_malloc_perror(TextBuf *textbuf, size_t size, uint32_t caps)
{
	void *t = heap_caps_malloc(size, caps);

#if defined(CONFIG_DEBUG_VGA_PROJ)
	// Print a debug message on the screen if the allocation failed
	if (!t)
	{
		char str[64];
		snprintf(str, 64, "Failed to alloc %f bytes", size/1024.f);
		textbuf->add(str);
	}
#endif

	return t;
}

