#include <ESP32-TV.h>

#include <stdlib.h>
#include <time.h>

#include <esp_event.h>
#include <esp_ota_ops.h>

const char __attribute__((section(".rodata_custom_desc"))) tvAppName[64] = CONFIG_APP_NAME;

void initCommon(VGAExtended **vga, int xres, int yres, int r1, int r2, int g1, int g2, int b1, int b2, int hsync, int vsync, int fbcount)
{
	srand(esp_random());

	esp_event_loop_create_default();

	initKeyboard();

	*vga = heap_caps_malloc_construct<VGAExtended>(MALLOC_CAP_PREFERRED);

	// Initialize VGA display
	(*vga)->setFrameBufferCount(fbcount);
	// Set display mode
	int r[] = {r1, r2}, g[] = {g1, g2}, b[] = {b1, b2};
	(*vga)->init((*vga)->MODE640x480.custom(xres, yres), r, g, b, hsync, vsync);
}

void initClientApp(VGAExtended **vga, int xres, int yres, int r1, int r2, int g1, int g2, int b1, int b2, int hsync, int vsync, int fbcount)
{
	esp_ota_mark_app_valid_cancel_rollback();

	initCommon(vga, xres, yres, r1, r2, g1, g2, b1, b2, hsync, vsync, fbcount);
}

void closeApp()
{
	esp_ota_mark_app_invalid_rollback_and_reboot();
}

void initNetwork(esp_err_t httpHandler(esp_http_client_event_t *event))
{
	initWifi();
	initHTTP(httpHandler);
}