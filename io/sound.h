#pragma once

#include <stdint.h>

void initSound();

extern bool enableSound;
void setDuty(uint8_t channel, uint32_t duty);
void setFrequency(uint8_t channel, uint32_t frequency);
void setNote(uint8_t channel, uint32_t duty, uint32_t frequency);