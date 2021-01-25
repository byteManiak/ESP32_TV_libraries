#include <ux.h>

long getPercentGradient(VGA6Bit &vga, double percent)
{
	// Get a gradient color based on a percent from 0 to 100
	// If the percent is under 50, then the color is predominantly red
	// Otherwise, the color is predominantly green
	// Color format is 0xBBGGRR
	if (percent < .5) return vga.RGB(0x0000FF + (int(2.*percent*0xFF) << 8));
	else return vga.RGB(0x00FF00 + (2.*(1.-percent)*0xFF));
}

void fillRectAlpha(VGA6Bit &vga, int x, int y, int w, int h, unsigned char color)
{
	for(int i = x; i < x + w; i++)
		for(int j = y; j < y + h; j++)
			vga.dotMix(i,j,color);
}
