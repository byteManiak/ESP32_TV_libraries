#pragma once

#include <ps2.h>

#include <widget.h>

class Textbox : public Widget
{
public:
	Textbox(VGAExtended *vga, bool textHidden = false)
		: Widget(vga, 0, 0), textHidden{textHidden} {memset(text, 0, 65);}

	int8_t update()
	{
		if (!isFocused) return 0;

		char ascii = getLastAsciiKey();
		if (ascii >= 32 && ascii <= 126)
		{
			text[textCursorPos++] = ascii;
			if (textCursorPos > 63) textCursorPos = 63;
			text[textCursorPos+1] = '\0';
		}

		if (isKeyPressed(Backspace_key))
		{
			textCursorPos = textCursorPos > 0 ? textCursorPos-1 : 0;
			text[textCursorPos] = '\0';
		}

		if (isKeyPressed(Enter_key))
		{
			text[textCursorPos+1] = '\0';
			return 1;
		}

		return 0;
	}

	const char *getText()
	{
		return text;
	}

    void setFocused(bool focus)
    {
        isFocused = focus;
		if (focus)
		{
        	textCursorPos = 0;
        	strcpy(text, "");
		}
    }

	void draw(int16_t offsetX)
	{
		if (isFocused)
		{
			int len = strnlen(text, 65);
			vga->printBox(text, vga->xres/2 - len/2.f * vga->font->charWidth - 4, vga->yres/2 - vga->font->charHeight/2 - 4);
		}
	}
private:
	bool textHidden;
	uint8_t textCursorPos = 0;
	char text[65];
};