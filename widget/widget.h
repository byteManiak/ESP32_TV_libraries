#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <vga.h>

class Widget
{
public:
    Widget(VGAExtended *vga, int16_t x, int16_t y)
        : vga{vga}, baseX{x}, baseY{y} {}
    virtual int8_t update() = 0;
    virtual void draw(int16_t offsetX);

	void setFocused(bool focus) {isFocused = focus;}
    void setVisible(bool visible) {isVisible = visible;}

protected:
    bool isFocused = false;
    bool isVisible = true;

    VGAExtended *vga;
    int16_t baseX, baseY;
};