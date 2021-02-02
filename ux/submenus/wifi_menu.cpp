#include <submenus/wifi_menu.h>

#include <string.h>

#include <alloc.h>
#include <ps2.h>
#include <util.h>
#include <wifi.h>

WifiMenu::WifiMenu(VGAExtended *vga, const char *title) : Submenu(vga, title)
{
    beginScanButton = (Button*)heap_caps_malloc(sizeof(Button), MALLOC_CAP_PREFERRED);
    new (beginScanButton) Button(vga, "Begin WiFi scan", vga->xres/2 + vga->xres/6, vga->yres/6);
    widgets.push_back(beginScanButton);

    ssidList = (List<const char*>*)heap_caps_malloc(sizeof(List<const char*>), MALLOC_CAP_PREFERRED);
    new (ssidList) List<const char*>(vga, 2*vga->xres/3, vga->yres/4, 8);
    widgets.push_back(ssidList);

    setFocusedWidget(0);
}

void WifiMenu::drawSubmenu()
{
    // TODO: Ideally, any block of code that sends out an event
    // should be contained within its own UI element, e.g. a button, list, dialog etc

    // So far, the only wifi event that is received is the list of scanned SSIDs
    if (queueRx)
    {
        wifi_queue_message *rxMessage;
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
            }
            heap_caps_free(rxMessage);
        }
    }

    int8_t focusedWidgetResult = 0;
    for(int i = 0; i < widgets.size(); i++)
    {
        int8_t result = widgets[i]->update();
        if (widgets[i] == focusedWidget)
            focusedWidgetResult = result;
    }

    // Logic for the wifi menu
    switch (state)
    {
        case WIFI_MENU_STATE_DEFAULT:
        {
            if (focusedWidgetResult == 1)
            {
                state = WIFI_MENU_STATE_SCAN_SSID;
                beginScanButton->setText("Scanning WiFi...");
                setFocusedWidget(1);
            }
            break;
        }
        case WIFI_MENU_STATE_SCAN_SSID:
        {
            wifi_queue_message *txMessage = heap_caps_malloc_cast<wifi_queue_message>(MALLOC_CAP_PREFERRED);
            txMessage->msg_flags = WIFI_QUEUE_TX_USER_BEGIN_SCAN;

            ssidList->clear();

            if (queueTx && xQueueSend(queueTx, &txMessage, 10 / portTICK_PERIOD_MS) == pdTRUE)
            {
                ESP_LOGI("menu", "Sent scan event to queue %p", queueTx);
                // No longer need to scan an SSID
                state = WIFI_MENU_STATE_WAITING_SCAN;
            }
            break;
        }
        case WIFI_MENU_STATE_WAITING_SCAN:
            break;
        case WIFI_MENU_STATE_CHOOSE_SSID:
        {
            beginScanButton->setText("Scan results:");
            // Logic for choosing a SSID for which to enter a password and then connect to
            if (!ssidList->isEmpty())
            {
                if (focusedWidgetResult != -1)
                {
                    wifi_queue_message *txMessage = heap_caps_malloc_cast<wifi_queue_message>(MALLOC_CAP_PREFERRED);
                    txMessage->msg_flags = WIFI_QUEUE_TX_USER_SSID;
                    strlcpy(txMessage->msg_text, ssidList->getElement(), 33);

                    if (xQueueSend(queueTx, &txMessage, 10 / portTICK_PERIOD_MS) == pdPASS)
                    {
                        ESP_LOGI("menu", "Sent ssid %s through queue %p", txMessage->msg_text, queueTx);
                        // Force the menu into entering the state where a password can be input
                        state = WIFI_MENU_STATE_QUERY_PASSWORD;

                        passwordTextbox = (Textbox*)heap_caps_malloc(sizeof(Textbox), MALLOC_CAP_PREFERRED);
                        new (passwordTextbox) Textbox(vga, true);
                        widgets.push_back(passwordTextbox);

                        setFocusedWidget(2);
                    }
                }
            }
            break;
        }
        case WIFI_MENU_STATE_QUERY_PASSWORD:
        {
            if (focusedWidgetResult == 1)
            {
                wifi_queue_message *txMessage = heap_caps_malloc_cast<wifi_queue_message>(MALLOC_CAP_PREFERRED);

                txMessage->msg_flags = WIFI_QUEUE_TX_USER_PSK;
                state = WIFI_MENU_STATE_DEFAULT;

                strlcpy(txMessage->msg_text, passwordTextbox->getText(), 65);

                if (xQueueSend(queueTx, &txMessage, 10 / portTICK_PERIOD_MS) == pdTRUE)
                {
                    ESP_LOGI("menu", "Sent psk %s through queue %p", txMessage->msg_text, queueTx);
                    state = WIFI_MENU_STATE_DEFAULT;
                    setFocusedWidget(0);

                    widgets.pop_back();
                    heap_caps_free(passwordTextbox);
                }
            }
            break;
        }
    }

    for(int i = 0; i < widgets.size(); i++)
        widgets[i]->draw(offsetX);
}