#ifndef SMART_RENDER
#define SMART_RENDER

#include <stdio.h>
#include "pico/stdlib.h"
#include "TFT.h"

///////////////////////////////// Screen arrays
#define RECT_SOLVER_WIDTH 160
#define RECT_SOLVER_HEIGHT 128

uint16_t currentScreen[RECT_SOLVER_HEIGHT][RECT_SOLVER_WIDTH];
uint16_t smartScreen[RECT_SOLVER_HEIGHT][RECT_SOLVER_WIDTH];

uint8_t screenChange[RECT_SOLVER_HEIGHT][RECT_SOLVER_WIDTH];

////////////////////////////// Render modes
#define FAST_BUT_FLICKER 0
#define CHANGE_DRAW 1
#define SLOW_BUT_SMOOTH 2

uint8_t renderMode = CHANGE_DRAW;

//////////////////////////// Dynamic variables

typedef struct ColorList
{
    struct ColorList *next;
    struct ColorList *previous;
    uint16_t color;
} ColorList;

#define MAX_COLORS 100
uint16_t pallete[MAX_COLORS];

uint16_t palleteColorCount = 0;

/////////////////////////////// Code

// Pallete helper functions
#pragma region

uint16_t IndexOfColor(uint16_t color)
{
    for (int i = 0; i < palleteColorCount; i++)
    {
        if (pallete[i] == color)
        {
            return i;
        }
    }
    return 0xFFFF;
}
void AddColor(uint16_t color)
{
    if (IndexOfColor(color) != 0xFFFF)
        return;
    pallete[palleteColorCount++] = color;
}

void ClearPallete()
{
    memset(pallete, 0, sizeof(pallete));
    palleteColorCount = 0;
}

#pragma endregion

// Converts a shape into a series of rectangles. Efficient, but CPU heavy
void DrawShapeLayerSolved(uint8_t layer, uint16_t color)
{
    bool renderedPixels[RECT_SOLVER_HEIGHT][RECT_SOLVER_WIDTH];

    for (int x = 0; x < RECT_SOLVER_WIDTH; x++)
        for (int y = 0; y < RECT_SOLVER_HEIGHT; y++)
            renderedPixels[y][x] = false;

    for (int y = 0; y < RECT_SOLVER_HEIGHT; y++)
    {
        for (int x = 0; x < RECT_SOLVER_WIDTH; x++)
        {
            if (renderedPixels[y][x])
            {
                continue;
            }

            if (screenChange[y][x] != layer)
            {
                continue;
            }

            uint8_t width = 1;
            while (screenChange[y][x + width] == layer && !renderedPixels[y][x + width] && (width + x < RECT_SOLVER_WIDTH))
            {
                width++;
            }

            bool doExtend = true;
            uint8_t height = 1;

            while (doExtend)
            {
                doExtend = true;
                for (int x2 = 0; x2 < width; x2++)
                {
                    if (screenChange[y + height][x + x2] != layer)
                    {
                        doExtend = false;
                        break;
                    }
                }
                if (doExtend)
                {
                    height++;
                }
            }

            for (int y2 = 0; y2 < height; y2++)
            {
                for (int x2 = 0; x2 < width; x2++)
                {
                    renderedPixels[y + y2][x + x2] = true;
                }
            }

            // printf("Rectangle: (%d,%d), width: %d, height %d\n", x, y, width, height);

            Rectangle(color, x, y, width, height);
        }
    }
}

// Handles the display of a frame
void DrawScreenSmart(uint16_t frame[RECT_SOLVER_HEIGHT][RECT_SOLVER_WIDTH])
{
    memset(screenChange, 0, sizeof(screenChange));

    ClearPallete();

    switch (renderMode)
    {
    case FAST_BUT_FLICKER:
        for (int y = 0; y < RECT_SOLVER_HEIGHT; y++)
        {
            for (int x = 0; x < RECT_SOLVER_WIDTH; x++)
            {
                SetPixel(x, y, frame[y][x]);
                currentScreen[y][x] = frame[y][x];
            }
        }
        break;

    case CHANGE_DRAW:
        for (int y = 0; y < RECT_SOLVER_HEIGHT; y++)
        {
            for (int x = 0; x < RECT_SOLVER_WIDTH; x++)
            {
                if (currentScreen[y][x] != frame[y][x])
                {
                    SetPixel(x, y, frame[y][x]);

                    currentScreen[y][x] = frame[y][x];
                }
            }
        }
        break;
    case SLOW_BUT_SMOOTH:
        for (int y = 0; y < RECT_SOLVER_HEIGHT; y++)
        {
            for (int x = 0; x < RECT_SOLVER_WIDTH; x++)
            {
                if (currentScreen[y][x] != frame[y][x])
                {
                    // printf("Setting pixel %d,%d: %d\n", x, y, frame[y][x]);
                    AddColor(frame[y][x]);

                    screenChange[y][x] = IndexOfColor(frame[y][x]) + 1;

                    currentScreen[y][x] = frame[y][x];
                }
            }
        }

        for (int i = 0; i < palleteColorCount; i++)
        {
            DrawShapeLayerSolved(i + 1, pallete[i]);
        }
        break;
    }
}

// Draw functions
void SmartRect(uint16_t color, int x, int y, int w, int h)
{
    if (renderMode == FAST_BUT_FLICKER)
    {
        Rectangle(color, x, y, w, h);
        return;
    }

    if (x > RECT_SOLVER_WIDTH)
    {
        return;
    }
    if (y > RECT_SOLVER_HEIGHT)
    {
        return;
    }

    if (x + w < 0)
    {
        return;
    }
    if (y + h < 0)
    {
        return;
    }

    if (x < 0)
    {
        w += x;
        x = 0;
    }
    if (y < 0)
    {
        h += y;
        y = 0;
    }

    w = min(w, RECT_SOLVER_WIDTH - x);
    h = min(h, RECT_SOLVER_HEIGHT - y);

    for (int x2 = 0; x2 < w; x2++)
    {
        for (int y2 = 0; y2 < h; y2++)
        {
            smartScreen[y + y2][x + x2] = color;
        }
    }
}

void SmartShow()
{
    if (renderMode == FAST_BUT_FLICKER)
    {
        return;
    }
    DrawScreenSmart(smartScreen);
}

void SmartShowAll()
{
    memset(currentScreen, 0, sizeof(currentScreen));
    SmartShow();
}

void SmartClear()
{
    if (renderMode == FAST_BUT_FLICKER)
    {
        Clear();
        memset(smartScreen, 0, sizeof(smartScreen));
        memset(currentScreen, 0, sizeof(currentScreen));
        return;
    }
    memset(smartScreen, 0, sizeof(smartScreen));
}

#endif