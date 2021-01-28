#pragma once

#include <VGA/VGA6Bit.h>

long getPercentGradient(VGA6Bit &vga, double percent);

void fillRectAlpha(VGA6Bit& vga, int x, int y, int w, int h, unsigned char color);

void printBox(VGA6Bit &vga, const char *text, int x, int y, long textColor = 0xFFFFFFFF, unsigned char boxColor = 63, unsigned char spacing = 2);
