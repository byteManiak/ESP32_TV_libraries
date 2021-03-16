#pragma once

#include <VGA/VGA6Bit.h>

#include <memory/alloc.h>

enum Colors {
    BLACK,          DARK_BROWN,      DARK_RED,    RED,
    DARK_GREEN,     BROWN,           DARK_RED2,   RED2,
    GREEN,          GREEN2,          MUSTARD1,    ORANGE,
    LIGHT_GREEN,    LIGHT_GREEN2,    ACID,        YELLOW,
    INDIGO,         VIOLET,          WINE,        BUBBLEGUM,
    INDIGO2,        VIOLET2,         WINE2,       BUBBLEGUM2,
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

    long getPercentGradient(double percent);
    void fillRectAlpha(int x, int y, int w, int h, unsigned char color);
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
     * @brief Alternative to rect()/fillRect(). NOTE: This function's behaviour is different when drawing without a backbuffer.
     */
    void drawRect(int x, int y, int w, int h, unsigned char color, bool fillRect = false);
    /**
     * @brief Alternative to print(). NOTE: This function's behaviour is different when drawing without a backbuffer.
     */
    void drawFloat(float f);
    /**
     * @brief Alternative to show(). NOTE: This function's behaviour is different when drawing without a backbuffer.
     */
    void showDrawables();

private:
    heap_caps_vector<Drawable*> prevFrameDrawables;
    heap_caps_vector<Drawable*> nextFrameDrawables;
};