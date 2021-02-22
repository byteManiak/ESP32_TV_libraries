#include <submenus/radio_menu.h>

RadioMenu::RadioMenu(VGAExtended *vga, const char *title) : Submenu(vga, title)
{
	nowPlaying = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
	new (nowPlaying) Button(vga, "Not connected", vga->xres/2, vga->yres/48);
	widgets.push_back(nowPlaying);

	radioStationList = heap_caps_malloc_cast<List<char*>>(MALLOC_CAP_PREFERRED);
	new (radioStationList) List<char*>(vga, vga->xres/2, vga->yres/16, 16);
	widgets.push_back(radioStationList);

	setFocusedWidget(NOW_PLAYING);
}

void RadioMenu::receiveQueueData()
{
	if (queueRx)
	{

	}
}

void RadioMenu::updateSubmenu()
{
	
}