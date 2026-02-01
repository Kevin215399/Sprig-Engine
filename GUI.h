#ifndef GUI
#define GUI

#include <stdio.h>
#include "pico/stdlib.h"
#include "EngineStructs.h"
#include "TFT.h"

#define CENTER_ANCHOR 0
#define LEFT_ANCHOR 1
#define RIGHT_ANCHOR 2

typedef struct UIButton
{
    uint8_t x;
    uint8_t y;

    uint8_t width;
    uint8_t height;

    uint8_t curveRadius;

    uint16_t fillColor;
    uint16_t normalOutline;
    uint16_t focusOutline;
    uint16_t pressedFill;

    bool isFocused;
    bool isPressed;
    bool lastPressedState;

    struct UIButton *upNext;
    struct UIButton *leftNext;
    struct UIButton *rightNext;
    struct UIButton *downNext;

    char *text;
    uint16_t fontColor;
    uint8_t fontSize;
    uint16_t icon[16][16];

    uint8_t textAnchor;

    bool hasText;
    bool hasIcon;

    bool onPressDown;

    bool visible;
} UIButton;

typedef struct
{
    uint8_t x;
    uint8_t y;

    uint8_t width;
    uint8_t height;

    uint8_t curveRadius;

    uint16_t fillColor;
    uint16_t outline;

    char *text;
    uint16_t fontColor;
    uint8_t fontSize;
    uint16_t icon[16][16];

    bool hasText;
    bool hasIcon;

    bool visible;

} UIPanel;




void DrawButton(UIButton *button)
{
    if (!button->visible)
    {
        return;
    }

    uint16_t outline = button->isFocused ? button->focusOutline : button->normalOutline;
    uint16_t fill = button->isPressed ? button->pressedFill : button->fillColor;

    // horizontal fill
    Rectangle(fill, button->x + 1, button->y + button->curveRadius, button->width - 2, button->height - button->curveRadius * 2);

    // corners
    for (int i = 0; i < 90; i += 5)
    {
        float radians = i * (M_PI / 180);

        float x = cos(radians) * button->curveRadius;
        float y = sin(radians) * button->curveRadius;

        // top
        Rectangle(fill, button->x + button->curveRadius - x + 1, button->y + button->curveRadius - y, (button->x + button->width - button->curveRadius + x) - (button->x + button->curveRadius - x) - 1, 1);

        // bottom
        Rectangle(fill, button->x + button->curveRadius - x + 1, button->y + button->height - button->curveRadius + y, (button->x + button->width - button->curveRadius + x) - (button->x + button->curveRadius - x) - 1, 1);

        // TR
        SetPixel(min(button->x + button->width - button->curveRadius + x, button->x + button->width - 1), max(button->y + button->curveRadius - y, button->y), outline);
        // TL
        SetPixel(max(button->x + button->curveRadius - x, button->x), max(button->y + button->curveRadius - y, button->y), outline);
        // BL
        SetPixel(max(button->x + button->curveRadius - x, button->x), min(button->y + button->height - button->curveRadius + y, button->y + button->height - 1), outline);
        // BR
        SetPixel(min(button->x + button->width - button->curveRadius + x, button->x + button->width - 1), min(button->y + button->height - button->curveRadius + y, button->y + button->height - 1), outline);
    }

    // left wall
    Rectangle(outline, button->x, button->y + button->curveRadius, 1, button->height - button->curveRadius * 2);
    // right wall
    Rectangle(outline, button->x + button->width - 1, button->y + button->curveRadius, 1, button->height - button->curveRadius * 2);

    // top wall
    Rectangle(outline, button->x + button->curveRadius, button->y, button->width - button->curveRadius * 2, 1);
    // bottom wall
    Rectangle(outline, button->x + button->curveRadius, button->y + button->height - 1, button->width - button->curveRadius * 2, 1);

    if (button->hasText)
    {
        WriteWord(button->text,
                  strlen(button->text),
                  (button->x + (button->width / 2)) - (strlen(button->text) * (FONT_WIDTHS[button->fontSize - 1] + 1) / 2),
                  (button->y + (button->height / 2)) - (FONT_HEIGHTS[button->fontSize - 1] / 2),
                  button->fontSize,
                  button->fontColor,
                  TRANSPARENT);
    }
    if (button->hasIcon)
    {
        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 16; y++)
            {
                if (button->icon[y][x] == TRANSPARENT)
                {
                    continue;
                }
                Rectangle(button->icon[y][x], (button->x + button->width / 2) + x - 8, (button->y + button->height / 2) + y - 8, 1, 1);
            }
        }
    }
}

void DrawPanel(UIPanel *panel)
{
    if (!panel->visible)
    {
        return;
    }

    uint16_t outline = panel->outline;
    uint16_t fill = panel->fillColor;

    // horizontal fill
    Rectangle(fill, panel->x + 1, panel->y + panel->curveRadius, panel->width - 2, panel->height - panel->curveRadius * 2);

    // corners
    for (int i = 0; i < 90; i += 1)
    {
        float radians = i * (M_PI / 180);

        float x = cos(radians) * panel->curveRadius;
        float y = sin(radians) * panel->curveRadius;

        // top
        Rectangle(fill, panel->x + panel->curveRadius - x + 1, panel->y + panel->curveRadius - y, (panel->x + panel->width - panel->curveRadius + x) - (panel->x + panel->curveRadius - x) - 1, 1);

        // bottom
        Rectangle(fill, panel->x + panel->curveRadius - x + 1, panel->y + panel->height - panel->curveRadius + y, (panel->x + panel->width - panel->curveRadius + x) - (panel->x + panel->curveRadius - x) - 1, 1);

        // TR
        Rectangle(outline, min(panel->x + panel->width - panel->curveRadius + x, panel->x + panel->width - 1), max(panel->y + panel->curveRadius - y, panel->y), 1, 1);
        // TL
        Rectangle(outline, max(panel->x + panel->curveRadius - x, panel->x), max(panel->y + panel->curveRadius - y, panel->y), 1, 1);
        // BL
        Rectangle(outline, max(panel->x + panel->curveRadius - x, panel->x), min(panel->y + panel->height - panel->curveRadius + y, panel->y + panel->height - 1), 1, 1);
        // BR
        Rectangle(outline, min(panel->x + panel->width - panel->curveRadius + x, panel->x + panel->width - 1), min(panel->y + panel->height - panel->curveRadius + y, panel->y + panel->height - 1), 1, 1);
    }

    // left wall
    Rectangle(outline, panel->x, panel->y + panel->curveRadius, 1, panel->height - panel->curveRadius * 2);
    // right wall
    Rectangle(outline, panel->x + panel->width - 1, panel->y + panel->curveRadius, 1, panel->height - panel->curveRadius * 2);

    // top wall
    Rectangle(outline, panel->x + panel->curveRadius, panel->y, panel->width - panel->curveRadius * 2, 1);
    // bottom wall
    Rectangle(outline, panel->x + panel->curveRadius, panel->y + panel->height - 1, panel->width - panel->curveRadius * 2, 1);

    if (panel->hasText)
    {
        WriteWord(panel->text,
                  strlen(panel->text),
                  (panel->x + (panel->width / 2)) - (strlen(panel->text) * (FONT_WIDTHS[panel->fontSize - 1] + 1) / 2),
                  (panel->y + (panel->height / 2)) - (FONT_HEIGHTS[panel->fontSize - 1] / 2),
                  panel->fontSize,
                  panel->fontColor,
                  TRANSPARENT);
    }
    if (panel->hasIcon)
    {
        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 16; y++)
            {
                if (panel->icon[y][x] == TRANSPARENT)
                {
                    continue;
                }
                Rectangle(panel->icon[y][x], (panel->x + panel->width / 2) + x - 8, (panel->y + panel->height / 2) + y - 8, 1, 1);
            }
        }
    }
}


void SetButton(UIButton *button,
               uint8_t x,
               uint8_t y,
               uint8_t width,
               uint8_t height,
               uint8_t curveRadius,
               uint16_t fillColor,
               uint16_t pressedFill,
               uint16_t normalOutline,
               uint16_t focusOutline,
               UIButton *up,
               UIButton *right,
               UIButton *down,
               UIButton *left)
{
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    button->curveRadius = curveRadius;
    button->fillColor = fillColor;
    button->pressedFill = pressedFill;
    button->normalOutline = normalOutline;
    button->focusOutline = focusOutline;
    button->upNext = up;
    button->rightNext = right;
    button->downNext = down;
    button->leftNext = left;
    button->visible = true;
}

void SetPanel(UIPanel *panel,
              uint8_t x,
              uint8_t y,
              uint8_t width,
              uint8_t height,
              uint8_t curveRadius,
              uint16_t fillColor,
              uint16_t outline)
{
    panel->x = x;
    panel->y = y;
    panel->width = width;
    panel->height = height;
    panel->curveRadius = curveRadius;
    panel->fillColor = fillColor;
    panel->outline = outline;
    panel->visible = true;
}

void AddTextToButton(UIButton *button, char *text, uint16_t color, uint8_t fontSize)
{
    button->text = malloc(strlen(text) + 1);
    strcpy(button->text, text);
    button->fontColor = color;
    button->fontSize = fontSize;
    button->hasText = true;
}
void AddIconToButton(UIButton *button, const uint16_t icon[16][16])
{
    memcpy(button->icon, icon, sizeof(uint16_t) * 16 * 16);
    button->hasIcon = true;
}

void AddTextToPanel(UIPanel *panel, char *text, uint16_t color, uint8_t fontSize)
{
    panel->text = text;
    panel->fontColor = color;
    panel->fontSize = fontSize;
    panel->hasText = true;
}
void AddIconToPanel(UIPanel *panel, const uint16_t icon[16][16])
{
    memcpy(panel->icon, icon, sizeof(uint16_t) * 16 * 16);
    panel->hasIcon = true;
}
#endif