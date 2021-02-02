#include <stdlib.h>
#include <time.h>
#include <esp_event.h>
#include <ESP32Lib.h>
#include <Ressources/CodePage437_8x8.h>
#include <alloc.h>
#include <ps2.h>
#include <stats.h>
#include <util.h>
#include <vga.h>
#include <menu.h>
#include <wifi.h>

VGAExtended *vga;

const int r[] = {13, 12}, g[] = {14, 27}, b[] = {33, 32};

Menu *menu;
WifiMenu *wifiMenu;

void loop();

extern "C" void app_main()
{
	srand(time(NULL));

	esp_event_loop_create_default();
	initKeyboard();
	initWifi();

	vga = heap_caps_malloc_construct<VGAExtended>(MALLOC_CAP_PREFERRED);

	// Initialize VGA display
	// Enable double buffering for non-tearing video
	vga->setFrameBufferCount(2);
	// Set display mode to 425x240
	vga->init(vga->MODE640x480.custom(425,240), r, g, b, 22, 23);
	// Use IBM BIOS font
	vga->setFont(CodePage437_8x8);

	menu = heap_caps_malloc_construct<Menu, VGAExtended*>(MALLOC_CAP_PREFERRED, vga);
	wifiMenu = heap_caps_malloc_construct<WifiMenu, VGAExtended*, const char*>(MALLOC_CAP_PREFERRED, vga, "Network");
	wifiMenu->attachQueues(wifiQueueRx, wifiQueueTx);
	menu->addSubMenu(wifiMenu);
	for(;;) loop();
}

void loop()
{
	// Get time spent to render last frame
	calculateTimeDelta();

	// Get PS/2 keyboard state
    updateKeyboard();

	vga->clear(22);

	menu->drawMenu();

#if defined(CONFIG_DEBUG_VGA_PROJ)
	// Display memory statistics
	printMemStats(vga);
#endif

#if defined(CONFIG_DEBUG_SHOW_FPS)
	// Display FPS counter
	printFPS(vga);
#endif

	// Display backbuffer
	vga->show();

	vTaskDelay(10 / portTICK_PERIOD_MS);
}
