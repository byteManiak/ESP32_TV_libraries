#pragma once

#include <vga.h>

#if defined(CONFIG_DEBUG_VGA_PROJ)
void printMemStat(VGAExtended *vga, uint32_t heap_caps);
void printMemStats(VGAExtended *vga);
#endif

#if defined(CONFIG_DEBUG_SHOW_FPS)
void printFPS(VGAExtended *vga);
#endif
