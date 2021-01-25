#include <stdlib.h>
#include <time.h>
#include <Arduino.h>
#include <ESP32Lib.h>
#include <Ressources/CodePage437_8x8.h>
#include <alloc.h>
#include <ps2.h>
#include <stats.h>

#if defined(CONFIG_DEBUG_VGA_PROJ)
#include <textbuf.h>
TextBuf textbuf;
#endif

VGA6Bit vga;
int &xres = vga.xres;
int &yres = vga.yres;

const int r[] = {13, 12}, g[] = {14, 27}, b[] = {33, 32};

void setup()
{
	srand(time(NULL));

	// Initialize VGA display

	// Enable double buffering for non-tearing video
	vga.setFrameBufferCount(2);
	// Set display mode to 425x240
	vga.init(vga.MODE640x480.custom(425,240), r, g, b, 22, 23);
	// Use IBM BIOS font
	vga.setFont(CodePage437_8x8);

	// Initialize PS/2 keyboard communication
	Serial2.begin(12000, SERIAL_8E1, 36, -1);

#if defined(CONFIG_DEBUG_VGA_PROJ)
	// Initialize the text view for debug builds
	textbuf.setVGAController(&vga);
#endif
}

void loop()
{
	vga.clear(32);

	// Get PS/2 keyboard state
    updateKeyboard();

	// Draw a Tempest-style wireframe cylinder
	static double lastx1 = 0, lasty1 = 0;
	static double lastx2 = 0, lasty2 = 0;
	for(int i = 0; i < 10; i++)
	{
		const double pi2 = 3.14159 * 2;
		//double time = millis()/1000.f;
		static double time = 1;
		double s = sin(time+i*pi2/10);
		double c = cos(time+i*pi2/10);
		double x1,y1,x2,y2;

		if (isKeyDown(Left_key))
			time += .01f;
		x1 = xres/2 + xres/16 * s;
		y1 = yres/2 + yres/16 * c;
		x2 = xres/2 + xres/3 * s;
		y2 = yres/2 + yres/3 * c;
		vga.line(x1,y1,x2,y2,vga.RGB(0x00FFAA));
		vga.line(x1,y1,lastx1,lasty1,vga.RGB(0xFFFFFF));
		vga.line(x2,y2,lastx2,lasty2,vga.RGB(0xFFFFFF));

		lastx1 = x1; lasty1 = y1;
		lastx2 = x2; lasty2 = y2;
	}

#if defined(CONFIG_DEBUG_VGA_PROJ)
	// Show error messages from text view
	textbuf.update();
	// Display memory statistics
	printMemStats(vga);
#endif

#if defined(CONFIG_DEBUG_SHOW_FPS)
	// Display FPS counter
	printFPS(vga);
#endif

	// Display backbuffer
	vga.show();
}
