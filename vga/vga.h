#pragma once

#include <VGA/VGA6Bit.h>

#include <memory/alloc.h>

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