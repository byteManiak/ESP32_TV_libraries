#pragma once

#include <alloc.h>
#include <submenus/submenus.h>
#include <util.h>
#include <ux.h>

class Menu
{
public:
	Menu(VGAExtended *vga);
    ~Menu() = default;

	void addSubMenu(Submenu *submenu);
	void drawMenu();

private:
	// Actual menus
	heap_caps_vector<Submenu*> submenus;
	uint16_t currentSubmenu = 0;

	// Helper to draw the correct menu strings on screen
	uint8_t radialMenuPos = 0;

	// Used for drawing
	VGAExtended *vga;
	// angle of the radial menu
	double theta;
	// used for smooth scrolling of the menu
	double position = 0, destination = 0;
	double scrollSpeed;
	const double scrollFactor = 3;
	// size of a menu "slice"
	static constexpr double sliceSize = pi2/8;
	// used for moving the menu aside when inside a submenu
	bool usingSubmenu = false;
	double offsetPosition = 0, offsetDestination = 0;
	double offsetScrollSpeed;
};
