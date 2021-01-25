#include <stats.h>
#include <ux.h>

#if defined(CONFIG_DEBUG_VGA_PROJ)
void printMemStat(VGA6Bit &vga, uint32_t heap_caps)
{
	// Get heap info
	multi_heap_info_t heap_info = {};
	heap_caps_get_info(&heap_info, heap_caps);

	// Get a total memory approximate.
	// Due to page tables, .bss section and others,
	// the total size is not 100% accurate
	int totalMem = heap_info.total_free_bytes + heap_info.total_allocated_bytes;

	// Get free memory in kb, with a gradient from
	// green to red to show the "freeness" of the RAM
	double percent = heap_info.total_free_bytes / (double)totalMem;
	long color = getPercentGradient(vga, percent);
	vga.setTextColor(color);
	vga.print(heap_info.total_free_bytes/1024.f);
	vga.print("k / ");
	vga.print(totalMem/1024.f);
	vga.println("k");
}

void printMemStats(VGA6Bit &vga)
{
	// Save text colors that were previously used
	long f = vga.frontColor, b = vga.backColor;

	// Print memstats at the bottom left of the screen
	vga.setCursor(4, vga.yres-20);

	// Get internal memory stats
	vga.setTextColor(vga.RGB(0xFFFFFF));
	vga.print("Free SRAM:   ");
	printMemStat(vga, MALLOC_CAP_INTERNAL);

	// Get external memory stat
	vga.setTextColor(vga.RGB(0xFFFFFF));
	vga.print("Free PSRAM:  ");
	printMemStat(vga, MALLOC_CAP_SPIRAM);

	// Restore previously used text colors
	vga.setTextColor(f, b);
}
#endif

#if defined(CONFIG_DEBUG_SHOW_FPS)
static long timePrev;

void printFPS(VGA6Bit &vga)
{
	// Save text colors that were previously used
	long f = vga.frontColor, b = vga.backColor;

	// Get time delta
	long timeNow = millis();
	long frametime = timeNow - timePrev;

	vga.setCursor(4,4);
	vga.print("FPS: ");

	// Avoid divide by 0 when nothing is drawn on screen
	if (frametime > 0)
	{
		vga.println(1000.f/frametime);
	}
	else
	{
		vga.println("1000+");
	}

	timePrev = timeNow;

	// Restore previously used text colors
	vga.setTextColor(f, b);
}
#endif
