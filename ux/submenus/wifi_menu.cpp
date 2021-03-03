#include <submenus/wifi_menu.h>

#include <string.h>

#include <alloc.h>
#include <ps2.h>
#include <util.h>
#include <wifi/common.h>

WifiMenu::WifiMenu(VGAExtended *vga, const char *title) : Submenu(vga, title)
{
    attachQueues(wifiQueueRx, wifiQueueTx);

    actionButton = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
    new (actionButton) Button(vga, "Begin WiFi scan", vga->xres/2 + vga->xres/16, vga->yres/6);
    actionButton->setFillColor(56);
    widgets.push_back(actionButton);

    ssidList = heap_caps_malloc_cast<List<char*>>(MALLOC_CAP_PREFERRED);
    new (ssidList) List<char*>(vga, vga->xres/2 + vga->xres/16, vga->yres/4, 8, true);
    widgets.push_back(ssidList);

    ipDetails = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
    new (ipDetails) Button(vga, "", vga->xres/2 + vga->xres/16, vga->yres/2 + vga->yres/12);
    ipDetails->setVisible(false);
    widgets.push_back(ipDetails);

    gatewayDetails = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
    new (gatewayDetails) Button(vga, "", vga->xres/2 + vga->xres/16, vga->yres/2 + vga->yres/6);
    gatewayDetails->setVisible(false);
    widgets.push_back(gatewayDetails);

    setFocusedWidget(ACTION_BUTTON);
}

void WifiMenu::receiveQueueData()
{
    if (queueRx)
    {
        queue_message *rxMessage;
        if (xQueueReceive(queueRx, &rxMessage, 0) == pdTRUE)
        {
            switch (rxMessage->msg_flags)
            {
                case WIFI_QUEUE_RX_SCAN_RESULT:
                {
                    ESP_LOGI("menu", "Received ssid %s through queue %p", rxMessage->msg_text, queueRx);
                    if (!ssidList->isFull())
                    {
                        char *ssid = heap_caps_malloc_cast<char>(MALLOC_CAP_PREFERRED, 33);
                        strlcpy(ssid, rxMessage->msg_text, 33);
                        ssidList->addElement(ssid);
                    }
                    state = WIFI_MENU_STATE_CHOOSE_SSID;
                    break;
                }

                case WIFI_QUEUE_RX_CONNECTED:
                {
                    ipDetails->setVisible(true);
                    gatewayDetails->setVisible(true);

                    actionButton->setText("Connected!");
                    actionButton->setFillColor(8);

                    state = WIFI_MENU_STATE_CONNECTED;
                    break;
                }

                case WIFI_QUEUE_RX_DISCONNECTED:
                {
                    actionButton->setText("Connection lost. Rescan?");
                    actionButton->setFillColor(22);

                    state = WIFI_MENU_STATE_DISCONNECTED;
                    break;
                }

                case WIFI_QUEUE_RX_IP_ADDRESS:
                {
                    char ipText[65];
                    strlcpy(ipAddress, rxMessage->msg_text, 16);
                    sprintf(ipText, "IP Address: %s", ipAddress);
                    ipDetails->setText(ipText);
                    break;
                }

                case WIFI_QUEUE_RX_GATEWAY_ADDRESS:
                {
                    char gatewayText[65];
                    strlcpy(gatewayAddress, rxMessage->msg_text, 16);
                    sprintf(gatewayText, "Gateway Address: %s", gatewayAddress);
                    gatewayDetails->setText(gatewayText);
                    break;
                }
            }
            heap_caps_free(rxMessage);
        }
    }
}

void WifiMenu::updateSubmenu()
{
    updateState();

    // Logic for the wifi menu
    switch (state)
    {
        case WIFI_MENU_STATE_DEFAULT:
        {
            bool isButtonPushed = actionButton->getStatus();
            if (isButtonPushed)
            {
                esp_err_t error = sendQueueData(queueTx, WIFI_QUEUE_TX_USER_BEGIN_SCAN, NULL);
                if (error == ESP_OK)
                {
                    ssidList->clear();
                    ESP_LOGI("menu", "Sent scan event to queue %p", queueTx);
                    // No longer need to scan an SSID
                    state = WIFI_MENU_STATE_WAITING;
                    actionButton->setFillColor(56);
                    actionButton->setText("Scanning WiFi...");
                    setFocusedWidget(SSID_LIST);
                }
            }
            break;
        }

        case WIFI_MENU_STATE_CHOOSE_SSID:
        {
            actionButton->setFillColor(50);
            actionButton->setText("Scan results:");
            // Logic for choosing a SSID for which to enter a password and then connect to
            if (!ssidList->isEmpty())
            {
                int8_t listElementIndex = ssidList->getStatus();
                if (listElementIndex != -1)
                {
                    esp_err_t e = sendQueueData(queueTx, WIFI_QUEUE_TX_USER_SSID, ssidList->getElement());
                    if (e == ESP_OK)
                    {
                        // Force the menu into entering the state where a password can be input
                        state = WIFI_MENU_STATE_QUERY_PASSWORD;

                        passwordTextbox = (Textbox*)heap_caps_malloc(sizeof(Textbox), MALLOC_CAP_PREFERRED);
                        new (passwordTextbox) Textbox(vga, true);
                        widgets.push_back(passwordTextbox);

                        setFocusedWidget(PASSWORD_TEXTBOX);
                    }
                }
            }
            break;
        }

        case WIFI_MENU_STATE_QUERY_PASSWORD:
        {
            int8_t textboxState = passwordTextbox->getStatus();
            if (textboxState == TEXTBOX_ENTER)
            {
                esp_err_t error = sendQueueData(queueTx, WIFI_QUEUE_TX_USER_PSK, passwordTextbox->getText());
                if (error == ESP_OK)
                {
                    actionButton->setText("Connecting...");
                    ssidList->clear();

                    state = WIFI_MENU_STATE_WAITING;
                    setFocusedWidget(ACTION_BUTTON);

                    widgets.pop_back();
                    heap_caps_free(passwordTextbox);
                }
            }
            else if (textboxState == TEXTBOX_CANCEL)
            {
                state = WIFI_MENU_STATE_CHOOSE_SSID;
                setFocusedWidget(SSID_LIST);

                widgets.pop_back();
                heap_caps_free(passwordTextbox);
            }
            break;
        }
        
        case WIFI_MENU_STATE_WAITING:
            break;

        case WIFI_MENU_STATE_CONNECTED:
        {
            break;
        }

        case WIFI_MENU_STATE_DISCONNECTED:
        {
            bool isButtonPushed = actionButton->getStatus();
            if (isButtonPushed)
            {
                BaseType_t e = sendQueueData(queueTx, WIFI_QUEUE_TX_USER_BEGIN_SCAN, NULL);
                if (e == pdTRUE)
                {
                    ssidList->clear();
                    ESP_LOGI("menu", "Sent scan event to queue %p", queueTx);
                    // No longer need to scan an SSID
                    state = WIFI_MENU_STATE_WAITING;
                    actionButton->setText("Scanning WiFi...");
                    setFocusedWidget(SSID_LIST);
                }
            }

            break;
        }
    }
}
