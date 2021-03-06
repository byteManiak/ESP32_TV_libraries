/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

#include <menu.h>

#include <io/ps2.h>
#include <util/numeric.h>

Menu::Menu(VGAExtended *vga, VGAColor activeColor, VGAColor inactiveColor)
	: vga{vga}, activeColor{activeColor}, inactiveColor{inactiveColor} {}

void Menu::addSubMenu(Submenu *submenu)
{
	submenus.push_back(submenu);
}

void Menu::drawMenu()
{
	for(int i = 0; i < submenus.size(); i++)
	{
		submenus[i]->updateSubmenu();
		submenus[i]->drawSubmenu();
	}

	if (!usingSubmenu)
	{
		if (isKeyPressed(Enter_key))
		{
			submenus[currentSubmenu]->setActiveSubmenu(true);
			usingSubmenu = !usingSubmenu;
		}

		offsetDestination = 0;

		if (isKeyPressed(Up_key))
		{
			// Set the cursor on the next submenu
			currentSubmenu = getPrevInt(currentSubmenu, submenus.size());
			radialMenuPos = getNextInt(radialMenuPos, 8);
			// Trigger the radial menu to spin
			destinationAngle += sliceSize;
		}
		if (isKeyPressed(Down_key))
		{
			// Set the cursor on the next submenu
			currentSubmenu = getNextInt(currentSubmenu, submenus.size());
			radialMenuPos = getPrevInt(radialMenuPos, 8);
			// Trigger the radial menu to spin
			destinationAngle -= sliceSize;
		}
	}
	else
	{
		if (isKeyPressed(ESC_key))
		{
			submenus[currentSubmenu]->setActiveSubmenu(false);
			usingSubmenu = !usingSubmenu;
		}

		offsetDestination = -vga->xres/4;
	}

	// Establish current angle of the menu
	smoothLerp(currentAngle, destinationAngle);

	// Establish offset currentAngle of the menu
	smoothLerp(offsetPosition, offsetDestination);

	vga->setTextColor(WHITE, vga->backColor);

	// Draw the radial menu
	for (uint8_t i = 0; i < 8; i++)
	{
		// Get the position of the current "needle"
		uint8_t offsetPos = (i + radialMenuPos) % 8;
		double s = sin(currentAngle + i * sliceSize);
		double c = cos(currentAngle + i * sliceSize);
		double x1, x2, y1, y2;
		unsigned char color = inactiveColor;

		// Change color of center "needle"
		if (offsetPos == 0) color = activeColor;

		if ((!usingSubmenu) || (usingSubmenu && offsetPos == 0))
		{
			x1 = vga->xres/16 * c + offsetPosition;
			y1 = vga->yres/2 + vga->yres/16 * s;
			x2 = vga->xres/2.5f * c + offsetPosition;
			y2 = vga->yres/2 + vga->yres/2.5f * s;
			vga->drawLine(x1, y1, x2, y2, color);

			uint16_t printSubmenu = 0;
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

			vga->setCursor(x2-(x2-x1)/3, y2 - vga->font->charHeight);
			submenus[printSubmenu]->drawTitle();
		}
	}
}