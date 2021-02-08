#pragma once

#include <alloc.h>
#include <ps2.h>
#include <widget.h>

template <typename T>
class List : public Widget
{
public:
    List(VGAExtended *vga, int16_t x, int16_t y, int8_t maxListSize)
        : Widget(vga, x, y), maxListSize{maxListSize} {}

    void addElement(T elem)
    {
        listElements.push_back(elem);
    }

    bool isEmpty()
    {
        return !(listElements.size() > 0);
    }

    bool isFull()
    {
        return listElements.size() == maxListSize;
    }

    void clear()
    {
        size_t listSize = listElements.size();
        for(int i = 0; i < listSize; i++)
            listElements.pop_back();
    }

    T &getElement()
    {
        return listElements[currentElement];
    }

    int8_t update()
    {
        if (isFocused)
        {
            if (isKeyPressed(Up_key))
            {
                currentElement--;
                if (currentElement < 0) currentElement = 0;
            }
            if (isKeyPressed(Down_key))
            {
                currentElement++;
                if (currentElement > maxListSize-1) currentElement = maxListSize-1;
            }

            if (isKeyPressed(Enter_key)) return currentElement;
        }
        return -1;
    }

    void draw(int16_t offsetX)
    {
        if (isVisible)
        {
            for(int i = 0; i < listElements.size(); i++)
            {
                if (i == currentElement) vga->setTextColor(vga->RGB(0x00AAFF), vga->backColor);
                else vga->setTextColor(vga->RGB(0xFFFFFF), vga->backColor);
                vga->setCursor(baseX + offsetX, baseY + i * vga->font->charHeight);
                vga->drawText(listElements[i]);
            }
        }
    }

private:
    int8_t maxListSize;
    int8_t currentElement = 0;

    heap_caps_vector<T> listElements;
};