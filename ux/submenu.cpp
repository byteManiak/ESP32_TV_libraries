#include <submenu.h>

#include <string.h>

#include <memory/alloc.h>
#include <util/queues.h>

Submenu::Submenu(VGAExtended *vga, const char *title)
{
	this->vga = vga;
	strlcpy(this->title, title, 64);
}

void Submenu::attachQueues(QueueHandle_t queueTx, QueueHandle_t queueRx)
{
	this->queueTx = queueTx;
	this->queueRx = queueRx;
}

void Submenu::drawSubmenu()
{
	if (isActive) offsetX = 0;
	else offsetX = vga->xres;

	for(int i = 0; i < widgets.size(); i++)
		widgets[i]->draw(offsetX);
}

void Submenu::drawTitle()
{
	vga->drawText(this->title);
}

void Submenu::updateState()
{
	receiveQueueData();

	// Update all widgets in case an event is received from low-level queue
	for(int i = 0; i < widgets.size(); i++)
		widgets[i]->update();
}

void Submenu::setActiveSubmenu(bool isActive)
{
	this->isActive = isActive;
	if (widgets.size() > 0)
	{
		if (isActive) setFocusedWidget(focusedWidgetIndex);
		else setFocusedWidget(-1);
	}
}

void Submenu::setFocusedWidget(int8_t widgetNum)
{
	// Unfocus the current widget
	if (focusedWidget) focusedWidget->setFocused(false);

	if (widgetNum < 0 || !isActive) {focusedWidget = NULL; return;}

	// Focus the new widget
	focusedWidget = widgets[widgetNum];
	focusedWidgetIndex = widgetNum;
	focusedWidget->setFocused(true);
}