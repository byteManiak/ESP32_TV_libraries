#include <stdlib.h>
#include <time.h>
#include <esp_event.h>
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

Menu *menu;

void loop();

extern "C" void app_main()
{
	srand(time(NULL));

	// Initialize VGA display
	// Enable double buffering for non-tearing video
	vga.setFrameBufferCount(2);
	// Set display mode to 425x240
	vga.init(vga.MODE640x480.custom(425,240), r, g, b, 22, 23);
	// Use IBM BIOS font
	vga.setFont(CodePage437_8x8);

#if defined(CONFIG_DEBUG_VGA_PROJ)
	// Initialize the text view for debug builds
	textview.setVGAController(&vga);
#endif
	menu = (Menu*)heap_caps_malloc(sizeof(Menu), MALLOC_CAP_PREFERRED);
	new (menu) Menu();
	menu->setVGAController(&vga);

	for(;;) loop();
}

void loop()
{
	// Get time spent to render last frame
	calculateTimeDelta();

	vga.clear(22);

	// Get PS/2 keyboard state
    updateKeyboard();

	menu->drawMenu();

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

	vTaskDelay(10 / portTICK_PERIOD_MS);
}
