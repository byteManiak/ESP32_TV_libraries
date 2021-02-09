#pragma once

#include <vga.h>
#include <submenu.h>

#include <button.h>
#include <list.h>
#include <textbox.h>

class WifiMenu : public Submenu
{
public:
    WifiMenu(VGAExtended *vga, const char *title);
    void updateSubmenu();

private:
    List<const char*> *ssidList;
    Button *actionButton;
    Textbox *passwordTextbox;
    Button *ipDetails, *gatewayDetails;

    enum wifiMenuState
    {
        WIFI_MENU_STATE_DEFAULT,
        WIFI_MENU_STATE_WAITING,
        WIFI_MENU_STATE_CHOOSE_SSID,
        WIFI_MENU_STATE_QUERY_PASSWORD,
        WIFI_MENU_STATE_CONNECTED,
        WIFI_MENU_STATE_DISCONNECTED
    };

    wifiMenuState state = WIFI_MENU_STATE_DEFAULT;

    void receiveQueueData();

    // List of SSIDs to choose from after scan event
	int selectedSsid = 0;
	int ssidCount = 0;
	char ssids[8][33];

    // Password to send to wifi task
	char password[65];
	int passwordLength = 0;

    // IP and gateway addresses
    char ipAddress[16], gatewayAddress[16];
};