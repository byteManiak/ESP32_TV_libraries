#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <vga/vga.h>

class Widget
{
public:
	Widget(VGAExtended *vga, int16_t x, int16_t y)
		: vga{vga}, baseX{x}, baseY{y} {currentX = baseX; currentY = baseY;}

	/**
	 * @brief Draw this widget on the screen
	 * @param offsetX Offset of the entire submenu when lerp'ing it around.
	 */
	virtual void draw(int16_t offsetX);

	/**
	 * @brief Update this widget's state.
	 */
	virtual void update() = 0;

	/**
	 * @brief Get this widget's status as a result of calling update()
	 */
	int8_t getStatus() {return status;}

	void setFocused(bool focus) {isFocused = focus;}
	void setVisible(bool visible) {isVisible = visible;}

protected:
	bool isFocused = false;
	bool isVisible = true;

	// Result of widget action
	int8_t status = 0;

	VGAExtended *vga;
	int16_t baseX, baseY;
	double currentX, currentY;
};