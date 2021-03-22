#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <memory/alloc.h>
#include <vga/vga.h>
#include <widget/widget.h>

class Submenu
{
public:
	Submenu(VGAExtended *vga, const char *title);
	~Submenu() = default;

	/**
	 * @brief Attach low-level event queues to send/receive data from.
	 * NOTE: In this case Tx/Rx denote the respective Rx/Tx low-level queues.
	 * For example: attachQueues(wifiQueueRx, wifiQueueTx) will enable the user
	 * to SEND data to the wifi module's rx queue, and to RECEIVE data from its tx queue.
	 * @param queueTx Low-level queue to send data to.
	 * @param queueRx Low-level queue to read data from.
	 */
	void attachQueues(QueueHandle_t queueTx, QueueHandle_t queueRx);

	/**
	 * @brief Update the submenu's state and UI elements.
	 */
	virtual void updateSubmenu() = 0;

	/**
	 * @brief Draw this submenu.
	 */
	virtual void drawSubmenu();

	/**
	 * @brief Draw the title of this submenu on the main menu list.
	 */
	void drawTitle();

	/**
	 * @brief Flag this submenu as the active one.
	 */
	void setActiveSubmenu(bool isActive);
	bool isActiveMenu = false;

	/**
	 * @brief Set the n-th widget in the widgets vector as the active one
	 * 
	 * @param widgetNum The index of the widget to set as active
	 */
	void setFocusedWidget(int8_t widgetNum);

	/**
	 * @brief Refresh state of the menu by reading from rx/tx queues and
	 * checking user input / changes from widgets.
	 * NOTE: Call before menu state handling in updateSubmenu().
	 */
	void updateState();

protected:

	/**
	 * @brief Wait for external data from low-level handlers to advance the menu's state.
	 * The queue handles must be valid when calling this function, otherwise the menu state
	 * most likely won't be updated.
	 */
	virtual void receiveQueueData() = 0;

	// Handle for the currently focused widget
	uint8_t focusedWidgetIndex = 0;
	// Handle for the currently focused widget
	Widget *focusedWidget = NULL;
	// Vector storing all the submenu's widgets
	heap_caps_vector<Widget*> widgets;

	VGAExtended *vga;
	// Offset of the submenu to enable smooth animation transition when [de]selecting it
	int16_t offsetX = 0;
	// Establish whether this menu is the currently opened one.
	bool isActive = false;
	// Title of the submenu as shown on the radial main menu.
	char title[64];

	// Handles of queues to send and receive data from, if needed.
	QueueHandle_t queueRx = NULL, queueTx = NULL;
};
