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
	virtual void updateSubmenu() = 0;
	void drawSubmenu()
	{
		if (isActive) offsetX = 0;
		else offsetX = vga->xres;

	for(int i = 0; i < widgets.size(); i++)
		widgets[i]->draw(offsetX);
	}

	void drawTitle();

	void setActiveSubmenu(bool isActive)
	{
		this->isActive = isActive;
		if (isActive) setFocusedWidget(focusedWidgetIndex);
		else setFocusedWidget(-1);
	}

	bool isActiveMenu = false;

protected:
	void setFocusedWidget(int8_t widgetNum)
	{
		// Unfocus the current widget
		if (focusedWidget) focusedWidget->setFocused(false);

		if (widgetNum < 0) {focusedWidget = nullptr; return;}

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
	bool isActive = false;

	char title[64];

	QueueHandle_t queueRx = nullptr, queueTx = nullptr;
};
