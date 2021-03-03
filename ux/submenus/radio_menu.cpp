#include <submenus/radio_menu.h>

#include <net/http.h>

RadioMenu::RadioMenu(VGAExtended *vga, const char *title) : Submenu(vga, title)
{
	attachQueues(httpQueueRx, radioQueueTx);

	connectionStatus = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
	new (connectionStatus) Button(vga, "Not connected", vga->xres/2, vga->yres/48);
	widgets.push_back(connectionStatus);

	radioStationList = heap_caps_malloc_cast<List<char*>>(MALLOC_CAP_PREFERRED);
	new (radioStationList) List<char*>(vga, vga->xres/4, vga->yres/8, 18);
	radioStationList->addElement("<- Prev stations <-");
	radioStationList->addElement("-> Next stations ->");
	widgets.push_back(radioStationList);

	setFocusedWidget(CONNECTION_STATUS);
}

void RadioMenu::receiveQueueData()
{
	if (queueRx)
	{
		queue_message *rxMessage;
		if (xQueueReceive(queueRx, &rxMessage, 0) == pdPASS)
		{
			switch(rxMessage->msg_flags)
			{
				case RADIO_QUEUE_RX_WIFI_CONNECTED:
				{
					connectionStatus->setText("WiFi connected");
					connectionStatus->setFillColor(8);

					state = RADIO_MENU_STATE_CONNECTED;
					break;
				}
				case RADIO_QUEUE_RX_RADIO_STATION:
				{
					char *radioStation = heap_caps_malloc_cast<char>(MALLOC_CAP_DEFAULT, 256);
					strlcpy(radioStation, rxMessage->msg_text, 256);
					radioStationList->addElementAt(radioStation, radioStationList->getSize()-1);
					break;
				}
				case RADIO_QUEUE_RX_FINISHED_OP:
				{
					if (radioStationList->getSize() < 18)
					{
						if (radioStationList->getSize() == 2)
							radioPageNumMax = radioPageNum-1;
						else radioPageNumMax = radioPageNum;
					}
					state = RADIO_MENU_STATE_DISPLAY_LIST;
					break;
				}
			}

			heap_caps_free(rxMessage);
		}
	}
}

void RadioMenu::updateSubmenu()
{
	updateState();

	switch(state)
	{
		case RADIO_MENU_STATE_WAITING_WIFI:
		{
			connectionStatus->setText("Waiting for WiFi");
			connectionStatus->setFillColor(56);
			break;
		}

		// This state will run only once after the device is connected to a WiFi network
		case RADIO_MENU_STATE_CONNECTED:
		{
			// Request the first 16 radio stations
			esp_err_t error = sendQueueData(queueTx, HTTP_QUEUE_TX_REQUEST_RADIO_LIST,
											HTTP_SERVER_ADDRESS "/radio=0,15");
			if (error == ESP_OK)
			{
				state = RADIO_MENU_STATE_DISPLAY_LIST;
			}
			break;
		}

		case RADIO_MENU_STATE_DISPLAY_LIST:
		{
			setFocusedWidget(RADIO_LIST);

			int8_t listElementIndex = radioStationList->getStatus();
			if ((listElementIndex == 0 || isKeyPressed(PgUp_key)) && radioPageNum > 0)
			{
				radioPageNum--;
				state = RADIO_MENU_STATE_REQUEST_LIST;
			}
			else if (listElementIndex == 17 || isKeyPressed(PgDown_key))
			{
				if (radioPageNum < radioPageNumMax) radioPageNum++;
				state = RADIO_MENU_STATE_REQUEST_LIST;
			}
			else if (listElementIndex > 0 && listElementIndex < 17)
			{
				char *url = heap_caps_malloc_cast<char>(MALLOC_CAP_PREFERRED, 256);
				snprintf(url, 256, HTTP_SERVER_ADDRESS "/station=%d", radioPageNum*16+listElementIndex-1);

				esp_err_t error = sendQueueData(queueTx, HTTP_QUEUE_TX_REQUEST_RADIO_STATION, url);
				if (error == ESP_OK)
				{
					char currentStation[64] = "On air: ";
					strlcpy(currentStation+8, radioStationList->getElement(), 48);
					connectionStatus->setText(currentStation);
				}
			}
			break;
		}

		case RADIO_MENU_STATE_REQUEST_LIST:
		{
			char *url = heap_caps_malloc_cast<char>(MALLOC_CAP_PREFERRED, 256);
			snprintf(url, 256, HTTP_SERVER_ADDRESS "/radio=%d,%d", radioPageNum*16, radioPageNum*16+15);

			esp_err_t error = sendQueueData(queueTx, HTTP_QUEUE_TX_REQUEST_RADIO_LIST, url);
			if (error == ESP_OK)
			{
				state = RADIO_MENU_STATE_LIST_UPDATING;

				while(radioStationList->getSize() > 2)
				{
					radioStationList->removeElementAt(1);
				}
				setFocusedWidget(RADIO_LIST);
			}
			break;
		}

		case RADIO_MENU_STATE_LIST_UPDATING:
			break;
	}
}