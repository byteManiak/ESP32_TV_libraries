#include <stdlib.h>
#include <time.h>
#include <Arduino.h>
#include <ESP32Lib.h>
#include <Ressources/CodePage437_8x8.h>
#include <alloc.h>
#include <ps2.h>
#include <stats.h>
#include <util.h>
#include <ux.h>
#include <menu.h>

#if defined(CONFIG_DEBUG_VGA_PROJ)
#include <textview.h>
TextView textview;
#endif

VGA6Bit vga;
int &xres = vga.xres;
int &yres = vga.yres;

const int r[] = {13, 12}, g[] = {14, 27}, b[] = {33, 32};

Menu menu;

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
	textview.setVGAController(&vga);
#endif
	menu.setVGAController(&vga);
}

void loop()
{
	// Get time spent to render last frame
	calculateTimeDelta();

	vga.clear(22);

	// Get PS/2 keyboard state
    updateKeyboard();

	menu.drawMenu();

#if defined(CONFIG_DEBUG_VGA_PROJ)
	// Show error messages from text view
	textview.update();
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
