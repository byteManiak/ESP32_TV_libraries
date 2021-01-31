#pragma once

#include <string.h>
#include <VGA/VGA6Bit.h>
#include <util.h>

#define LIFETIME 4000
#define NUMLINES 26
static EXT_RAM_ATTR char lines[NUMLINES][64];
static EXT_RAM_ATTR int lifetimes[NUMLINES];

class TextView
{
public:
	TextView() {}
	~TextView() {}

	void setVGAController(VGA6Bit *vga)
	{
		// Set VGA display to print to
		v = vga;
	}

	void print(const char *text)
	{
		// Scroll text upwards
		for(int i = 0; i < NUMLINES-1; i++)
		{
			lifetimes[i] = lifetimes[i+1];
			strncpy(lines[i], lines[i+1], 64);
		}

		// Add the new string to print
		lifetimes[NUMLINES-1] = getMillis();
		strncpy(lines[NUMLINES-1], text, 64);
	}

	void update()
	{
		// Print all lines in the list of strings
		// Do not display strings that have been
		// on screen for too long
		for(int i = 0; i < NUMLINES; i++)
		{
			if (lifetimes[i] + LIFETIME > getMillis())
			{
				v->setCursor(4, i*8);
				v->print(lines[i]);
			}
		}
	}

private:
	VGA6Bit *v;
};
