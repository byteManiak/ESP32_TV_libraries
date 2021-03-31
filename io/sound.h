#pragma once

#include <stdint.h>

void initSound();
void audioDispatchTask(void *arg);

enum Note
{
	NONE, C1, Cs1, D1, Ds1, E1, F1, Fs1, G1, Gs1, A1, As1,
	B1, C2, Cs2, D2, Ds2, E2, F2, Fs2, G2, Gs2, A2, As2,
	B2, C3, Cs3, D3, Ds3, E3, F3, Fs3, G3, Gs3, A3, As3,
	B3, C4, Cs4, D4, Ds4, E4, F4, Fs4, G4, Gs4, A4, As4,
	B4, C5, Cs5, D5, Ds5, E5, F5, Fs5, G5, Gs5, A5, As5,
	B5, C6, Cs6, D6, Ds6, E6, F6, Fs6, G6, Gs6, A6, As6,
	B6, C7, Cs7, D7, Ds7, E7, F7, Fs7, G7, Gs7, A7, As7,
	B7, C8, Cs8, D8, Ds8
};

struct BuzzerInstrument
{
	Note note;
	uint16_t duty;
};

struct BuzzerMusic
{
	uint64_t tick_period;
	uint8_t ticks_per_note;
	uint32_t data_length;
	BuzzerInstrument *left_ch_data;
	BuzzerInstrument *right_ch_data;
};

/**
 * @brief Initialises the onboard 5V buzzers.
 */
void initBuzzers();

/**
 * @brief Starts playing a music loop from the specified BuzzerMusic structure.
 * @param music Pointer to the music data to play.
 */
void playBuzzerMusic(BuzzerMusic *music);