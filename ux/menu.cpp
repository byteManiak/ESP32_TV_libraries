#include <menu.h>
#include <ps2.h>
#include <util.h>

Menu::Menu(VGAExtended *vga)
{
    this->vga = vga;
}

void Menu::addSubMenu(Submenu *submenu)
{
    submenus.push_back(submenu);
}

void Menu::drawMenu()
{
    if (!usingSubmenu)
    {
        if (isKeyPressed(Enter_key))
        {
            usingSubmenu = !usingSubmenu;
        }

        offsetDestination = 0;

        if (isKeyPressed(Up_key))
        {
            // Set the cursor on the next submenu
            currentSubmenu = getPrevInt(currentSubmenu, submenus.size());
            radialMenuPos = getNextInt(radialMenuPos, 8);
            // Trigger the radial menu to spin
            destination += sliceSize;
        }
        if (isKeyPressed(Down_key))
        {
            // Set the cursor on the next submenu
            currentSubmenu = getNextInt(currentSubmenu, submenus.size());
            radialMenuPos = getPrevInt(radialMenuPos, 8);
            // Trigger the radial menu to spin
            destination -= sliceSize;
        }
    }
    else
    {
        if (isKeyPressed(ESC_key))
            usingSubmenu = !usingSubmenu;

        submenus[currentSubmenu]->drawSubmenu();
        offsetDestination = -vga->xres/4;
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

    // Establish the current speed of the scroll
    double offsetDistance = abs(offsetPosition - offsetDestination);
    if (offsetDistance > 0.001)
    {
        // Scale the scroll speed with the distance for a smooth animation
        offsetScrollSpeed = offsetDistance * timeDelta * scrollFactor;
        // Move radial menu towards the new destination
        if (offsetPosition > offsetDestination)
            offsetPosition -= offsetScrollSpeed;
        else offsetPosition += offsetScrollSpeed;
    }

    vga->setTextColor(vga->RGB(0xFFFFFF));
    // Draw the radial menu
    for (uint8_t i = 0; i < 8; i++)
    {
        // Get the position of the current "needle"
        uint8_t offsetPos = (i + radialMenuPos) % 8;
        double s = sin(position + i * sliceSize);
        double c = cos(position + i * sliceSize);
        double x1, x2, y1, y2;
        long color = 0x00FFAA;

        // Change color of center "needle"
        if (offsetPos == 0)
            color = 0x00AAFF;

        if ((!usingSubmenu) || (usingSubmenu && offsetPos == 0))
        {
            x1 = vga->xres/16 * c + offsetPosition;
            y1 = vga->yres/2 + vga->yres/16 * s;
            x2 = vga->xres/2.5f * c + offsetPosition;
            y2 = vga->yres/2 + vga->yres/2.5f * s;
            vga->line(x1, y1, x2, y2, vga->RGB(color));

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

            vga->setCursor(x2-(x2-x1)/3, y2 - vga->font->charHeight);
            submenus[printSubmenu]->drawTitle();
        }
    }
}