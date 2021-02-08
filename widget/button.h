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
        if (isKeyPressed(Enter_key)) buttonPushed = true;
        else if(isKeyReleased(Enter_key)) buttonPushed = false;

        if (isFocused)
        {
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
        if (isVisible)
        {
            unsigned char fillColor = 255, textColor = 63;
            if (isFocused && buttonPushed) { fillColor = 63; textColor = 0; }
            vga->printBox(text, baseX + offsetX, baseY, textColor, textColor, fillColor, 4);
        }
    }
private:
    bool buttonPushed = false;
    char text[32];
};