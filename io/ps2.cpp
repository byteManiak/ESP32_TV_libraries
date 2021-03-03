#include <driver/uart.h>
#include <sound.h>
#include <util.h>
#include <math.h>
#include "ps2.h"

// Holds current state of all keys as 1-bit flags
static uint32_t ps2Keys[4];
// Holds previous state of all keys as 1-bit flags
static uint32_t ps2Keys_prev[4];

static bool isKeyBeingReleased = false;
static bool isShiftPressed = false;

struct asciiMapping
{
	char scanCode, asciiCode, asciiCodeShift;
};

static asciiMapping asciiMappings[] = {
	{Space_key, 32, 32},
	{One_key, 49, 33},
	{Two_key, 50, 64},
	{Three_key, 51, 35},
	{Four_key, 52, 36},
	{Five_key, 53, 37},
	{Six_key, 54, 94},
	{Seven_key, 55, 38},
	{Eight_key, 56, 42},
	{Nine_key, 57, 40},
	{Zero_key, 48, 41},
	{Minus_key, 45, 95},
	{Equal_key, 61, 43},
	{A_key, 97, 65},
	{B_key, 98, 66},
	{C_key, 99, 67},
	{D_key, 100, 68},
	{E_key, 101, 69},
	{F_key, 102, 70},
	{G_key, 103, 71},
	{H_key, 104, 72},
	{I_key, 105, 73},
	{J_key, 106, 74},
	{K_key, 107, 75},
	{L_key, 108, 76},
	{M_key, 109, 77},
	{N_key, 110, 78},
	{O_key, 111, 79},
	{P_key, 112, 80},
	{Q_key, 113, 81},
	{R_key, 114, 82},
	{S_key, 115, 83},
	{T_key, 116, 84},
	{U_key, 117, 85},
	{V_key, 118, 86},
	{W_key, 119, 87},
	{X_key, 120, 88},
	{Y_key, 121, 89},
	{Z_key, 122, 90},
	{LBracket_key, 91, 123},
	{RBracket_key, 93, 125},
	{Backslash_key, 92, 124},
	{Semicolon_key, 59, 58},
	{Quote_key, 39, 34},
	{Comma_key, 44, 60},
	{Period_key, 46, 62},
	{Slash_key, 47, 63},
	{Tilda_key, 96, 126},
};

#define IS_CURRENT_KEYCODE_BIT_SET(keycode) \
	(ps2Keys[keycode / 32] & (1 << (keycode % 32)))

#define IS_PREV_KEYCODE_BIT_SET(keycode) \
	(ps2Keys_prev[keycode / 32] & (1 << (keycode % 32)))

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
					// Clear flag corresponding to the key
					ps2Keys[current / 32] &= ~(1 << (current % 32));
					isKeyBeingReleased = false;
				}
				else
				{
					// Set flag corresponding to the key
					ps2Keys[current / 32] |= 1 << (current % 32);
				}
				continue;
			}
		}
	}

	if (IS_CURRENT_KEYCODE_BIT_SET(LShift_key) || IS_CURRENT_KEYCODE_BIT_SET(RShift_key))
		isShiftPressed = true;
	else isShiftPressed = false;

	if (isKeyDown(Ctrl_key) && isKeyDown(Alt_key) && isKeyDown(Del_key))
	{
		esp_restart();
	}
}

bool isKeyHeld(char keycode)
{
	return IS_CURRENT_KEYCODE_BIT_SET(keycode)
		 & IS_PREV_KEYCODE_BIT_SET(keycode);
}

bool isKeyPressed(char keycode)
{
	return IS_CURRENT_KEYCODE_BIT_SET(keycode)
		& ~IS_PREV_KEYCODE_BIT_SET(keycode);
}

bool isKeyReleased(char keycode)
{
	return ~IS_CURRENT_KEYCODE_BIT_SET(keycode)
		  & IS_PREV_KEYCODE_BIT_SET(keycode);
}

bool isKeyDown(char keycode)
{
	return isKeyPressed(keycode) || isKeyHeld(keycode);
}

char getLastAsciiKey()
{
	for(int i = 0; i < sizeof(asciiMappings)/sizeof(asciiMapping); i++)
		if (isKeyPressed(asciiMappings[i].scanCode))
		{
			return isShiftPressed ? asciiMappings[i].asciiCodeShift : asciiMappings[i].asciiCode;
		}
	return 0;
}
