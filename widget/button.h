#pragma once

#include <string.h>

#include <ps2.h>
#include <util.h>

#include <widget.h>

class Button : public Widget
{
public:
    Button(VGAExtended *vga, const char *text, int16_t x, int16_t y)
        : Widget(vga, x, y) {strlcpy(this->text, text, 32);}

    void update()
    {
        // This boolean is separate from buttonPushed to avoid
        // considering the status of the button as pressed when the
        // Enter key is held down. This is done to avoid sending
        // messages to the low-level queues once every frame.
        bool buttonPressed = false;
        if (isKeyPressed(Enter_key)) { buttonPressed = true; buttonPushed = true; }
        else if(isKeyReleased(Enter_key)) buttonPushed = false;

        if (isFocused) status = buttonPressed;
        else status = 0;
    }

    void setText(const char *text)
    {
        strlcpy(this->text, text, 32);
    }

    void setFillColor(unsigned char color)
    {
        fillColor = color;
    }

    void draw(int16_t offsetX)
    {
        if (isVisible)
        {
            double to = baseX + offsetX - strlen(text)/2.f*vga->font->charWidth;
            smoothLerp(currentX, to);
            // Swap colors of text and background if button is pressed
            if (isFocused && buttonPushed)
                vga->printBox(text, currentX, baseY, fillColor, fillColor, textColor, 4);
            else
                vga->printBox(text, currentX, baseY, textColor, textColor, fillColor, 4);
        }
    }
private:
    unsigned char fillColor = 255, textColor = 63;
    bool buttonPushed = false;
    char text[32];
};