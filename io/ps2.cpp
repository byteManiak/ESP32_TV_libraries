#include <driver/uart.h>
#include "ps2.h"

// Holds current state of all keys as 1-bit flags
static uint32_t ps2Keys[4];
// Holds previous state of all keys as 1-bit flags
static uint32_t ps2Keys_prev[4];

static bool isKeyBeingReleased = false;

void initKeyboard()
{
	uart_config_t keyboardConfig = {};
	keyboardConfig.baud_rate = 12000;
	keyboardConfig.data_bits = UART_DATA_8_BITS;
	keyboardConfig.stop_bits = UART_STOP_BITS_1;
	keyboardConfig.parity = UART_PARITY_EVEN;

	uart_param_config(UART_NUM_2, &keyboardConfig);
	uart_set_pin(UART_NUM_2, -1, 36, -1, -1);
	uart_driver_install(UART_NUM_2, 256, 0, 0, NULL, 0);
}

void updateKeyboard()
{

	// Read from PS/2
	uint8_t rxBuffer[256];
	size_t rxLength = 0;
	uart_get_buffered_data_len(UART_NUM_2, &rxLength);
	uart_read_bytes(UART_NUM_2, rxBuffer, rxLength, 10 / portTICK_RATE_MS);

	// Save old states to determine if a key was just pressed/released
	ps2Keys_prev[0] = ps2Keys[0];
	ps2Keys_prev[1] = ps2Keys[1];
	ps2Keys_prev[2] = ps2Keys[2];
	ps2Keys_prev[3] = ps2Keys[3];

	for (int i = 0; i < rxLength; i++)
	{
		uint8_t current = rxBuffer[i];
		switch (current)
		{
			// ignore 0xE0 and any other unrecognised codes as we do not use special/alt keys
			// TODO: Consider whether it is worth supporting these keys
			case 0xE0:
				continue;

			// 0xF0 signifies a key was released
			case 0xF0:
			{
				isKeyBeingReleased = true;
				continue;
			}

			default:
			{
				if (isKeyBeingReleased)
				{
					// Set flag corresponding to the key to false
					ps2Keys[current / 32] &= ~(1 << (current % 32));
					isKeyBeingReleased = false;
				}
				else
				{
					// Set flag corresponding to the key to true
					ps2Keys[current / 32] |= 1 << (current % 32);
				}
				continue;
			}
		}
	}

	if (isKeyDown(Ctrl_key) && isKeyDown(Alt_key) && isKeyDown(Del_key))
		esp_restart();
}

#define IS_CURRENT_KEYCODE_BIT_SET(keycode) \
	(ps2Keys[keycode / 32] & (1 << (keycode % 32)))

#define IS_PREV_KEYCODE_BIT_SET(keycode) \
	(ps2Keys_prev[keycode / 32] & (1 << (keycode % 32)))

// A key is held if both the current and previous states show the flag as true
bool isKeyHeld(char keycode)
{
	return IS_CURRENT_KEYCODE_BIT_SET(keycode)
		 & IS_PREV_KEYCODE_BIT_SET(keycode);
}

// A key is pressed if the current state's flag is true and the previous state's flag is false
bool isKeyPressed(char keycode)
{
	return IS_CURRENT_KEYCODE_BIT_SET(keycode)
		& ~IS_PREV_KEYCODE_BIT_SET(keycode);
}

// A key is released if the current state's flag is false and the previous state's flag is true
bool isKeyReleased(char keycode)
{
	return ~IS_CURRENT_KEYCODE_BIT_SET(keycode)
		  & IS_PREV_KEYCODE_BIT_SET(keycode);
}

// A key is down if it was just pressed or if it is still being held
bool isKeyDown(char keycode)
{
	return isKeyPressed(keycode) || isKeyHeld(keycode);
}
