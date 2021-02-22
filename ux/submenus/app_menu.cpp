#include <submenus/app_menu.h>

AppMenu::AppMenu(VGAExtended *vga, const char *title) : Submenu(vga, title)
{
	loadInternalButton = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
	new (loadInternalButton) Button(vga, "Load downloaded app", 0, 0);
	widgets.push_back(loadInternalButton);

	downloadListButton = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
	new (downloadListButton) Button(vga, "Get list of apps", 0, 0);
	widgets.push_back(downloadListButton);

	downloadList = heap_caps_malloc_cast<List<const char*>>(MALLOC_CAP_PREFERRED);
	new (downloadList) List<const char*>(vga, 0, 0, 8);
	widgets.push_back(downloadList);

	setFocusedWidget(LOAD_INTERNAL_BUTTON);
}

void AppMenu::receiveQueueData()
{
	if (queueRx)
	{

	}
}

void AppMenu::updateSubmenu()
{
	
}