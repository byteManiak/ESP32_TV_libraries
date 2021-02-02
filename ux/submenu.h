#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <alloc.h>
#include <vga.h>
#include <widget.h>

class Submenu
{
public:
	Submenu(VGAExtended *vga, const char *title);
	~Submenu() = default;
	virtual void attachQueues(QueueHandle_t queueTx, QueueHandle_t queueRx);
	virtual void drawSubmenu() = 0;
	void drawTitle();

	bool isActiveMenu = false;

protected:
	void setFocusedWidget(uint8_t widgetNum)
	{
		// Unfocus the current widget
		if (focusedWidget) focusedWidget->setFocused(false);

		// Focus the new widget
		focusedWidget = widgets[widgetNum];
		focusedWidgetIndex = widgetNum;
		focusedWidget->setFocused(true);
	}
	uint8_t focusedWidgetIndex = 0;
	Widget *focusedWidget = nullptr;
	heap_caps_vector<Widget*> widgets;

	VGAExtended *vga;
	int16_t offsetX = 0;

	char title[64];

	QueueHandle_t queueRx = nullptr, queueTx = nullptr;
};