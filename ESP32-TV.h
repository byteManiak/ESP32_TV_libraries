#pragma once

#include <io/ps2.h>
#include <io/sound.h>

#include <memory/alloc.h>

#include <net/wifi.h>
#include <net/http.h>

#include <vga/vga.h>

#include <util/log.h>
#include <util/numeric.h>
#include <util/queues.h>

#include <stats/stats.h>

/**
 * @brief Initialize keyboard, VGA and miscellaneous features that are used across all apps
 * @param vga VGA handle to initialize.
 * @param xres Horizontal resolution
 * @param yres Vertical resolution
 * @param r1 Red pin low
 * @param r2 Red pin high
 * @param g1 Green pin low
 * @param g2 Green pin high
 * @param b1 Blue pin low
 * @param b2 Blue pin high
 * @param hsync HSync pin
 * @param vsync VSync pin
 * @param fbcount Number of framebuffers
 **/
void initCommon(VGAExtended **vga, int xres, int yres, int r1, int r2, int g1, int g2, int b1, int b2, int hsync, int vsync, int fbcount = 2);

/**
 * @brief Initialize keyboard, VGA and miscellaneous features for current downloadable app
 * @param vga VGA handle to initialize.
 * @param xres Horizontal resolution
 * @param yres Vertical resolution
 * @param r1 Red pin low
 * @param r2 Red pin high
 * @param g1 Green pin low
 * @param g2 Green pin high
 * @param b1 Blue pin low
 * @param b2 Blue pin high
 * @param hsync HSync pin
 * @param vsync VSync pin
 * @param fbcount Number of framebuffers
 **/
void initClientApp(VGAExtended **vga, int xres, int yres, int r1, int r2, int g1, int g2, int b1, int b2, int hsync, int vsync, int fbcount = 2);

/**
 * @brief Closes the currently downloaded app and reboots back to the main menu
 **/
void closeApp();

/**
 * @brief Initialize Wifi and HTTP stacks.
 **/
void initNetwork(esp_err_t httpHandler(esp_http_client_event_t *event));

extern "C" const char tvAppName[64];