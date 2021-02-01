#pragma once

#include <VGA/VGA6Bit.h>


class VGAExtended : public VGA6Bit
{
public:
    VGAExtended() : VGA6Bit() {}

    long getPercentGradient(double percent);
    void fillRectAlpha(int x, int y, int w, int h, unsigned char color);
    void printBox(const char *text, int x, int y, long textColor = 0xFFFFFFFF, unsigned char boxColor = 63, unsigned char spacing = 2);
    void echoPassword(int numChars, int x, int y, long textColor = 0xFFFFFFFF);
};