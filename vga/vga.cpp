#include <vga.h>
#include <string.h>

long VGAExtended::getPercentGradient(double percent)
{
	// Get a gradient color based on a percent from 0 to 100
	// If the percent is under 50, then the color is predominantly red
	// Otherwise, the color is predominantly green
	// Color format is 0xBBGGRR
	if (percent < .5) return RGB(0x0000FF + (int(2.*percent*0xFF) << 8));
	else return RGB(0x00FF00 + (2.*(1.-percent)*0xFF));
}

void VGAExtended::fillRectAlpha(int x, int y, int w, int h, unsigned char color)
{
	for(int i = x; i < x + w; i++)
		for(int j = y; j < y + h; j++)
			dotMix(i,j,color);
}

void VGAExtended::printBox(const char *text, int x, int y, long textColor, unsigned char borderColor, signed char fillColor, unsigned char spacing)
{
	// Get original cursor and colors
	long cx = this->cursorX, cy = this->cursorY;
	long f = this->frontColor, b = this->backColor;

	// Get length of text to establish size of box
	unsigned int len = strnlen(text, 64);

	// Display box and text
	if (fillColor != -1) fillRect(x+1, y+1, len*8 + spacing*2 - 2, 6 + spacing*2, (unsigned char)fillColor);
	rect(x, y, len*8 + spacing*2, 8 + spacing*2, borderColor);
	setCursor(x+spacing, y+spacing);
	setTextColor(RGB(0xFFFFFF));
	print(text);

	// Restore original cursor and colors
	setTextColor(f, b);
	setCursor(cx, cy);
}

void VGAExtended::echoPassword(int numChars, int x, int y, long textColor)
{
	setTextColor(textColor);
	for(int i = 0; i < numChars; i++)
	{
		setCursor(x + (i%32)*font->charWidth, y + (i/32)*font->charWidth);
		print('*');
	}
}
