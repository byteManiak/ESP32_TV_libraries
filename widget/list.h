#pragma once

#include <string.h>

#include <memory/alloc.h>
#include <io/ps2.h>
#include <widget/widget.h>
#include <util/numeric.h>
#include <util/queues.h>

template <typename T>
class List : public Widget
{
public:
	List(VGAExtended *vga, int16_t x, int16_t y, int8_t maxListSize, bool centeredText = false, bool scrollText = false)
		: Widget(vga, x, y), maxListSize{maxListSize}, centeredText{centeredText}, scrollText{scrollText} {}

	/**
	 * @brief Add element at the end of the list.
	 * @param elem Element to add in the list.
	 */
	void addElement(T elem)
	{
		listElements.push_back(elem);
	}

	/**
	 * @brief Add element at the specified index in the list.
	 * @param elem Element to add in the list.
	 * @param position Index of the list to add the element at.
	 */
	void addElementAt(T elem, uint8_t position)
	{
		listElements.insert(listElements.begin()+position, elem);
	}

	/**
	 * @brief Remove element at the specified index in the list.
	 * @param position Index of the element to remove.
	 */
	void removeElementAt(uint8_t position)
	{
		T currentElem = listElements[position];
		heap_caps_free(currentElem);

		listElements.erase(listElements.begin()+position);
	}

	/**
	 * @brief Get the number of elements currently in the list.
	 */
	size_t getSize()
	{
		return listElements.size();
	}

	/**
	 * @brief Check if there are no elements in the list.
	 */
	bool isEmpty()
	{
		return !(listElements.size() > 0);
	}

	/**
	 * @brief Check if the list has reached max capacity.
	 */
	bool isFull()
	{
		return listElements.size() == maxListSize;
	}

	/**
	 * @brief Clear the elements in the list.
	 */
	void clear()
	{
		size_t listSize = listElements.size();
		for(int i = 0; i < listSize; i++)
		{
			T currentElem = listElements[i];
			heap_caps_free(currentElem);
		}
		listElements.clear();
		currentElement = 0;

		hoverTime = getMillis();
	}

	/**
	 * @brief Get the currently hovered element from the list.
	 */
	T &getElement()
	{
		return listElements[currentElement];
	}

	/**
	 * @brief Update this widget's state.
	 */
	void update()
	{
		status = -1;

		if (isFocused)
		{
			if (isKeyPressed(Up_key))
			{
				currentElement = getPrevInt(currentElement, listElements.size());
				hoverTime = getMillis();
			}
			if (isKeyPressed(Down_key))
			{
				currentElement = getNextInt(currentElement, listElements.size());
				hoverTime = getMillis();
			}

			if (isKeyPressed(Enter_key)) status = currentElement;
			else status = -1;
		}
	}

	/**
	 * @brief Draw this widget on the screen
	 * @param offsetX Offset of the entire submenu when lerp'ing it around.
	 */
	void draw(int16_t offsetX)
	{
		if (isVisible)
		{
			double to = baseX + offsetX;
			smoothLerp(currentX, to);
			for(int i = 0; i < listElements.size(); i++)
			{
				if (i == currentElement)
				{
					vga->setTextColor(vga->RGB(0x00AAFF), vga->backColor);

					// Start scrolling text after 2 seconds of hovering over it
					int64_t scrollTime = getMillis() - hoverTime;
					if (isFocused && scrollText && scrollTime > 2000)
						offsetIndex = ((scrollTime - 2000) / 500) % (strlen(listElements[i]));
				}
				else
				{
					vga->setTextColor(vga->RGB(0xFFFFFF), vga->backColor);
					offsetIndex = 0;
				}

				int xcoord = currentX;
				if (centeredText) xcoord -= vga->font->charWidth * strlen(listElements[i]) / 2.f;

				vga->setCursor(xcoord, baseY + i * (vga->font->charHeight+1));
				vga->drawText(listElements[i]+offsetIndex);

				int ylinecoord = baseY + (i+1) * (vga->font->charHeight+1) - 1;
				vga->drawLine(xcoord, ylinecoord, xcoord+vga->xres, ylinecoord, vga->RGB(0x00FFAA));
			}
		}
	}

private:
	int8_t maxListSize;
	int8_t currentElement = 0;

	heap_caps_vector<T> listElements;

	bool centeredText;
	bool scrollText;

	// Optionally used to scroll through text that does not fit on text
	int64_t hoverTime;
	uint8_t offsetIndex = 0;
};
