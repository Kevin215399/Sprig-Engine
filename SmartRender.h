#ifndef SMART_RENDER
#define SMART_RENDER

#include <stdio.h>
#include "pico/stdlib.h"
#include "TFT.h"

#include "LinkedList.h"

#define PI 3.1415926

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

float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

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

void SmartBottomTriangle(uint16_t color, int b1X, int b2X, int bY, int vX, int vY)
{
    if (b2X < b1X)
    {
        int temp = b2X;
        b2X = b1X;
        b1X = temp;
    }

    float slope1 = (float)(b1X - vX) / (float)(bY - vY);
    float slope2 = (float)(b2X - vX) / (float)(bY - vY);

    float x1 = vX;
    float x2 = vX;

    for (int y = vY; y <= bY; y++)
    {
        SmartRect(color, x1, y, x2 - x1, 1);
        x1 += slope1;
        x2 += slope2;
    }
}
void SmartTopTriangle(uint16_t color, int b1X, int b2X, int bY, int vX, int vY)
{
    if (b2X < b1X)
    {
        int temp = b2X;
        b2X = b1X;
        b1X = temp;
    }
    float slope1 = (float)(b1X - vX) / (float)(bY - vY);
    float slope2 = (float)(b2X - vX) / (float)(bY - vY);

    float x1 = vX;
    float x2 = vX;

    for (int y = vY; y > bY; y--)
    {
        SmartRect(color, x1, y, x2 - x1, 1);
        x1 -= slope1;
        x2 -= slope2;
    }
}

void SmartTriangle(uint16_t color, int p1X, int p1Y, int p2X, int p2Y, int p3X, int p3Y)
{
    if (p2Y < p1Y)
    {
        int temp = p2X;
        p2X = p1X;
        p1X = temp;

        temp = p2Y;
        p2Y = p1Y;
        p1Y = temp;
    }
    if (p3Y < p1Y)
    {
        int temp = p3X;
        p3X = p1X;
        p1X = temp;

        temp = p3Y;
        p3Y = p1Y;
        p1Y = temp;
    }
    if (p3Y < p2Y)
    {
        int temp = p3X;
        p3X = p2X;
        p2X = temp;

        temp = p3Y;
        p3Y = p2Y;
        p2Y = temp;
    }

    if (p2Y == p3Y)
    {
        SmartBottomTriangle(color, p2X, p3X, p2Y, p1X, p1Y);
    }
    else if (p2Y == p1Y)
    {
        SmartTopTriangle(color, p2X, p1X, p2Y, p3X, p3Y);
    }
    else
    {
        int p4X = (int)(p1X + ((float)(p2Y - p1Y) / (float)(p3Y - p1Y)) * (p3X - p1X));

        SmartBottomTriangle(color, p2X, p4X, p2Y, p1X, p1Y);
        SmartTopTriangle(color, p2X, p4X, p2Y, p3X, p3Y);
    }
}

void SmartLetter(char letter, int startX, int startY, uint8_t scale, uint16_t color, uint16_t backgroundColor)
{
    if (backgroundColor != TRANSPARENT)
    {
        uint8_t bufferWidth = 5 * scale + (scale - 1) * 2;
        uint8_t bufferHeight = 7 * scale + (scale - 1) * 2;

        for (int x = 0; x < bufferWidth; x++)
        {
            for (int y = 0; y < bufferHeight; y++)
            {
                SmartRect(backgroundColor, startX + x, startY + y, 1, 1);
            }
        }
    }

    uint8_t letterIndex = 255;
    for (int i = 0; i < sizeof(fontKeys) / sizeof(char); i++)
    {
        if (fontKeys[i] == letter)
        {
            letterIndex = i;
            break;
        }
    }
    if (letterIndex == 255)
    {
        return;
    }

    for (int y = 0; y < 7; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            if (font[letterIndex * 35 + x + (y * 5)])
            {
                for (int dx = -scale + 1; dx < scale; dx++)
                {
                    for (int dy = -scale + 1; dy < scale; dy++)
                    {
                        SmartRect(color, x * scale + dx + startX + (scale - 1), y * scale + dy + startY + (scale - 1), 1, 1);
                    }
                }
            }
        }
    }
}

uint8_t SmartWord(char *word, int len, int x, int y, uint8_t scale, uint16_t color, uint16_t background)
{
    uint8_t letterWidth = 5 * scale + (scale - 1) * 2 + 1;
    uint8_t currentLine = 0;
    int xOffset = 0;
    for (int i = 0; i < len; i++)
    {
        if (x + xOffset * letterWidth + FONT_WIDTHS[scale] >= 155)
        {
            xOffset = 0;
            currentLine++;
        }
        if (word[i] == '\n')
        {
            xOffset = 0;
            currentLine++;
            continue;
        }
        if (background != TRANSPARENT)
        {
            uint8_t bufferHeight = 7 * scale + (scale - 1) * 2;
            if (currentLine == 0)
            {
                SmartRect(background, x + xOffset * letterWidth - 1, y + (FONT_HEIGHTS[scale - 1] * currentLine) - 1, letterWidth + 2, bufferHeight + 2);
            }
            else
            {
                SmartRect(background, x + xOffset * letterWidth - 1, y + (FONT_HEIGHTS[scale - 1] * currentLine), letterWidth + 2, bufferHeight + 1);
            }
        }
        SmartLetter(word[i], x + xOffset * letterWidth, y + ((FONT_HEIGHTS[scale - 1] + 1) * currentLine), scale, color, background);
        xOffset++;
    }

    return currentLine + 1;
}

void SmartLine(uint16_t color, int x1, int y1, int x2, int y2)
{
    float distance = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
    for (int i = 0; i < distance; i++)
    {
        int x = Lerp(x1, x2, (float)i / (float)distance);
        int y = Lerp(y1, y2, (float)i / (float)distance);
        smartScreen[y][x] = color;
    }
}
void SmartRectAngled(uint16_t color, int cX, int cY, int w, int h, double angle)
{
    Vector2 TL;
    Vector2 TR;
    Vector2 BL;
    Vector2 BR;

    TL.x = (-(double)w / 2) * cos(angle * (PI / 180)) - (-(double)h / 2) * sin(angle * (PI / 180)) + cX;
    TL.y = (-(double)w / 2) * sin(angle * (PI / 180)) + (-(double)h / 2) * cos(angle * (PI / 180)) + cY;

    TR.x = ((double)w / 2) * cos(angle * (PI / 180)) - (-(double)h / 2) * sin(angle * (PI / 180)) + cX;
    TR.y = ((double)w / 2) * sin(angle * (PI / 180)) + (-(double)h / 2) * cos(angle * (PI / 180)) + cY;

    BL.x = (-(double)w / 2) * cos(angle * (PI / 180)) - ((double)h / 2) * sin(angle * (PI / 180)) + cX;
    BL.y = (-(double)w / 2) * sin(angle * (PI / 180)) + ((double)h / 2) * cos(angle * (PI / 180)) + cY;

    BR.x = ((double)w / 2) * cos(angle * (PI / 180)) - ((double)h / 2) * sin(angle * (PI / 180)) + cX;
    BR.y = ((double)w / 2) * sin(angle * (PI / 180)) + ((double)h / 2) * cos(angle * (PI / 180)) + cY;

    SmartLine(color, TL.x, TL.y, TR.x, TR.y);
    SmartLine(color, TR.x, TR.y, BR.x, BR.y);
    SmartLine(color, BR.x, BR.y, BL.x, BL.y);
    SmartLine(color, BL.x, BL.y, TL.x, TL.y);
}
void SmartFilledRect(uint16_t color, int cX, int cY, int w, int h, double angle)
{
    if ((int)angle % 360 == 0 || (int)angle % 360 == 180)
    {
        SmartRect(color, cX - w / 2, cY - h / 2, w, h);
        return;
    }
    if ((int)angle % 360 == 90 || (int)angle % 360 == 270)
    {
        SmartRect(color, cX - h / 2, cY - w / 2, h, w);
        return;
    }

    Vector2 TL;
    Vector2 TR;
    Vector2 BL;
    Vector2 BR;

    TL.x = (-(double)w / 2) * cos(angle * (PI / 180)) - (-(double)h / 2) * sin(angle * (PI / 180)) + cX;
    TL.y = (-(double)w / 2) * sin(angle * (PI / 180)) + (-(double)h / 2) * cos(angle * (PI / 180)) + cY;

    TR.x = ((double)w / 2) * cos(angle * (PI / 180)) - (-(double)h / 2) * sin(angle * (PI / 180)) + cX;
    TR.y = ((double)w / 2) * sin(angle * (PI / 180)) + (-(double)h / 2) * cos(angle * (PI / 180)) + cY;

    BL.x = (-(double)w / 2) * cos(angle * (PI / 180)) - ((double)h / 2) * sin(angle * (PI / 180)) + cX;
    BL.y = (-(double)w / 2) * sin(angle * (PI / 180)) + ((double)h / 2) * cos(angle * (PI / 180)) + cY;

    BR.x = ((double)w / 2) * cos(angle * (PI / 180)) - ((double)h / 2) * sin(angle * (PI / 180)) + cX;
    BR.y = ((double)w / 2) * sin(angle * (PI / 180)) + ((double)h / 2) * cos(angle * (PI / 180)) + cY;

    SmartTriangle(color, TL.x, TL.y, TR.x, TR.y, BR.x, BR.y);
    SmartTriangle(color, TR.x, TR.y, BR.x, BR.y, BL.x, BL.y);
    SmartTriangle(color, BR.x, BR.y, BL.x, BL.y, TL.x, TL.y);
    SmartTriangle(color, BL.x, BL.y, TL.x, TL.y, TR.x, TR.y);

    if (w > 3 && h > 3)
        SmartRect(color, cX - 1, cY - 1, 3, 3);
}
/*
void Fill(uint16_t fill, uint16_t avoid, int cX, int cY)
{
    FillNode *openNodes;
    PushNode(cX, cY, &openNodes);

    int openCount = 1;

    int minX = cX;
    int minY = cY;
    int maxX = cX;
    int maxY = cY;

    while (openCount > 0)
    {
        Vector2 current = PopNode(&openNodes);
        smartScreen[(int)current.y][(int)current.x] = TRANSPARENT;
        openCount--;
        if (current.x < minX)
            minX = current.x;
        if (current.y < minY)
            minY = current.y;
        if (current.x > maxX)
            maxX = current.x;
        if (current.y > maxY)
            maxY = current.y;

        if (smartScreen[(int)current.y][(int)current.x + 1] != TRANSPARENT && smartScreen[(int)current.y][(int)current.x + 1] != avoid)
        {
            PushNode(current.x + 1, current.y, &openNodes);
            openCount++;
        }

        if (smartScreen[(int)current.y][(int)current.x - 1] != TRANSPARENT && smartScreen[(int)current.y][(int)current.x - 1] != avoid)
        {
            PushNode(current.x - 1, current.y, &openNodes);
            openCount++;
        }

        if (smartScreen[(int)current.y + 1][(int)current.x] != TRANSPARENT && smartScreen[(int)current.y + 1][(int)current.x] != avoid)
        {
            PushNode(current.x, current.y + 1, &openNodes);
            openCount++;
        }

        if (smartScreen[(int)current.y - 1][(int)current.x] != TRANSPARENT && smartScreen[(int)current.y - 1][(int)current.x] != avoid)
        {
            PushNode(current.x, current.y - 1, &openNodes);
            openCount++;
        }
    }
    for (int x = minX; x <= maxX; x++)
    {
        for (int y = minY; y <= maxY; y++)
        {
            if (smartScreen[y][x] == TRANSPARENT)
                smartScreen[y][x] = fill;
        }
    }
}
*/
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

void SmartClearAll()
{
    memset(smartScreen, 0, sizeof(smartScreen));
    memset(currentScreen, 0, sizeof(currentScreen));
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

void BufferSpriteRects(uint16_t sprite[SPRITE_WIDTH][SPRITE_HEIGHT], uint16_t color, float dx, float dy, float scaleX, float scaleY, int angle)
{
    angle %= 360;
    print("solving");
    bool renderedPixels[SPRITE_WIDTH][SPRITE_HEIGHT];

    memset(renderedPixels, 0, sizeof(renderedPixels));

    for (int y = 0; y < SPRITE_HEIGHT; y++)
    {
        for (int x = 0; x < SPRITE_WIDTH; x++)
        {
            if (renderedPixels[x][y])
            {
                continue;
            }

            if (sprite[x][y] != color)
            {
                continue;
            }

            uint8_t width = 1;
            while (sprite[x + width][y] == color && !renderedPixels[x + width][y] && (width + x < SPRITE_WIDTH))
            {
                width++;
            }

            bool doExtend = true;
            uint8_t height = 1;

            while (doExtend)
            {
                if (y + height >= SPRITE_HEIGHT)
                {
                    break;
                }
                doExtend = true;
                for (int x2 = 0; x2 < width; x2++)
                {
                    if (sprite[x + x2][y + height] != color)
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
                    renderedPixels[x + x2][y + y2] = true;
                }
            }
            float originalX = (float)((float)x - SPRITE_WIDTH / 2 + (float)width / 2) * (float)scaleX;
            float originalY = (float)((float)y - SPRITE_HEIGHT / 2 + (float)height / 2) * (float)scaleY;

            float xPrime = (originalX * cos((float)angle * (PI / 180)) - originalY * sin((float)angle * (PI / 180))) + dx + SPRITE_WIDTH / 2 * scaleX;
            float yPrime = (originalX * sin((float)angle * (PI / 180)) + originalY * cos((float)angle * (PI / 180))) + dy + SPRITE_HEIGHT / 2 * scaleY;

            // printf("Rectangle: (%f,%f), width: %f, height %f\n", xPrime, yPrime, width * scaleX, height * scaleY);

            SmartFilledRect(color, xPrime + 1, yPrime - 1, width * scaleX + 1, height * scaleY, angle);
        }
    }
}

void SmartSprite(uint16_t sprite[SPRITE_HEIGHT][SPRITE_WIDTH], float dx, float dy, float scaleX, float scaleY, int angle)
{
    angle %= 360;
    ClearPallete();
    for (int y = 0; y < SPRITE_HEIGHT; y++)
    {
        for (int x = 0; x < SPRITE_WIDTH; x++)
        {
            if (sprite[y][x] == TRANSPARENT)
                continue;
            // printf("add color %d,%d: %d\n", x, y, sprite[y][x]);
            AddColor(sprite[y][x]);
        }
    }

    for (int i = 0; i < palleteColorCount; i++)
    {
        BufferSpriteRects(sprite, pallete[i], dx, dy, scaleX, scaleY, angle);
    }
}

void FillSimilar(uint16_t fillColor, uint16_t replaceColor, Vector2 start)
{
    GeneralList openNodes = {0};
    Vector2 *newPos = malloc(sizeof(Vector2));
    newPos->x = start.x;
    newPos->y = start.y;
    PushList(&openNodes, newPos);
    while (openNodes.count > 0)
    {
        Vector2 *node = (Vector2 *)PopList(&openNodes);

        smartScreen[(int)node->y][(int)node->x] = fillColor;

        if (node->y < 128 && smartScreen[(int)node->y + 1][(int)node->x] == replaceColor)
        {
            Vector2 *newPos = malloc(sizeof(Vector2));
            newPos->x = node->x;
            newPos->y = node->y + 1;
            PushList(&openNodes, newPos);
        }
        if (node->y > 0 && smartScreen[(int)node->y - 1][(int)node->x] == replaceColor)
        {
            Vector2 *newPos = malloc(sizeof(Vector2));
            newPos->x = node->x;
            newPos->y = node->y - 1;
            PushList(&openNodes, newPos);
        }
        if (node->x < 160 && smartScreen[(int)node->y][(int)node->x + 1] == replaceColor)
        {
            Vector2 *newPos = malloc(sizeof(Vector2));
            newPos->x = node->x + 1;
            newPos->y = node->y;
            PushList(&openNodes, newPos);
        }
        if (node->x > 0 && smartScreen[(int)node->y][(int)node->x - 1] == replaceColor)
        {
            Vector2 *newPos = malloc(sizeof(Vector2));
            newPos->x = node->x - 1;
            newPos->y = node->y;
            PushList(&openNodes, newPos);
        }

        free(node);
    }
}

#endif