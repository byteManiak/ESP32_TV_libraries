#pragma once

#include <vga.h>

#if defined(CONFIG_DEBUG_VGA_PROJ)
/**
 * @brief Print free RAM statistics of DRAM and PSRAM (if enabled).
 *        NOTE: Call at the end of the frame.
 *
 * @param vga Handle of VGA display to output to.
 */
void printMemStats(VGAExtended *vga);
#endif

#if defined(CONFIG_DEBUG_SHOW_FPS)
/**
 * @brief Display the framerate of the VGA display.
 *        NOTE: Call at the end of the frame.
 *
 * @param vga Handle of VGA display to output to.
 */
void printFPS(VGAExtended *vga);
#endif
