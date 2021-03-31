#pragma once

#include <string.h>

#include <io/ps2.h>
#include <util/queues.h>
#include <util/numeric.h>

#include <widget.h>

class Button : public Widget
{
public:
	Button(VGAExtended *vga, const char *text, int16_t x, int16_t y)
		: Widget(vga, x, y) {strlcpy(this->text, text, 64);}

	/**
	 * @brief Update this widget's state.
	 */
	void update()
	{
		// This boolean is separate from buttonPushed to avoid
		// considering the status of the button as pressed when the
		// Enter key is held down. This is done to avoid sending
		// messages to the low-level queues on every frame.
		bool buttonPressed = false;
		if (isKeyPressed(Enter_key)) { buttonPressed = true; buttonPushed = true; }
		else if(isKeyReleased(Enter_key)) buttonPushed = false;

		if (isFocused) status = buttonPressed;
		else status = 0;
	}

	/**
	 * @brief Set the text that is shown inside the button.
	 * @param text Text to show inside the button.
	 */
	void setText(const char *text)
	{
		strlcpy(this->text, text, 64);
	}

	/**
	 * @brief Set the color that is shown behind the text of the button.
	 * @param color Color to show behind the text.
	 */
	void setFillColor(VGAColor color)
	{
		fillColor = color;
	}

	/**
	 * @brief Draw this widget on the screen
	 * @param offsetX Offset of the entire submenu when lerp'ing it around.
	 */
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
	char text[64];
};