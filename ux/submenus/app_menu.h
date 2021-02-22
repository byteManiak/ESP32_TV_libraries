#pragma once

#include <submenu.h>
#include <button.h>
#include <list.h>

class AppMenu : public Submenu
{
public:
    AppMenu(VGAExtended *vga, const char *title);
    void updateSubmenu();

private:
	enum appMenuState
	{
		APP_MENU_STATE_DEFAULT,
		APP_MENU_STATE_LOAD_INTERNAL,
		APP_MENU_STATE_REQUEST_LIST,
		APP_MENU_STATE_DOWNLOAD_BIN
	};

	enum activeWidget
	{
		LOAD_INTERNAL_BUTTON,
		DOWNLOAD_LIST_BUTTON,
		DOWNLOADED_LIST
	};

	appMenuState state = APP_MENU_STATE_DEFAULT;

	void receiveQueueData();

	Button *loadInternalButton;
	Button *downloadListButton;
	List<const char*> *downloadList;
};