/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

#pragma once

#include <VGA/VGA6Bit.h>
#include <Graphics/Sprites.h>

#include <memory/alloc.h>

enum VGAColor {
	BLACK,          DARK_BROWN,      DARK_RED,    RED,
	DARK_GREEN,     BROWN,           DARK_RED2,   RED2,
	GREEN,          GREEN2,          MUSTARD1,    ORANGE,
	LIGHT_GREEN,    LIGHT_GREEN2,    ACID,        YELLOW,
	INDIGO,         VIOLET,          WINE,        BUBBLEGUM,
	INDIGO2,        GRAY,            WINE2,       BUBBLEGUM2,
	GREEN3,         GREEN4,          MUSTARD2,    LIGHT_ORANGE,
	LIGHT_GREEN3,   LIGHT_GREEN4,    ACID2,       YELLOW2,
	DARK_BLUE,      DARK_BLUE2,      PURPLE,      MAGENTA,
	BLUE,           BLUE2,           PURPLE2,     MAGENTA2,
	MIDNIGHT_GREEN, MIDNIGHT_GREEN2, CHALK_BLUE,  PINK,
	CYAN,           CYAN2,           LIGHT_CYAN,  CREAM,
	DARK_BLUE3,     DARK_BLUE4,      PURPLE3,     MAGENTA3,
	BLUE3,          BLUE4,           PURPLE4,     MAGENTA4,
	CORNFLOWER,     CORNFLOWER2,     CHALK_BLUE2, PINK2,
	CYAN3,          CYAN4,           LIGHT_CYAN2, WHITE
};

enum DrawableType {
	DRAWABLE_LINE,
	DRAWABLE_TEXT,
	DRAWABLE_RECT
};

struct Drawable
{
public:
	DrawableType type;
};

struct DrawableLine : public Drawable
{
public:
	int x1, y1, x2, y2;
	unsigned char color;
};

struct DrawableText : public Drawable
{
public:
	char text[65];
	int x, y;
	unsigned char color;
};

struct DrawableRect : public Drawable
{
public:
	int x, y, w, h;
	unsigned char color;
	bool fillRect;
};

class VGAExtended : public VGA6Bit
{
public:
	VGAExtended() : VGA6Bit() {}

	/**
	 * @brief Convert percentage to color.
	 * @param percent Percentage as a value from 0.0 to 1.0.
	 */
	unsigned char getPercentGradient(double percent);

	/**
	 * @brief Alternative to fillRect() which also supports alpha blending.
	 */
	void fillRectAlpha(int x, int y, int w, int h, unsigned char color);

	/**
	 * @brief Mixes fillRect() with print().
	 */
	void printBox(const char *text, int x, int y, unsigned char textColor = 63,
				  unsigned char borderColor = 63, unsigned char fillColor = 255, unsigned char spacing = 2);

	/**
	 * @brief Alternative to line(). NOTE: This function's behaviour is different when drawing without a backbuffer.
	 */
	void drawLine(int x1, int x2, int y1, int y2, unsigned char color);
	/**
	 * @brief Alternative to print(). NOTE: This function's behaviour is different when drawing without a backbuffer.
	 */
	void drawText(const char *text);
	/**
	 * @brief Draw text at the specified coordinates
	 */
	void drawText(const char *text, int x, int y);
	/**
	 * @brief Alternative to rect()/fillRect(). NOTE: This function's behaviour is different when drawing without a backbuffer.
	 */
	void drawRect(int x, int y, int w, int h, unsigned char color, bool fillRect = false);
	/**
	 * @brief Alternative to print(). NOTE: This function's behaviour is different when drawing without a backbuffer.
	 */
	void drawFloat(float f);

	/**
	 * @brief Draw sprite with scaling. NOTE: This function's results are undefined when drawing without a backbuffer.
	 * @param spr Sprite map to choose a single sprite from.
	 * @param index Index of the sprite to choose from the sprite map.
	 * @param scaleFactor Factor to scale the sprite by.
	 * E.g. 1.0 does not scale the sprite, 2.0 scales the sprite twice its size.
	 */
	void drawSprite(Sprites spr, int index, int x, int y, float scaleFactor);

	/**
	 * @brief Alternative to drawRect() which uses a color index rather than an actual color
	 */
	void drawRectIndexed(int x, int y, int w, int h, unsigned char color, bool fillRect = false);
	/**
	 * @brief Draw a texture from a buffer containing per-pixel indexed color data.
	 * @param imageData Buffer containing per-pixel indexed color data.
	 * @param x X coordinate at which to draw the texture.
	 * @param y Y coordinate at which to draw the texture.
	 * @param w Width of the texture to draw.
	 * @param h Height of the texture to draw.
	 * @param sx Source X coordinate of the buffer data.
	 * @param sy Source Y coordinate of the buffer data.
	 * @param scaleFactor Integer scaling with which to draw the texture.
	 */
	void drawTextureIndexed(unsigned char **imageData, int x, int y, int w, int h, int sx, int sy, int scaleFactor = 1);

	/**
	 * @brief Set the colors to be used when using indexed functions (drawRectIndexed()/drawTextureIndexed())
	 * @param colors Array of colors that will represent each index.
	 * @param colorCount Number of colors to use in indexed mode.
	 */
	void setIndexedColors(VGAColor *colors, int colorCount);

	/** @brief Set the index to be used as a key when using indexed functions. This key will act as transparency.
	 * @param index Index which will be used as a transparency key.
	 */
	void setIndexedKey(unsigned char index);

	/**
	 * @brief Clear the screen contents using an indexed color.
	 * @param index Index of the color to clear the screen contents with.
	 */
	void clearIndexed(unsigned char index);

	/**
	 * @brief Alternative to show(). NOTE: This function's behaviour is different when drawing without a backbuffer.
	 */
	void showDrawables();

private:
	VGAColor indexedColors[64] = {};
	unsigned char indexedAlphaKey;

	heap_caps_vector<Drawable*> prevFrameDrawables;
	heap_caps_vector<Drawable*> nextFrameDrawables;
};
