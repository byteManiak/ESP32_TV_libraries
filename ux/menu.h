#include <vector>
#include <VGA/VGA6Bit.h>
#include <util.h>

class Submenu
{
public:
	Submenu() {}
	~Submenu() {}

private:
	char title[64];
};

class Menu
{
public:
	Menu() {}
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
			currentSubmenu = (currentSubmenu == 0) ? numSubmenus-1 : currentSubmenu-1;
			// Trigger the radial menu to spin
			destination += sliceSize;
		}
		if (isKeyPressed(Down_key))
		{
			// Set the cursor on the next submenu
			currentSubmenu = (currentSubmenu + 1) % numSubmenus;
			// Trigger the radial menu to spin
			destination -= sliceSize;
		}

		// Establish the current speed of the scroll
		double distance = abs(position - destination);
		if (distance > 0.001)
		{
			// Scale the scroll speed with the distance for a smooth animation
			scrollSpeed = distance * timeDelta * scrollFactor;
		}

		// Move radial menu towards the new destination
		if (position > destination)
			position -= scrollSpeed;
		else position += scrollSpeed;

		// Draw the radial menu
		for (int i = 0; i < 10; i++)
		{
			double s = sin(position + i * sliceSize);
			double c = cos(position + i * sliceSize);
			double x1, x2, y1, y2;

			x1 = vga->xres/16 * s;
			y1 = vga->yres/2 + vga->yres/16 * c;
			x2 = vga->xres/1.5f * s;
			y2 = vga->yres/2 + vga->yres/1.5f * c;
			vga->line(x1, y1, x2, y2, vga->RGB(0x00FFAA));
		}

		vga->setCursor(vga->xres/2 + vga->xres/6, vga->yres/2 - vga->font->charHeight);
	}

private:
	// Actual menus
	uint16_t numSubmenus = 1;
	int32_t currentSubmenu = 0;
	std::vector<Submenu> submenus;

	// Used for drawing
	VGA6Bit *vga;
	// angle of the radial menu
	double theta;
	// used for smooth scrolling of the menu
	double position = 0, destination = 0;
	double scrollSpeed;
	const double scrollFactor = 3;
	// size of a menu "slice"
	const double sliceSize = pi2/10;
};
