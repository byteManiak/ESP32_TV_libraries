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
	enum radioMenuState
	{
		RADIO_MENU_STATE_DEFAULT,
		RADIO_MENU_STATE_REQUEST_LIST,
		RADIO_MENU_STATE_NOW_PLAYING
	};

	enum widgetPosition
	{
		NOW_PLAYING,
		RADIO_LIST
	};

	radioMenuState state = RADIO_MENU_STATE_DEFAULT;

	void receiveQueueData();

	List<char*> *radioStationList;
	Button *nowPlaying;
};