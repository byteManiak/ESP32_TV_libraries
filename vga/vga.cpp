#include <vga/vga.h>

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

void VGAExtended::printBox(const char *text, int x, int y, unsigned char textColor, unsigned char borderColor, unsigned char fillColor, unsigned char spacing)
{
	// Get original cursor and colors
	long cx = this->cursorX, cy = this->cursorY;
	long f = this->frontColor, b = this->backColor;

	// Get length of text to establish size of box
	unsigned int len = strnlen(text, 64);

	// Display box and text
	if (fillColor != 255) drawRect(x+1, y+1, len*8 + spacing*2 - 2, 6 + spacing*2, (unsigned char)fillColor, true);
	drawRect(x, y, len*8 + spacing*2, 8 + spacing*2, borderColor);
	setCursor(x+spacing, y+spacing);
	setTextColor(textColor, backColor);
	drawText(text);

	// Restore original cursor and colors
	setTextColor(f, b);
	setCursor(cx, cy);
}

void VGAExtended::drawLine(int x1, int y1, int x2, int y2, unsigned char color)
{
	if (frameBufferCount == 1)
	{
		DrawableLine *l = heap_caps_malloc_cast<DrawableLine>(MALLOC_CAP_PREFERRED);
		l->type = DRAWABLE_LINE;
		l->x1 = x1; l->y1 = y1; l->x2 = x2; l->y2 = y2; l->color = color;
		nextFrameDrawables.push_back((Drawable*)l);
	}
	else line(x1, y1, x2, y2, color);
}

void VGAExtended::drawText(const char *text)
{
	if (frameBufferCount == 1)
	{
		DrawableText *t = heap_caps_malloc_cast<DrawableText>(MALLOC_CAP_PREFERRED);
		t->type = DRAWABLE_TEXT;
		t->x = cursorX; t->y = cursorY; t->color = frontColor;
		strlcpy(t->text, text, 65);
		cursorX += strlen(t->text) * font->charWidth;
		nextFrameDrawables.push_back((Drawable*)t);
	}
	else print(text);
}

void VGAExtended::drawRect(int x, int y, int w, int h, unsigned char color, bool doFillRect)
{
	if (frameBufferCount == 1)
	{
		DrawableRect *r = heap_caps_malloc_cast<DrawableRect>(MALLOC_CAP_PREFERRED);
		r->type = DRAWABLE_RECT;
		r->x = x; r->y = y; r->w = w; r->h = h; r->color = color; r->fillRect = doFillRect;
		nextFrameDrawables.push_back((Drawable*)r);
	}
	else
	{
		if (!doFillRect) rect(x, y, w, h, color);
		else fillRect(x, y, w, h, color);
	}
}

void VGAExtended::drawFloat(float f)
{
	if (frameBufferCount == 1)
	{
		char text[65];
		sprintf(text, "%.2f", f);
		drawText(text);
	}
	else print(f);
}

void VGAExtended::showDrawables()
{
	if (frameBufferCount == 2)
	{
		show();
		return;
	}

	// Assume that the contents on screen did not change
	// in order to avoid unneeded draw calls
	bool drawablesUnchanged = true;

	/** Check if any of the contents on screen changed **/

	// If the vector sizes are not equal, then clearly something must have changed
	if (prevFrameDrawables.size() != nextFrameDrawables.size()) drawablesUnchanged = false;

	else for(int i = 0; i < prevFrameDrawables.size(); i++)
	{
		Drawable *d1 = prevFrameDrawables[i];
		Drawable *d2 = nextFrameDrawables[i];
		// At all times, the objects on screen are drawn in the same order
		// Checking if the types are different means the order changed,
		// and therefore new elements are being drawn
		if (d1->type != d2->type)
		{
			drawablesUnchanged = false;
			break;
		}

		// If both objects are lines, check that they have the same coordinates and color
		if (d1->type == DRAWABLE_LINE)
		{
			DrawableLine *l1 = (DrawableLine*)d1;
			DrawableLine *l2 = (DrawableLine*)d2;
			if ((l1->color != l2->color) ||
				(l1->x1 != l2->x1) || (l1->y1 != l2->y1) ||
				(l1->x2 != l2->x2) || (l1->y2 != l2->y2))
			{
				drawablesUnchanged = false;
				break;
			}
		}
		// If both objects are text, check that they have the same position, text, and color
		else if (d1->type == DRAWABLE_TEXT)
		{
			DrawableText *t1 = (DrawableText*)d1;
			DrawableText *t2 = (DrawableText*)d2;
			if (t1->x != t2->x || t1->y != t2->y || t1->color != t2->color ||
				strncmp(t1->text, t2->text, 64) != 0)
			{
				drawablesUnchanged = false;
				break;
			}
		}
		// If both objects are rectangles, check that they have the same coordinates and color
		else if (d1->type == DRAWABLE_RECT)
		{
			DrawableRect *r1 = (DrawableRect*)d1;
			DrawableRect *r2 = (DrawableRect*)d2;
			if (r1->color != r2->color || r1->fillRect != r2->fillRect ||
				r1->x != r2->x || r1->y != r2->y ||
				r1->w != r2->w || r1->h != r2->h)
			{
				drawablesUnchanged = false;
				break;
			}
		}
	}

	// Don't redraw screen contents if they did not change
	// Because this frame is being "skipped", the contents of
	// the next frame are passed to the previous one, virtually
	// advancing the frame by 1
	if (drawablesUnchanged)
	{
		// Empty the previous frame's contents first
		// as they are no longer needed now
		for (Drawable *d : prevFrameDrawables) heap_caps_free(d);
		prevFrameDrawables.clear();
		// Move contents to the previous frame
		std::swap(prevFrameDrawables, nextFrameDrawables);
		return;
	}

	// The repainting algorithm takes the previously drawn frame contents
	// and draws them again using the background color
	// This heavily reduces DRAM usage by not needing a backbuffer into
	// which to draw elements without screen tearing
	for (Drawable *d : prevFrameDrawables)
	{
		if (!d) continue;

		switch (d->type)
		{
			case DRAWABLE_LINE:
			{
				DrawableLine *l = (DrawableLine*)d;
				line(l->x1, l->y1, l->x2, l->y2, backColor);

				break;
			}
			case DRAWABLE_TEXT:
			{
				DrawableText *t = (DrawableText*)d;
				setCursor(t->x, t->y);
				setTextColor(backColor, backColor);
				print(t->text);

				break;
			}
			case DRAWABLE_RECT:
			{
				DrawableRect *r = (DrawableRect*)d;
				if (r->fillRect) fillRect(r->x, r->y, r->w, r->h, backColor);
				else rect(r->x, r->y, r->w, r->h, backColor);

				break;
			}
		}
		// Empty the vector as we are traversing through it
		heap_caps_free(d);
	}
	prevFrameDrawables.clear();

	// The repainting algorithm then draws the contents of the next frame
	// over what is now essentially a blank canvas which has the background color
	// This eliminates the need to call vga->clear(), which heavily increases
	// screen tearing when not using a backbuffer
	for (Drawable *d : nextFrameDrawables)
	{
		switch (d->type)
		{
			case DRAWABLE_LINE:
			{
				DrawableLine *l = (DrawableLine*)d;
				line(l->x1, l->y1, l->x2, l->y2, l->color);
				break;
			}
			case DRAWABLE_TEXT:
			{
				DrawableText *t = (DrawableText*)d;
				setCursor(t->x, t->y);
				setTextColor(t->color, backColor);
				print(t->text);
				break;
			}
			case DRAWABLE_RECT:
			{
				DrawableRect *r = (DrawableRect*)d;
				if (r->fillRect) fillRect(r->x, r->y, r->w, r->h, r->color);
				else rect(r->x, r->y, r->w, r->h, r->color);
				break;
			}
		}
	}
	// Move contents to the previous frame
	std::swap(prevFrameDrawables, nextFrameDrawables);
}