#ifndef SMART_RENDER
#define SMART_RENDER

#include <stdio.h>
#include "pico/stdlib.h"
#include "C:\Users\kphoh\Documents\RP pico\lib\TFT.h"

#define RECT_SOLVER_WIDTH 160
#define RECT_SOLVER_HEIGHT 128

uint16_t currentScreen[RECT_SOLVER_HEIGHT][RECT_SOLVER_WIDTH];
uint16_t smartScreen[RECT_SOLVER_HEIGHT][RECT_SOLVER_WIDTH];

typedef struct ColorList
{
    struct ColorList *next;
    struct ColorList *previous;
    uint16_t color;
    bool affected[RECT_SOLVER_HEIGHT][RECT_SOLVER_WIDTH];
} ColorList;

ColorList *palleteHead = NULL;
uint16_t palleteColorCount = 0;

uint8_t IndexOfColor(uint16_t color)
{
    if (palleteHead == NULL)
    {
        return 255;
    }

    ColorList *currentColor = palleteHead;
    uint16_t index = 0;
    while (currentColor->previous != NULL)
    {
        currentColor = currentColor->previous;
    }
    while (currentColor->color != color)
    {
        if (currentColor->next == NULL)
        {
            return 255;
        }
        currentColor = currentColor->next;
        index++;
    }
    return index;
}
ColorList *GetColorNodeByColor(uint16_t color)
{
    ColorList *currentColor = palleteHead;

    while (currentColor->previous != NULL)
    {
        currentColor = currentColor->previous;
    }
    while (currentColor->color != color)
    {
        if (currentColor->next == NULL)
        {
            return NULL;
        }
        currentColor = currentColor->next;
    }
    return currentColor;
}
void AddColorToPallete(uint16_t color)
{
    if (IndexOfColor(color) != 255)
    {
        return;
    }

    if (palleteHead == NULL)
    {
        palleteHead = malloc(sizeof(ColorList));
        palleteHead->color = color;
        palleteHead->previous = NULL;
        palleteHead->next = NULL;
        palleteColorCount++;
        memset(palleteHead->affected, 0, sizeof(palleteHead->affected));
        return;
    }
    ColorList *newColor = malloc(sizeof(ColorList));
    palleteHead->next = newColor;
    newColor->previous = palleteHead;
    newColor->color = color;

    palleteHead = newColor;
    memset(palleteHead->affected, 0, sizeof(palleteHead->affected));

    palleteColorCount++;
}
uint16_t GetColorFromPallete(uint8_t colorIndex)
{
    ColorList *currentColor = palleteHead;
    while (currentColor->previous != NULL)
    {
        currentColor = currentColor->previous;
    }
    for (int i = 0; i < colorIndex; i++)
    {
        if (currentColor->next == NULL)
        {
            return BLACK;
        }
        currentColor = currentColor->next;
    }
    return currentColor->color;
}
ColorList *GetColorNodeByIndex(uint8_t colorIndex)
{
    ColorList *currentColor = palleteHead;
    while (currentColor->previous != NULL)
    {
        currentColor = currentColor->previous;
    }
    for (int i = 0; i < colorIndex; i++)
    {
        if (currentColor->next == NULL)
        {
            return NULL;
        }
        currentColor = currentColor->next;
    }
    return currentColor;
}

void ClearPallete()
{
    if (palleteHead == NULL)
    {
        return;
    }
    ColorList *currentColor = palleteHead;
    while (currentColor->previous != NULL)
    {
        currentColor = currentColor->previous;
        free(currentColor->next);
    }
    free(currentColor);
    palleteColorCount = 0;
    palleteHead = NULL;
}

void DrawShapeSolved(bool shape[RECT_SOLVER_HEIGHT][RECT_SOLVER_WIDTH], uint16_t color)
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

            if (!shape[y][x])
            {
                continue;
            }

            uint8_t width = 1;
            while (shape[y][x + width] && !renderedPixels[y][x + width] && (width + x < RECT_SOLVER_WIDTH))
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
                    if (!shape[y + height][x + x2])
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

void DrawScreenSmart(uint16_t frame[RECT_SOLVER_HEIGHT][RECT_SOLVER_WIDTH])
{
    // memset(currentScreen,0,sizeof(currentScreen));

    ClearPallete();

    for (int y = 0; y < RECT_SOLVER_HEIGHT; y++)
    {
        for (int x = 0; x < RECT_SOLVER_WIDTH; x++)
        {
            if (currentScreen[y][x] != frame[y][x])
            {

                AddColorToPallete(frame[y][x]);

                GetColorNodeByColor(frame[y][x])->affected[y][x] = true;

                currentScreen[y][x] = frame[y][x];
            }
        }
    }

    for (int i = 0; i < palleteColorCount; i++)
    {
        DrawShapeSolved(GetColorNodeByIndex(i)->affected, GetColorFromPallete(i));
    }
}

void SmartRect(uint16_t color, int x, int y, uint8_t w, uint8_t h)
{
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
        h += y - 0;
        y = 0;
    }

    w = min(w, RECT_SOLVER_WIDTH - x);
    h = min(h, RECT_SOLVER_HEIGHT - y);
    

    for (int x2 = 0; x2 < w; x2++)
    {
        for (int y2 = 0; y2 < h; y2++)
        {
            smartScreen[y+y2][x+x2] = color;
        }
    }
}

void SmartShow()
{
    DrawScreenSmart(smartScreen);
}

void SmartClear()
{
    memset(smartScreen, 0, sizeof(smartScreen));
}

#endif