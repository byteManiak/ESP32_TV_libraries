#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <ux.h>

class Submenu
{
public:
	Submenu(VGAExtended *vga, const char *title);
	~Submenu() = default;
	virtual void attachQueues(QueueHandle_t queueTx, QueueHandle_t queueRx);
	virtual void drawSubmenu() = 0;
	void drawTitle();

protected:
	VGAExtended *vga;
	char title[64];

	QueueHandle_t queueRx = nullptr, queueTx = nullptr;
};