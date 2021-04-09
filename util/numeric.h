/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

#pragma once

#include <stdint.h>

// PI*2
constexpr double pi2 = 3.14159 * 2;

/**
 * @brief Get microseconds since the ESP started.
 */
int64_t getMicros();

/**
 * @brief Get milliseconds since the ESP started.
 */
int64_t getMillis();

/**
 * @brief Get the next integer between [0,max), wrapping around to 0 if >= max.
 * @param s Source integer from which to get the next integer in the sequence [0,max)
 * @param max Maximum value the next integer can be.
 */
uint32_t getNextInt(uint32_t s, uint32_t max);

/**
 * @brief Get the previous integer between [0,max), wrapping around if < 0.
 * @param s Source integer from which to get the prev integer in the sequence [0,max)
 * @param max Maximum value the prev integer can be.
 */
uint32_t getPrevInt(uint32_t s, uint32_t max);

// Result of calculateTimeDelta().
extern double timeDelta;
/**
 * @brief Calculate the time delta between this call and the previous call to calculateTimeDelta()
 * NOTE: Do not call more than once per frame, and make sure to always call this from the same one task.
 * The result is stored in the global variable "timeDelta".
 */
void calculateTimeDelta();

/**
 * @brief Interpolate from a double to the other, storing the result in the first parameter.
 * @param from Double to interpolate from.
 * @param to Destination double to interpolate to.
 */
void smoothLerp(double &from, double to);