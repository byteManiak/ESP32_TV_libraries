#include <stats.h>
#include <vga.h>

#if defined(CONFIG_DEBUG_VGA_PROJ)
void printMemStat(VGAExtended *vga, uint32_t heap_caps)
{
	// Get heap info
	multi_heap_info_t heap_info = {};
	heap_caps_get_info(&heap_info, heap_caps);

	// Get a total memory approximate.
	// Due to page tables, .bss section and others,
	// the total size is not 100% accurate
	int freeMem = heap_info.total_free_bytes;
	int totalMem = freeMem + heap_info.total_allocated_bytes;
	int largestBlock = heap_info.largest_free_block;

	// Get free memory in kb, with a gradient from
	// green to red to show the "freeness" of the RAM
	double percent = freeMem / (double)totalMem;
	long color = vga->getPercentGradient(percent);
	vga->setTextColor(color);
	vga->print(freeMem/1024.f);
	vga->print("k (");
	vga->print(largestBlock/1024.f);
	vga->print("k) / ");
	vga->print(totalMem/1024.f);
	vga->println("k");
}

void printMemStats(VGAExtended *vga)
{
	// Save text colors that were previously used
	long f = vga->frontColor, b = vga->backColor;

	// Print memstats at the bottom left of the screen
	vga->setCursor(4, vga->yres-20);

	// Get internal memory stats
	vga->setTextColor(vga->RGB(0xFFFFFF));
	vga->print("Free SRAM:   ");
	printMemStat(vga, MALLOC_CAP_INTERNAL);

#if defined(CONFIG_ESP32_SPIRAM_SUPPORT)
	// Get external memory stat
	vga->setTextColor(vga->RGB(0xFFFFFF));
	vga->print("Free PSRAM:  ");
	printMemStat(vga, MALLOC_CAP_SPIRAM);
#endif

	// Restore previously used text colors
	vga->setTextColor(f, b);
}
#endif

#if defined(CONFIG_DEBUG_SHOW_FPS)
extern double timeDelta;

void printFPS(VGAExtended *vga)
{
	// Save text colors that were previously used
	long f = vga->frontColor, b = vga->backColor;

	vga->setCursor(4,4);
	vga->println(1/timeDelta);

	// Restore previously used text colors
	vga->setTextColor(f, b);
}
#endif
