#pragma once

#include <widget/widget.h>

class ProgressBar : public Widget
{
public:
	ProgressBar(VGAExtended *vga) : Widget(vga, 0, 0) {}

	void setPercentage(double percent) { this->percent = percent; }

	void draw(int16_t offsetX)
	{
		if (isVisible)
		{
			vga->drawRect(vga->xres/4-1, vga->yres/2-5, vga->xres/2+2, 10, BLACK, true);
			vga->drawRect(vga->xres/4, vga->yres/2-4, vga->xres/2 * percent, 8, vga->getPercentGradient(percent), true);
		}
	}

	void update() { status = 0; }

private:
	double percent;
};