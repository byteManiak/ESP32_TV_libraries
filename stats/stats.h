#pragma once

#include <VGA/VGA6Bit.h>

#if defined(CONFIG_DEBUG_VGA_PROJ)
void printMemStat(VGA6Bit &vga, uint32_t heap_caps);
void printMemStats(VGA6Bit &vga);
#endif

#if defined(CONFIG_DEBUG_SHOW_FPS)
void printFPS(VGA6Bit &vga);
#endif
