#ifndef SCENE_VIEWER
#define SCENE_VIEWER

#include <stdio.h>
#include "pico/stdlib.h"

#include "SaveSystem.h"
#include "C:\Users\kphoh\Documents\RP pico\lib\TFT.h"

#include "Keyboard.h"

#include "InputManager.h"

#include "EngineStructs.h"
#include <string.h>

#include "pico/time.h"
#include "GUI_Icons.h"

#include "SmartRender.h"

// #include "Interpreter.h"

#define CENTER_ANCHOR 0
#define LEFT_ANCHOR 1
#define RIGHT_ANCHOR 2

uint8_t anchorType = CENTER_ANCHOR;

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

#define SELECT_OBJECTS_UI 0
#define SELECT_LAYOUT_UI 1
#define SELECT_MODULE_UI 2
#define SELECT_OPTIONS_UI 3

#define OPEN_NAV_PANEL 4

#define PLAY_SCENE_UI 5
#define EXIT_EDITOR_UI 6

#define RENDER_MODE_BUTTON 7

#define OBJECT_BUTTONS 8

#define MODULE_BUTTONS (8 + MAX_OBJECTS)

#define MAIN_PANEL 0
#define OBJECT_PANEL 1
#define NAV_PANEL 2

#define MODULE_PANELS (MAX_SCRIPTS_PER_OBJECT)

bool NavPanelOpen = 0;

UIButton buttons[8 + MAX_OBJECTS + (MAX_SCRIPTS_PER_OBJECT * 10)];

UIPanel panels[3];

UIButton *currentButton = NULL;
bool sceneRefreshUI = true;

EngineScene *currentScene;

// Button and Panel functions
#pragma region

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
    for (int i = 0; i < 90; i += 1)
    {
        float radians = i * (M_PI / 180);

        float x = cos(radians) * button->curveRadius;
        float y = sin(radians) * button->curveRadius;

        // top
        Rectangle(fill, button->x + button->curveRadius - x + 1, button->y + button->curveRadius - y, (button->x + button->width - button->curveRadius + x) - (button->x + button->curveRadius - x) - 1, 1);

        // bottom
        Rectangle(fill, button->x + button->curveRadius - x + 1, button->y + button->height - button->curveRadius + y, (button->x + button->width - button->curveRadius + x) - (button->x + button->curveRadius - x) - 1, 1);

        // TR
        Rectangle(outline, min(button->x + button->width - button->curveRadius + x, button->x + button->width - 1), max(button->y + button->curveRadius - y, button->y), 1, 1);
        // TL
        Rectangle(outline, max(button->x + button->curveRadius - x, button->x), max(button->y + button->curveRadius - y, button->y), 1, 1);
        // BL
        Rectangle(outline, max(button->x + button->curveRadius - x, button->x), min(button->y + button->height - button->curveRadius + y, button->y + button->height - 1), 1, 1);
        // BR
        Rectangle(outline, min(button->x + button->width - button->curveRadius + x, button->x + button->width - 1), min(button->y + button->height - button->curveRadius + y, button->y + button->height - 1), 1, 1);
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

void UpdateUIButtons()
{
    if (GetButton() == BUTTON_A && currentButton->leftNext != NULL)
    {
        currentButton->isFocused = false;
        DrawButton(currentButton);
        currentButton = currentButton->leftNext;
        currentButton->isFocused = true;
        DrawButton(currentButton);

        sleep_ms(120);
    }

    if (GetButton() == BUTTON_D && currentButton->rightNext != NULL)
    {
        currentButton->isFocused = false;
        DrawButton(currentButton);
        currentButton = currentButton->rightNext;
        currentButton->isFocused = true;
        DrawButton(currentButton);

        sleep_ms(120);
    }

    if (GetButton() == BUTTON_W && currentButton->upNext != NULL)
    {
        currentButton->isFocused = false;
        DrawButton(currentButton);
        currentButton = currentButton->upNext;
        currentButton->isFocused = true;
        DrawButton(currentButton);

        sleep_ms(120);
    }

    if (GetButton() == BUTTON_S && currentButton->downNext != NULL)
    {
        currentButton->isFocused = false;
        DrawButton(currentButton);
        currentButton = currentButton->downNext;
        currentButton->isFocused = true;
        DrawButton(currentButton);

        sleep_ms(120);
    }

    for (int i = 0; i < sizeof(buttons) / sizeof(UIButton); i++)
    {

        if (&buttons[i] == currentButton && GetButton() == BUTTON_J)
        {
            buttons[i].isPressed = true;
            if (buttons[i].isPressed != buttons[i].lastPressedState)
            {
                DrawButton(&buttons[i]);
                buttons[i].onPressDown = true;
            }
            else
            {
                buttons[i].onPressDown = false;
            }
            buttons[i].lastPressedState = true;
        }
        else
        {
            buttons[i].onPressDown = false;
            buttons[i].isPressed = false;
            if (buttons[i].isPressed != buttons[i].lastPressedState)
            {
                DrawButton(&buttons[i]);
            }
            buttons[i].lastPressedState = false;
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
    button->text = text;
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

#pragma endregion

// Scene renderers
#pragma region

uint8_t sceneScale = 4;

void DrawSpriteCentered(uint8_t sprite, int x, int y, uint8_t scale)
{
    int topLeftX = x - scale * SPRITE_WIDTH / 2;
    int topLeftY = y - scale * SPRITE_HEIGHT / 2;

    for (int x = 0; x < SPRITE_WIDTH; x++)
    {
        for (int y = 0; y < SPRITE_HEIGHT; y++)
        {
            // printf("Color: %d\n", sprites[sprite].sprite[x][y]);
            SmartRect(sprites[sprite].sprite[x][y], topLeftX + x * scale, topLeftY + y * scale, scale, scale);
        }
    }
}

void RenderScene(int offsetX, int offsetY)
{
    leftBound = 1;
    rightBound = 159;
    topBound = 1;
    bottomBound = 127;

    for (int i = leftBound; i < rightBound; i++)
    {
        if ((i - offsetX - 80) % (10 * sceneScale) == 0)
        {
            SmartRect(RGBTo16(70, 70, 70), i, topBound, 1, bottomBound - topBound);
        }
    }

    for (int i = topBound; i < bottomBound; i++)
    {
        if ((i - offsetY - 64) % (10 * sceneScale) == 0)
        {
            SmartRect(RGBTo16(70, 70, 70), leftBound, i, rightBound - leftBound, 1);
        }
    }

    for (int i = 0; i < currentScene->objectCount; i++)
    {
        // printf("Sprite: %d\n", currentScene->objects[i].objectData[1]->data.i);
        DrawSpriteCentered(currentScene->objects[i].objectData[1]->data.i, 80 + offsetX, 64 + offsetY, sceneScale);
    }

    SmartShow();

    leftBound = 0;
    rightBound = 160;
    topBound = 0;
    bottomBound = 128;
}

#pragma endregion

void SetNavPanelVisibility(bool visible)
{
    panels[NAV_PANEL].visible = visible;

    buttons[SELECT_OBJECTS_UI].visible = visible;
    buttons[SELECT_LAYOUT_UI].visible = visible;
    buttons[SELECT_MODULE_UI].visible = visible;
    buttons[SELECT_OPTIONS_UI].visible = visible;

    buttons[OPEN_NAV_PANEL].visible = !visible;

    Clear();
    // Draw background panels
    for (int i = 0; i < sizeof(panels) / sizeof(UIPanel); i++)
    {
        DrawPanel(&panels[i]);
    }
    // draw background buttons
    for (int i = 4; i < sizeof(buttons) / sizeof(UIButton); i++)
    {
        DrawButton(&buttons[i]);
    }

    // draw front buttons
    DrawPanel(&panels[NAV_PANEL]);
    for (int i = 0; i < 4; i++)
    {
        DrawButton(&buttons[i]);
    }
}

void HideAllButtons()
{
    panels[MAIN_PANEL].fillColor = RGBTo16(80, 80, 80);
    panels[OBJECT_PANEL].visible = false;
    for (int i = 0; i < sizeof(buttons) / sizeof(UIButton); i++)
    {
        buttons[i].visible = false;
    }
}

void RefocusButton(UIButton *focusTo, bool redraw)
{
    for (int i = 0; i < sizeof(buttons) / sizeof(UIButton); i++)
    {
        if (buttons[i].isFocused)
        {
            buttons[i].isFocused = false;
            if (redraw)
                DrawButton(&buttons[i]);
        }

        if (&buttons[i] == focusTo)
        {
            currentButton = &buttons[i];
            buttons[i].isFocused = true;
            if (redraw)
                DrawButton(&buttons[i]);
        }
    }
}

void UpdateRenderButton(bool draw)
{
    switch (renderMode)
    {
    case FAST_BUT_FLICKER:
        AddTextToButton(&buttons[RENDER_MODE_BUTTON], "FAST", RED, 1);
        break;
    case CHANGE_DRAW:
        AddTextToButton(&buttons[RENDER_MODE_BUTTON], "ON CHANGE", GREEN, 1);
        break;
    case SLOW_BUT_SMOOTH:
        AddTextToButton(&buttons[RENDER_MODE_BUTTON], "SMOOTH", YELLOW, 1);
        break;
    }
    if (draw)
        DrawButton(&buttons[RENDER_MODE_BUTTON]);
}

void ManageSceneUI()
{

    SetPanel(&panels[MAIN_PANEL], 0, 0, 160, 128, 6, RGBTo16(80, 80, 80), RGBTo16(120, 120, 120));
    SetPanel(&panels[NAV_PANEL], 80, 0, 80, 128, 6, RGBTo16(0, 0, 0), RGBTo16(120, 120, 120));

    SetPanel(&panels[OBJECT_PANEL], 5, 5, 150, 118, 6, RGBTo16(0, 0, 40), RGBTo16(120, 120, 120));

    SetButton(&buttons[SELECT_OBJECTS_UI], 85, 5, 70, 15, 7, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, NULL, NULL, &buttons[1], NULL);
    SetButton(&buttons[SELECT_LAYOUT_UI], 85, 22, 70, 15, 7, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, &buttons[0], NULL, &buttons[2], NULL);
    SetButton(&buttons[SELECT_MODULE_UI], 85, 39, 70, 15, 7, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, &buttons[1], NULL, &buttons[3], NULL);
    SetButton(&buttons[SELECT_OPTIONS_UI], 85, 56, 70, 15, 7, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, &buttons[2], NULL, NULL, NULL);

    SetButton(&buttons[PLAY_SCENE_UI], 10, 10, 60, 20, 6, RGBTo16(0, 167, 0), WHITE, BLACK, GREEN, NULL, &buttons[OPEN_NAV_PANEL], &buttons[EXIT_EDITOR_UI], NULL);
    SetButton(&buttons[EXIT_EDITOR_UI], 10, 35, 60, 20, 6, RGBTo16(255, 0, 0), WHITE, BLACK, GREEN, &buttons[PLAY_SCENE_UI], &buttons[OPEN_NAV_PANEL], &buttons[RENDER_MODE_BUTTON], NULL);
    SetButton(&buttons[RENDER_MODE_BUTTON], 10, 60, 60, 20, 6, RGBTo16(100, 100, 100), WHITE, BLACK, GREEN, &buttons[EXIT_EDITOR_UI], &buttons[OPEN_NAV_PANEL], NULL, NULL);

    SetButton(&buttons[OPEN_NAV_PANEL], 150, 0, 10, 20, 5, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, NULL, NULL, NULL, NULL);

    AddTextToButton(&buttons[SELECT_OBJECTS_UI], "Objects", WHITE, 1);
    AddTextToButton(&buttons[SELECT_LAYOUT_UI], "Layout", WHITE, 1);
    AddTextToButton(&buttons[SELECT_MODULE_UI], "Modules", WHITE, 1);
    AddTextToButton(&buttons[SELECT_OPTIONS_UI], "Options", WHITE, 1);

    AddTextToButton(&buttons[PLAY_SCENE_UI], "Play", WHITE, 1);
    AddTextToButton(&buttons[EXIT_EDITOR_UI], "Exit", WHITE, 1);

    UpdateRenderButton(false);

    buttons[PLAY_SCENE_UI].visible = false;
    buttons[EXIT_EDITOR_UI].visible = false;
    buttons[RENDER_MODE_BUTTON].visible = false;
    SetNavPanelVisibility(false);

    currentButton = &buttons[OPEN_NAV_PANEL];
    currentButton->isFocused = true;

    for (int i = 0; i < sizeof(panels) / sizeof(UIPanel); i++)
    {
        DrawPanel(&panels[i]);
    }

    for (int i = 0; i < sizeof(buttons) / sizeof(UIButton); i++)
    {
        DrawButton(&buttons[i]);
    }

    int currentMode = 0;
    bool refresh = true;
    bool refreshSceneView = false;

    int scenePosX = 0;
    int scenePosY = 0;

    uint8_t currentObject = 0;

    bool showKeyboard = false;
    char keyboardSelect = '\0';
    uint8_t caretPosition = 0;
    bool refreshKeyboard = false;

    char *modifyVariable = (char *)malloc(16);

    for (int i = 0; i < 16; i++)
    {
        modifyVariable[i] = '\0';
    }

    uint8_t variableBeingModified = 0;

    EngineVar **variableList = (EngineVar **)malloc(sizeof(EngineVar *) * MAX_SCRIPTS_PER_OBJECT * 10);
    uint8_t variableCount = 0;

    // Prototype functions
    bool EqualType(EngineVar * var1, EngineVar * var2, uint8_t type);
    float ShuntYard(char *equation, uint16_t equationLength, EngineVar *output, ScriptData *scriptData);
    bool EqualType(EngineVar * var1, EngineVar * var2, uint8_t type);

    while (1)
    {
        if (!showKeyboard)
            UpdateUIButtons();

        if (buttons[OPEN_NAV_PANEL].onPressDown)
        {
            for (int i = SELECT_LAYOUT_UI; i <= SELECT_OPTIONS_UI; i++)
            {
                buttons[i].isFocused = false;
            }
            RefocusButton(&buttons[SELECT_OBJECTS_UI], false);
            SetNavPanelVisibility(true);
        }

        // Objects mode
        if (buttons[SELECT_OBJECTS_UI].onPressDown || (refresh && currentMode == 0))
        {
            currentMode = 0;
            HideAllButtons();

            RefocusButton(&buttons[OPEN_NAV_PANEL], false);

            panels[OBJECT_PANEL].visible = true;

            for (int i = 0; i < currentScene->objectCount; i++)
            {
                UIButton *top = NULL;
                UIButton *bottom = NULL;

                if (i > 0)
                {
                    top = &buttons[OBJECT_BUTTONS + i - 1];
                }
                if (i < currentScene->objectCount - 1)
                {
                    bottom = &buttons[OBJECT_BUTTONS + i + 1];
                }
                SetButton(&buttons[OBJECT_BUTTONS + i], 10, 10 + i * 17, 140, 15, 6, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, top, &buttons[OPEN_NAV_PANEL], bottom, NULL);
                AddTextToButton(&buttons[OBJECT_BUTTONS + i], currentScene->objects[i].name, WHITE, 1);
            }

            if (currentScene->objectCount > 0)
            {
                buttons[OPEN_NAV_PANEL].leftNext = &buttons[OBJECT_BUTTONS];
            }

            SetNavPanelVisibility(false);
        }

        // Layout mode
        if (buttons[SELECT_LAYOUT_UI].onPressDown || (refresh && currentMode == 1))
        {
            currentMode = 1;
            HideAllButtons();

            RefocusButton(&buttons[OPEN_NAV_PANEL], false);

            panels[MAIN_PANEL].fillColor = RGBTo16(0, 0, 0);

            SetNavPanelVisibility(false);

            refreshSceneView = true;
        }

        // Module mode
        if (buttons[SELECT_MODULE_UI].onPressDown || (refresh && currentMode == 2))
        {
            currentMode = 2;
            HideAllButtons();

            RefocusButton(&buttons[OPEN_NAV_PANEL], false);

            panels[MAIN_PANEL].fillColor = RGBTo16(0, 0, 0);

            SetNavPanelVisibility(false);

            WriteWord(currentScene->objects[currentObject].name, strlen(currentScene->objects[currentObject].name), 3, 3, 1, WHITE, BLUE);

            int height = 13;
            uint8_t buttonIndex = MODULE_BUTTONS;
            variableCount = 0;
            printf("Script count: %d\n", currentScene->objects[currentObject].scriptCount);

            // Serialize var prototype
            char *SerializeVar(EngineVar * variable);

            for (int i = 0; i < currentScene->objects[currentObject].scriptCount; i++)
            {

                uint8_t scriptVariables = (currentScene->objects[currentObject].scriptData[i]->variableCount);

                SetPanel(&panels[MODULE_PANELS + i], 3, height, 154, 15 * (1 + scriptVariables), 6, RGBTo16(70, 70, 70), RGBTo16(120, 120, 120));
                DrawPanel(&panels[MODULE_PANELS + i]);

                WriteWord(
                    (currentScene->objects[currentObject].scriptData[i]->script->name),
                    strlen(currentScene->objects[currentObject].scriptData[i]->script->name),
                    6,
                    height + 3,
                    1,
                    WHITE,
                    ORANGE);

                for (int x = 0; x < scriptVariables; x++)
                {
                    printf("Var button: %d\n", x);

                    variableList[variableCount] = &(currentScene->objects[currentObject].scriptData[i]->data[x]);

                    WriteWord(
                        (variableList[variableCount]->name),
                        strlen(variableList[variableCount]->name),
                        6,
                        height + 15 * (x + 1) + 2,
                        1,
                        WHITE,
                        RGBTo16(0, 0, 80));

                    uint8_t buttonStart = 15 + (FONT_WIDTHS[0] + 1) * strlen(variableList[variableCount]->name);

                    SetButton(&buttons[buttonIndex], buttonStart, height + 15 * (x + 1), 145 - buttonStart, 12, 3, BLACK, RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, NULL, NULL, NULL, NULL);
                    buttons[buttonIndex].textAnchor = LEFT_ANCHOR;
                    AddTextToButton(&buttons[buttonIndex], SerializeVar(variableList[variableCount]), WHITE, 1);
                    DrawButton(&buttons[buttonIndex++]);

                    variableCount++;
                }
            }

            if (buttonIndex > MODULE_BUTTONS)
            {
                buttons[OPEN_NAV_PANEL].downNext = &buttons[MODULE_BUTTONS];
                buttons[MODULE_BUTTONS].upNext = &buttons[OPEN_NAV_PANEL];

                for (int i = MODULE_BUTTONS; i < buttonIndex; i++)
                {
                    if (i > MODULE_BUTTONS)
                    {
                        buttons[i].upNext = &buttons[i - 1];
                    }
                    if (i < buttonIndex - 1)
                    {
                        buttons[i].downNext = &buttons[i + 1];
                    }
                }
            }
        }

        // Settings mode
        if (buttons[SELECT_OPTIONS_UI].onPressDown || (refresh && currentMode == 3))
        {
            currentMode = 3;
            HideAllButtons();

            RefocusButton(&buttons[OPEN_NAV_PANEL], false);

            buttons[OPEN_NAV_PANEL].leftNext = &buttons[PLAY_SCENE_UI];
            buttons[PLAY_SCENE_UI].visible = true;
            buttons[EXIT_EDITOR_UI].visible = true;
            buttons[RENDER_MODE_BUTTON].visible = true;
            SetNavPanelVisibility(false);
        }

        if (buttons[EXIT_EDITOR_UI].onPressDown)
        {
            // Add save code
            return;
        }

        refresh = false;

        // Exit Nav Panel
        if (GetButton() == BUTTON_L && panels[NAV_PANEL].visible)
        {
            refresh = true;
        }

        // Manage Scene View
        if (currentMode == 1 && !panels[NAV_PANEL].visible)
        {
            uint8_t delay = 0;

            if (GetButton() == BUTTON_D)
            {
                scenePosX -= 1;
                refreshSceneView = true;
            }

            if (GetButton() == BUTTON_A)
            {
                scenePosX += 1;
                refreshSceneView = true;
            }

            if (GetButton() == BUTTON_W)
            {
                scenePosY += 1;
                refreshSceneView = true;
            }

            if (GetButton() == BUTTON_S)
            {
                scenePosY -= 1;
                refreshSceneView = true;
            }

            if (GetButton() == BUTTON_I)
            {
                sceneScale += 1;
                refreshSceneView = true;
                delay = 100;
            }

            if (GetButton() == BUTTON_K)
            {
                sceneScale -= 1;
                refreshSceneView = true;
                delay = 100;
            }

            if (refreshSceneView)
            {
                SmartClear();
                RenderScene(scenePosX, scenePosY);
                refreshSceneView = false;
            }
            sleep_ms(delay);
        }

        // Manage module mode
        if (currentMode == 2)
        {
            if (showKeyboard)
            {
                if (refreshKeyboard)
                {
                    Rectangle(RGBTo16(0, 0, 0), 0, 128 - 55, 160, 55);
                    refreshKeyboard = false;
                    keyboardSelect = PrintKeyboard(keyboardX, keyboardY, 5, 10);
                    strcpy(buttons[variableBeingModified + MODULE_BUTTONS].text, modifyVariable);
                    DrawButton(&buttons[variableBeingModified + MODULE_BUTTONS]);
                    sleep_ms(160);
                }

                if (GetButton() != 0)
                {

                    HandleKeyboardInputs();
                    if (GetButton() == BUTTON_J && caretPosition < MAX_NAME_LENGTH)
                    {
                        if (keyboardSelect == UPPERCASE_SYB)
                        {
                            uppercase = true;
                        }
                        else if (keyboardSelect == LOWERCASE_SYB)
                        {
                            uppercase = false;
                        }
                        else
                        {
                            if (keyboardSelect >= 65 && keyboardSelect <= 90 && !uppercase)
                            {
                                keyboardSelect += 32;
                            }
                            modifyVariable[caretPosition] = keyboardSelect;
                            caretPosition++;
                        }
                    }
                    if (GetButton() == BUTTON_K && caretPosition > 0)
                    {
                        caretPosition--;
                        modifyVariable[caretPosition] = '\0';
                    }
                    if (GetButton() == BUTTON_L)
                    {

                        if (variableList[variableBeingModified]->currentType == TYPE_STRING)
                        {
                            if (variableList[variableBeingModified]->data.s)
                            {
                                free(variableList[variableBeingModified]->data.s);
                            }
                            variableList[variableBeingModified]->data.s = malloc(16);
                            strcpy(variableList[variableBeingModified]->data.s, modifyVariable);
                            variableList[variableBeingModified]->data.s[15] = '\0';
                        }
                        else
                        {

                            EngineVar *output = malloc(sizeof(EngineVar));
                            ScriptData *emptyScripData = malloc(sizeof(ScriptData));
                            emptyScripData->variableCount = 0;
                            uint32_t error = ShuntYard(modifyVariable, strlen(modifyVariable), output, emptyScripData);

                            if (error == 0)
                            {
                                if (EqualType(variableList[variableBeingModified], output, TYPE_BOOL))
                                {
                                    variableList[variableBeingModified]->data.b = output->data.b;
                                }
                                if (EqualType(variableList[variableBeingModified], output, TYPE_VECTOR))
                                {
                                    variableList[variableBeingModified]->data.XY.x = output->data.XY.x;
                                    variableList[variableBeingModified]->data.XY.y = output->data.XY.y;
                                }
                                if (EqualType(variableList[variableBeingModified], output, TYPE_FLOAT))
                                {
                                    variableList[variableBeingModified]->data.f = output->data.f;
                                }
                                if (EqualType(variableList[variableBeingModified], output, TYPE_INT))
                                {
                                    variableList[variableBeingModified]->data.i = output->data.i;
                                }
                                if (EqualType(variableList[variableBeingModified], output, TYPE_STRING))
                                {
                                    char *temp = malloc(strlen(output->data.s) + 1);
                                    sprintf(temp, "%s", output->data.s);

                                    free(variableList[variableBeingModified]->data.s);
                                    free(output->data.s);

                                    variableList[variableBeingModified]->data.s = temp;
                                }
                                if (EqualType(variableList[variableBeingModified], output, TYPE_OBJ))
                                {
                                    variableList[variableBeingModified]->data.objID = output->data.objID;
                                }
                            }
                        }

                        showKeyboard = false;
                        refresh = true;
                    }
                    if (GetButton() == BUTTON_I)
                    {
                        modifyVariable[caretPosition] = ' ';
                        caretPosition++;
                    }

                    refreshKeyboard = true;
                }
            }
            else
            {
                for (int i = 0; i < variableCount; i++)
                {
                    if (buttons[i + MODULE_BUTTONS].isPressed)
                    {
                        buttons[i + MODULE_BUTTONS].isPressed = false;
                        showKeyboard = true;
                        refreshKeyboard = true;
                        variableBeingModified = i;
                    }
                }
            }
        }
    
        //Manage settings
        if(currentMode == 3 && buttons[RENDER_MODE_BUTTON].isPressed){
            renderMode++;
            if(renderMode == 3){
                renderMode = 0;
            }
            UpdateRenderButton(true);
            sleep_ms(120);
        }
    }
}

void EditScene(int sceneIndex)
{
    currentScene = &scenes[sceneIndex];

    currentScene->objects[currentScene->objectCount] = *ObjectConstructor(0, "Object1", strlen("Object1"));

    EngineScript *script = ScriptConstructor(0, "script1",
                                             "int x = 67; string myString = \"hi\"; bool b = true;");

    currentScene->objects[currentScene->objectCount].scriptData[0] = ScriptDataConstructor(script);
    currentScene->objects[currentScene->objectCount].scriptCount++;

    // Function prototypes
    uint32_t SetScriptData(EngineScript * script, ScriptData * output, uint8_t scopeLevel);
    char *UnpackErrorMessage(uint32_t error);
    //

    uint32_t errorNum = SetScriptData(script, currentScene->objects[currentScene->objectCount].scriptData[0], 0);

    char *error = UnpackErrorMessage(errorNum);

    printf("set script error: %s\n", error);
    free(error);

    currentScene->objectCount++;
    ManageSceneUI();
}

char *CreateNewScene()
{
    bool refresh = true;
    bool modifyName = false;
    uint8_t option = 0;
    char *name = (char *)malloc((MAX_NAME_LENGTH + 1) * sizeof(char));
    for (int i = 0; i <= MAX_NAME_LENGTH; i++)
    {
        name[i] = '\0';
    }
    char keyboardSelect = '\0';
    uint8_t animateKeyboard = 0;
    uint8_t caretPosition = 0;
    unsigned long blinkTime = millis();
    bool sceneAlreadyExists = false;
    bool drawCaret = true;

    uint8_t maxCharacterBlocks = 5;

    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("New Scene", strlen("New Scene"), 1, 1, 2, WHITE, TRANSPARENT);

            Rectangle(WHITE, 0, 18, 160, 1);

            WriteWord("Scene Name", strlen("Scene Name"), 2, 31, 1, WHITE, TRANSPARENT);

            Rectangle((option == 0) ? WHITE : RGBTo16(150, 150, 150), 2, 40, 156, 11);
            Rectangle((option == 0) ? RGBTo16(0, 0, 100) : BLACK, 3, 41, 154, 9);

            WriteWord(name, strlen(name), 4, 42, 1, WHITE, TRANSPARENT);

            if (sceneAlreadyExists)
            {
                WriteWord("Scene name exists", strlen("Scene name exists"), 2, 52, 1, RED, TRANSPARENT);
            }

            if (modifyName)
            {
                Rectangle(RGBTo16(0, 0, 60), 0, 128 - animateKeyboard, 160, animateKeyboard);
                keyboardSelect = PrintKeyboard(keyboardX, keyboardY, 5, 65 - animateKeyboard);
                if (animateKeyboard < 55)
                {
                    animateKeyboard += 10;

                    refresh = true;

                    sleep_ms(20);
                }
                else
                {
                    sleep_ms(160);
                }
            }
            else
            {
                animateKeyboard = 0;

                WriteWord(">Done", strlen(">Done"), (option == 1) ? 10 : 2, 75, 2, (option == 1) ? GREEN : RGBTo16(0, 100, 0), TRANSPARENT);

                WriteWord("W/S - Navigate", strlen("W/S - Navigate"), 1, 99, 1, RGBTo16(100, 100, 100), TRANSPARENT);
                WriteWord("J - Select/Edit", strlen("J - Select/Edit"), 1, 109, 1, RGBTo16(100, 100, 100), TRANSPARENT);
                WriteWord("L - Exit", strlen("L - Exit"), 1, 119, 1, RGBTo16(100, 100, 100), TRANSPARENT);
                while (GetButton() != 0)
                    sleep_ms(10);
            }
        }

        if (GetButton() != 0)
        {
            if (modifyName && animateKeyboard >= 55)
            {
                HandleKeyboardInputs();
                if (GetButton() == BUTTON_J && caretPosition < MAX_NAME_LENGTH)
                {
                    if (keyboardSelect == UPPERCASE_SYB)
                    {
                        uppercase = true;
                    }
                    else if (keyboardSelect == LOWERCASE_SYB)
                    {
                        uppercase = false;
                    }
                    else
                    {
                        if (keyboardSelect >= 65 && keyboardSelect <= 90 && !uppercase)
                        {
                            keyboardSelect += 32;
                        }
                        name[caretPosition] = keyboardSelect;
                        caretPosition++;
                    }
                }
                if (GetButton() == BUTTON_K && caretPosition > 0)
                {
                    caretPosition--;
                    name[caretPosition] = '\0';
                }
                if (GetButton() == BUTTON_L)
                {
                    modifyName = false;
                }
                if (GetButton() == BUTTON_I)
                {
                    name[caretPosition] = ' ';
                    caretPosition++;
                }
            }
            else
            {
                if (GetButton() == BUTTON_W)
                {
                    option = 0;
                }
                if (GetButton() == BUTTON_S)
                {
                    option = 1;
                }
                if (GetButton() == BUTTON_J)
                {
                    if (option == 1)
                    {
                        if (GetSceneByName(name) != -1)
                        {
                            sceneAlreadyExists = true;
                        }
                        else
                        {
                            return name;
                        }
                    }
                    else if (option == 0)
                    {
                        sceneAlreadyExists = false;
                        modifyName = true;
                    }
                }
                if (GetButton() == BUTTON_L)
                {
                    free(name);
                    return NULL;
                }
            }
            refresh = true;
        }

        if (modifyName)
        {
            if (millis() - blinkTime < 10)
            {
                if (drawCaret)
                {
                    WriteLetter('_', 4 + (strlen(name) * 6), 42, 1, WHITE, RGBTo16(0, 0, 100));
                }
                else
                {
                    Rectangle(RGBTo16(0, 0, 100), 4 + (strlen(name) * 6), 42, 5, 7);
                }
            }
            if (millis() - blinkTime > 250)
            {
                blinkTime = millis();
                drawCaret = !drawCaret;
            }
        }
    }
}

int SelectScene()
{
    bool refresh = true;

    uint8_t totalPages = (uint8_t)((float)sceneCount / 5) + 1; // floor and then add 1 basically ceil
    uint8_t page = 0;

    uint8_t currentScene = 0;

    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("Load", strlen("Load"), 1, 1, 2, WHITE, TRANSPARENT);
            Rectangle(WHITE, 0, 18, 160, 1);

            WriteWord("Scenes", strlen("Scenes"), 2, 23, 1, WHITE, TRANSPARENT);
            Rectangle(WHITE, 2, 32, 156, 63);
            Rectangle(RGBTo16(30, 30, 30), 3, 33, 154, 61);

            for (int i = page * 5; i < min((page + 1) * 5, sceneCount); i++)
            {
                uint8_t yPos = 36 + (i - page * 5) * 10;
                if (i == currentScene)
                {
                    Rectangle(GREEN, 4, yPos - 1, 152, 9);
                    WriteWord(scenes[i].name, strlen(scenes[i].name), 6, yPos, 1, BLACK, TRANSPARENT);
                }
                else
                {
                    WriteWord(scenes[i].name, strlen(scenes[i].name), 6, yPos, 1, WHITE, TRANSPARENT);
                }
            }

            char buffer[16];

            snprintf(buffer, sizeof(buffer), "%u / %u pages", page + 1, totalPages);

            WriteWord(buffer, 15, 4, 86, 1, RGBTo16(0, 0, 100), TRANSPARENT);

            WriteWord("W/S - Navigate", strlen("W/S - Navigate"), 1, 99, 1, RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("J - Select/Edit", strlen("J - Select/Edit"), 1, 109, 1, RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("L - Exit", strlen("L - Exit"), 1, 119, 1, RGBTo16(100, 100, 100), TRANSPARENT);

            while (GetButton() != 0)
                sleep_ms(10);
        }

        if (GetButton() != 0)
        {
            refresh = true;

            if (GetButton() == BUTTON_S && currentScene < min((page + 1) * 5, sceneCount) - 1)
            {
                currentScene++;
            }
            if (GetButton() == BUTTON_W && currentScene > page * 5)
            {
                currentScene--;
            }
            if (GetButton() == BUTTON_D && page < totalPages)
            {
                page++;
                currentScene = page * 5;
            }
            if (GetButton() == BUTTON_A && page > 0)
            {
                page--;
                currentScene = page * 5;
            }
            if (GetButton() == BUTTON_J)
            {
                return currentScene;
            }
            if (GetButton() == BUTTON_L)
            {
                return -1;
            }
        }
    }
}

void SceneMenu()
{
    bool refresh = true;
    uint8_t option = 0;
    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("Scene", strlen("Scene"), 1, 1, 2, WHITE, TRANSPARENT);
            Rectangle(WHITE, 0, 18, 160, 1);

            WriteWord("Create Scene", strlen("Create Scene"), (option == 0) ? 5 : 0, 40, 1, (option == 0) ? WHITE : RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("Open Scene", strlen("Open Scene"), (option == 1) ? 5 : 0, 50, 1, (option == 1) ? WHITE : RGBTo16(100, 100, 100), TRANSPARENT);

            WriteWord("W/S - Navigate", strlen("W/S - Navigate"), 1, 109, 1, RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("J - Select", strlen("J - Select"), 1, 119, 1, RGBTo16(100, 100, 100), TRANSPARENT);

            while (GetButton() != 0)
                sleep_ms(10);
        }

        if (GetButton() != 0)
        {
            if (GetButton() == BUTTON_W)
            {
                option = 0;
            }
            if (GetButton() == BUTTON_S)
            {
                option = 1;
            }
            if (GetButton() == BUTTON_J)
            {
                if (option == 0)
                {
                    char *sceneName = CreateNewScene();

                    if (sceneName != NULL)
                    {
                        uint8_t index = CreateScene(sceneName, strlen(sceneName));
                        EditScene(index);
                    }
                }
                else
                {
                    int index = SelectScene();
                    printf("sceneNumber: %d\n", index);
                    if (index != -1)
                    {
                        EditScene(index);
                    }
                }
            }
            if (GetButton() == BUTTON_L)
            {
                return;
            }
            refresh = true;
        }
    }
}

#endif