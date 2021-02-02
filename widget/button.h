#pragma once

#include <string.h>

#include <ps2.h>

#include <widget.h>

class Button : public Widget
{
public:
    Button(VGAExtended *vga, const char *text, int16_t x, int16_t y)
        : Widget(vga, x, y) {strlcpy(this->text, text, 32);}

    int8_t update()
    {
        if (isFocused)
        {
            buttonPushed = isKeyPressed(Enter_key);
            return buttonPushed;
        }
        return 0;
    }

    void setText(const char *text)
    {
        strlcpy(this->text, text, 32);
    }

    void draw(int16_t offsetX)
    {
        char fillColor = -1, textColor = vga->RGB(0xFFFFFF);
        if (buttonPushed) { fillColor = vga->RGB(0xFFFFFF); textColor = vga->RGB(0); }
        vga->printBox(text, baseX + offsetX, baseY, textColor, textColor, fillColor, 4);
    }
private:
    bool buttonPushed = false;
    char text[32];
};