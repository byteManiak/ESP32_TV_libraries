#include <vector>
#include <string.h>
#include <VGA/VGA6Bit.h>
#include <util.h>

class Submenu
{
public:
	Submenu(const char *title) {strncpy(this->title, title, 64);}
	~Submenu() {}
	char title[64];
};

class Menu
{
public:
	Menu()
	{
		submenus.push_back(Submenu("1 menu Test"));
		submenus.push_back(Submenu("2 menu Test"));
		submenus.push_back(Submenu("3 menu test"));
		submenus.push_back(Submenu("4 menu test"));
		submenus.push_back(Submenu("5 menu test"));
		submenus.push_back(Submenu("6 menu test"));
		submenus.push_back(Submenu("7 menu test"));
		submenus.push_back(Submenu("8 menu test"));
		submenus.push_back(Submenu("9 menu test"));
		submenus.push_back(Submenu("10 menu test"));
		submenus.push_back(Submenu("11 menu test"));
		submenus.push_back(Submenu("12 menu test"));
		submenus.push_back(Submenu("13 menu test"));
		submenus.push_back(Submenu("14 menu test"));
		submenus.push_back(Submenu("15 menu test"));
	}
	~Menu() {}

	void setVGAController(VGA6Bit *vga)
	{
		this->vga = vga;
	}

	void addSubMenu()
	{

	}

	void drawMenu()
	{
		if (isKeyPressed(Up_key))
		{
			// Set the cursor on the next submenu
			currentSubmenu = getPrevSubmenu(currentSubmenu);
			radialMenuPos = getNextRadialPos(radialMenuPos);
			// Trigger the radial menu to spin
			destination += sliceSize;
		}
		if (isKeyPressed(Down_key))
		{
			// Set the cursor on the next submenu
			currentSubmenu = getNextSubmenu(currentSubmenu);
			radialMenuPos = getPrevRadialPos(radialMenuPos);
			// Trigger the radial menu to spin
			destination -= sliceSize;
		}

		// Establish the current speed of the scroll
		double distance = abs(position - destination);
		if (distance > 0.001)
		{
			// Scale the scroll speed with the distance for a smooth animation
			scrollSpeed = distance * timeDelta * scrollFactor;
			// Move radial menu towards the new destination
			if (position > destination)
				position -= scrollSpeed;
			else position += scrollSpeed;
		}

		// Draw the radial menu
		for (uint8_t i = 0; i < 8; i++)
		{
			double s = sin(position + i * sliceSize);
			double c = cos(position + i * sliceSize);
			double x1, x2, y1, y2;

			x1 = vga->xres/16 * c;
			y1 = vga->yres/2 + vga->yres/16 * s;
			x2 = vga->xres/2.5f * c;
			y2 = vga->yres/2 + vga->yres/2.5f * s;
			vga->line(x1, y1, x2, y2, vga->RGB(0x00FFAA));

			// Get the position of the current "needle"
			uint8_t offsetPos = (i + radialMenuPos) % 8;
			uint16_t printSubmenu;
			// If on the bottom 3 "needles" visible on the screen
			if (offsetPos <= 2)
			{
				printSubmenu = (currentSubmenu + offsetPos) % submenus.size();
			}
			else if (offsetPos >= 6)
			{
				int32_t printSubmenuTemp;
				offsetPos = 8 - offsetPos;
				printSubmenuTemp = currentSubmenu - offsetPos;
				if (printSubmenuTemp < 0) printSubmenuTemp += submenus.size();
				printSubmenu = printSubmenuTemp % submenus.size();
			}

			vga->setCursor(x2-(x2-x1)/3, y2 - vga->font->charHeight/2);
			vga->print(submenus[printSubmenu].title);
		}
	}

private:
	// Actual menus
	std::vector<Submenu> submenus;
	uint16_t currentSubmenu = 0;

	// Helpers to draw the correct menu strings on screen
	uint16_t getNextSubmenu(uint16_t submenu)
	{
		return currentSubmenu < submenus.size()-1 ? currentSubmenu+1 : 0;
	}
	uint16_t getPrevSubmenu(uint16_t submenu)
	{
		return currentSubmenu > 0 ? currentSubmenu-1 : submenus.size()-1;
	}

	// Helpers to draw the correct menu strings on screen
	uint8_t radialMenuPos = 0;
	uint8_t getNextRadialPos(uint8_t radialPos)
	{
		return radialMenuPos < 7 ? radialMenuPos+1 : 0;
	}
	uint8_t getPrevRadialPos(uint8_t radialPos)
	{
		return radialMenuPos > 0 ? radialMenuPos-1 : 7;
	}

	// Used for drawing
	VGA6Bit *vga;
	// angle of the radial menu
	double theta;
	// used for smooth scrolling of the menu
	double position = 0, destination = 0;
	double scrollSpeed;
	const double scrollFactor = 3;
	// size of a menu "slice"
	const double sliceSize = pi2/8;
};
