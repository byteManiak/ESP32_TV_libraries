#pragma once

#include <submenu.h>
#include <button.h>
#include <list.h>

/*struct RadioStation
{
	const char *url;
	const char *name;
};

RadioStation stations[] = 
{

};*/

class RadioMenu : public Submenu
{
public:
    RadioMenu(VGAExtended *vga, const char *title);
    void updateSubmenu();

private:
	List<char*> *radioStationList;
	Button *connectionStatus;

	enum radioMenuState
	{
		RADIO_MENU_STATE_WAITING_WIFI,
		RADIO_MENU_STATE_CONNECTED,
		RADIO_MENU_STATE_REQUEST_LIST,
		RADIO_MENU_STATE_DISPLAY_LIST,
		RADIO_MENU_STATE_LIST_UPDATING
	};

	enum widgetPosition
	{
		CONNECTION_STATUS,
		RADIO_LIST
	};

	radioMenuState state = RADIO_MENU_STATE_WAITING_WIFI;

	void receiveQueueData();

	// Track the current page number from the remote radio list
	uint16_t radioPageNum = 0;
	uint16_t radioPageNumMax = 0xFFFF;
};