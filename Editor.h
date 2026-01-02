#ifndef EDITOR
#define EDITOR

#include <stdio.h>
#include "pico/stdlib.h"

#include "SaveSystem.h"
#include "TFT.h"

#include "Keyboard.h"

#include "InputManager.h"

#include "EngineStructs.h"
#include <string.h>

#include "pico/time.h"

#include "SceneView.h"

#include "Keybinds.h"

#define PALLETE_WIDTH 12
#define PALLETE_HEIGHT 15

#define PLAY_VIEW 1
#define DEBUG_VIEW 2
#define SCRIPT_VIEW 3
#define SPRITE_VIEW 4
#define SCENE_VIEW 5

uint8_t editorView = 0;
uint8_t debugLine = 0;

char currentScriptText[SCRIPT_LENGTH];

uint16_t colors[PALLETE_WIDTH];

float maxF(float a, float b)
{
    if (a < b)
        return b;
    return a;
}
float minF(float a, float b)
{
    if (a > b)
        return b;
    return a;
}

void SetupColors()
{
    colors[0] = RGBTo16(255, 0, 0);
    colors[1] = RGBTo16(255, 35, 0);
    colors[2] = RGBTo16(255, 55, 0);
    colors[3] = RGBTo16(255, 255, 0);
    colors[4] = RGBTo16(200, 255, 0);
    colors[5] = RGBTo16(0, 255, 0);
    colors[6] = RGBTo16(0, 255, 100);
    colors[7] = RGBTo16(0, 255, 255);
    colors[8] = RGBTo16(0, 0, 255);
    colors[9] = RGBTo16(160, 0, 255);
    colors[10] = RGBTo16(255, 0, 200);
    colors[11] = RGBTo16(255, 255, 255);
}

uint16_t ColorMultiply(uint16_t color, float value, float saturation)
{
    uint8_t r = (color & 0xf800) >> 11;
    uint8_t g = (color & 0x07e0) >> 5;
    uint8_t b = (color & 0x001f);

    r *= 8.2;
    g *= 7;
    b *= 8.2;

    r = min(255, r * value);
    g = min(255, g * value);
    b = min(255, b * value);

    uint8_t i = (r + g + b) / 3;

    r = r + (i - r) * (1 - saturation);
    g = g + (i - g) * (1 - saturation);
    b = b + (i - b) * (1 - saturation);

    return RGBTo16(r, g, b);
}

///////////////////////////////////////////////////////////////////////// INTERPRETER CALLS ///////////////////////////////////////////

bool UI_ClearDebug()
{
    if (editorView != DEBUG_VIEW)
        return false;
    Clear();
    debugLine = 0;
    return true;
}

bool UI_PrintToScreen(char *message, bool isError)
{
    if (editorView != DEBUG_VIEW)
        return false;

    uint16_t color = 0;
    uint16_t backgroundColor = 0;
    if (isError)
    {
        color = RGBTo16(255, 100, 100);
        backgroundColor = RGBTo16(60, 40, 0);
    }
    else
    {
        color = RGBTo16(255, 255, 255);
    }

    if (debugLine >= 10)
    {
        UI_ClearDebug();
    }

    debugLine += WriteWord(message, strlen(message), 5, 5 + debugLine * 10, 1, color, backgroundColor);

    return true;
}

//////////////////////////////////////////////////////////////////////// SPRITE EDITOR /////////////////////////////////////////////////////////////////
uint8_t SelectSprite()
{
    uint8_t option = 0;
    bool refresh = true;
    uint8_t pixelSize = 5;

    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("Load Sprite", strlen("Load Sprite"), 1, 1, 2, RGBTo16(255, 200, 0), TRANSPARENT);
            Rectangle(RGBTo16(255, 200, 0), 0, 18, 160, 1);

            for (int x = 0; x < SPRITE_WIDTH; x++)
            {
                for (int y = 0; y < SPRITE_HEIGHT; y++)
                {
                    Rectangle(sprites[option].sprite[x][y], (80 - SPRITE_WIDTH * pixelSize / 2) + x * (pixelSize), 25 + y * (pixelSize), pixelSize, pixelSize);
                }
            }

            WriteWord("A/D - Navigate", strlen("A/D - Navigate"), 1, 99, 1, RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("J - Select", strlen("J - Select"), 1, 109, 1, RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("L - Exit", strlen("L - Exit"), 1, 119, 1, RGBTo16(100, 100, 100), TRANSPARENT);

            sleep_ms(160);
        }

        if (GetButton() != 0)
        {
            refresh = true;

            if (GetButton() == BUTTON_D)
            {
                if (option < spriteCount - 1)
                {
                    option++;
                }
                else
                {
                    option = 0;
                }
            }
            if (GetButton() == BUTTON_A)
            {
                if (option > 0)
                {
                    option--;
                }
                else
                {
                    option = spriteCount - 1;
                }
            }

            if (GetButton() == BUTTON_J)
            {
                return option;
            }
            if (GetButton() == BUTTON_L)
            {
                return 255;
            }
        }
    }
}

void EditSprite(uint8_t spriteIndex)
{

    bool refresh = true;
    bool setColor = false;
    uint8_t cursorX = 0;
    uint8_t cursorY = 0;

    uint8_t pixelSize = 4;
    uint8_t spacing = 2;

    uint16_t currentColor = RGBTo16(255, 255, 255);

    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("Edit Sprite", strlen("Edit Sprite"), 1, 1, 2, RGBTo16(255, 200, 0), TRANSPARENT);
            Rectangle(RGBTo16(255, 200, 0), 0, 18, 160, 1);

            Rectangle(WHITE, 4 - spacing, 24 - spacing, (pixelSize + spacing) * SPRITE_WIDTH + spacing * 2, (pixelSize + spacing) * SPRITE_HEIGHT + spacing * 2);
            Rectangle(BLACK, 5 - spacing, 25 - spacing, (pixelSize + spacing) * SPRITE_WIDTH - 2 + spacing * 2, (pixelSize + spacing) * SPRITE_HEIGHT - 2 + spacing * 2);

            WriteWord("WASD - Move", strlen("WASD - Move"), 1, 99, 1, RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("J - Select", strlen("J - Select"), 1, 109, 1, RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("L - Save/Exit", strlen("L - Save/Exit"), 1, 119, 1, RGBTo16(100, 100, 100), TRANSPARENT);

            if (cursorX < SPRITE_WIDTH)
            {
                Rectangle(WHITE, 4 + cursorX * (pixelSize + spacing), 24 + cursorY * (pixelSize + spacing), pixelSize + 2, pixelSize + 2);
            }
            else
            {
                if (cursorX == SPRITE_WIDTH + PALLETE_WIDTH - 1)
                {
                    Rectangle(CYAN, (156 - PALLETE_WIDTH * 6) + (cursorX - SPRITE_WIDTH) * 6, 24 + cursorY * 6, 7, 7);
                }
                else
                {
                    Rectangle(WHITE, (156 - PALLETE_WIDTH * 6) + (cursorX - SPRITE_WIDTH) * 6, 24 + cursorY * 6, 7, 7);
                }
            }

            for (int x = 0; x < SPRITE_WIDTH; x++)
            {
                for (int y = 0; y < SPRITE_HEIGHT; y++)
                {
                    if (sprites[spriteIndex].sprite[x][y] == TRANSPARENT)
                    {
                        Rectangle(WHITE, 5 + x * (pixelSize + spacing), 25 + y * (pixelSize + spacing), 2, 2);
                        Rectangle(WHITE, 5 + x * (pixelSize + spacing) + 2, 25 + y * (pixelSize + spacing) + 2, 2, 2);
                        Rectangle(RGBTo16(100, 100, 100), 5 + x * (pixelSize + spacing) + 2, 25 + y * (pixelSize + spacing), 2, 2);
                        Rectangle(RGBTo16(100, 100, 100), 5 + x * (pixelSize + spacing), 25 + y * (pixelSize + spacing) + 2, 2, 2);
                    }
                    else
                        Rectangle(sprites[spriteIndex].sprite[x][y], 5 + x * (pixelSize + spacing), 25 + y * (pixelSize + spacing), pixelSize, pixelSize);
                }
            }

            for (int y = 0; y < PALLETE_HEIGHT; y++)
            {
                for (int x = 0; x < PALLETE_WIDTH; x++)
                {

                    float brightness = minF(1, (float)y / (((float)PALLETE_HEIGHT) / 2));

                    float sat = 1 - (maxF(1, (float)y / (((float)PALLETE_HEIGHT) / 2)) - 1);
                    uint16_t color = ColorMultiply(colors[x], brightness, sat);
                    if (setColor && x == (cursorX - SPRITE_WIDTH) && y == cursorY)
                    {
                        setColor = false;
                        currentColor = color;
                        refresh = true;
                    }

                    if (currentColor == color)
                    {
                        if (x == PALLETE_WIDTH - 1)
                        {
                            Rectangle(CYAN, (156 - PALLETE_WIDTH * 6) + x * 6, 24 + y * 6, 7, 7);
                        }
                        else
                        {
                            Rectangle(WHITE, (156 - PALLETE_WIDTH * 6) + x * 6, 24 + y * 6, 7, 7);
                        }
                    }

                    Rectangle(color, (157 - PALLETE_WIDTH * 6) + x * 6, 25 + y * 6, 5, 5);
                }
            }

            setColor = false;

            sleep_ms(120);
        }

        if (GetButton() != 0)
        {
            refresh = true;

            if (GetButton() == BUTTON_A && cursorX > 0)
            {
                cursorX--;
            }
            if (GetButton() == BUTTON_D && cursorX < SPRITE_WIDTH + PALLETE_WIDTH - 1)
            {
                cursorX++;
            }
            uint8_t maxY = (cursorX < SPRITE_WIDTH) ? SPRITE_HEIGHT - 1 : PALLETE_HEIGHT - 1;
            if (GetButton() == BUTTON_W && cursorY > 0)
            {
                cursorY--;
            }
            if (GetButton() == BUTTON_S && cursorY < maxY)
            {
                cursorY++;
            }

            if (cursorY > maxY)
            {
                cursorY = maxY;
            }

            if (gpio_get(BUTTON_J) == 0)
            {
                setColor = true;
                if (cursorX < SPRITE_WIDTH)
                {
                    sprites[spriteIndex].sprite[cursorX][cursorY] = currentColor;
                    setColor = false;
                }
            }

            if (GetButton() == BUTTON_L)
            {
                print("Save Project");
                SaveProject(program);
                return;
            }
        }
    }
}
void SpriteMode()
{
    bool refresh = true;
    uint8_t option = 0;
    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("Sprite", strlen("Sprite"), 1, 1, 2, RGBTo16(255, 200, 0), TRANSPARENT);
            Rectangle(RGBTo16(255, 200, 0), 0, 18, 160, 1);

            WriteWord("Create Sprite", strlen("Create Sprite"), (option == 0) ? 5 : 0, 40, 1, (option == 0) ? WHITE : RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("Open Sprite", strlen("Open Sprite"), (option == 1) ? 5 : 0, 50, 1, (option == 1) ? WHITE : RGBTo16(100, 100, 100), TRANSPARENT);

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
                    uint8_t spriteIndex = CreateSprite();
                    for (int x = 0; x < SPRITE_WIDTH; x++)
                    {
                        for (int y = 0; y < SPRITE_HEIGHT; y++)
                        {
                            sprites[spriteIndex].sprite[x][y] = TRANSPARENT;
                        }
                    }
                    EditSprite(spriteIndex);
                }
                else
                {
                    uint8_t spriteIndex = SelectSprite();
                    if (spriteIndex != 255)
                        EditSprite(spriteIndex);
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

//////////////////////////////////////////////////////////////////////// SCRIPT EDITOR /////////////////////////////////////////////////////////////////

// shortcuts
#pragma region

#define SHORTCUT_CONDITIONAL 0
#define SHORTCUT_TYPES 1
#define SHORTCUT_MATH 2
#define SHORTCUT_OBJECT 3
#define SHORTCUT_COLLIDER 4
#define SHORTCUT_PHYSICS 5
#define SHORTCUT_IO 6
#define SHORTCUT_SCREEN 7

#define SHORTCUT_CATEGORY_COUNT 8
char *SC_CATEGORY_NAMES[] = {
    "Conditions",
    "Types",
    "Math",
    "Object",
    "Collider",
    "Physics",
    "IO",
    "Display"};

#define SC_CONDITION_COUNT 4
char *SC_CONDITION_SHORTCUTS[] = {
    "if () {",
    "} else {",
    "while () {",
    "break"};

#define SC_TYPES_COUNT 6
char *SC_TYPE_SHORTCUTS[] = {
    "void",
    "bool",
    "int",
    "float",
    "string",
    "Vector(,)"};

#define SC_MATH_COUNT 5
char *SC_MATH_SHORTCUTS[] = {
    "PI",
    "pow",
    "sin",
    "cos",
    "deltaTime"};

#define SC_OBJECT_COUNT 10
char *SC_OBJECT_SHORTCUTS[] = {
    "setPosition()",
    "setScale()",
    "setSprite()",
    "setVelocity()",

    "addPosition()",
    "addScale()",
    "addVelocity()",

    "getPosition()",
    "getScale()",
    "getSprite()"};

// collider here

// physic here

#define SC_IO_COUNT 3
char *SC_IO_SHORTCUTS[] = {
    "input(\"\")",

    "leftLED()",
    "rightLED()"};

#pragma endregion

int shortcutCategory = -1;
uint8_t shortcutPage = 0;

void PrintListOnKeyboard(char **list, uint8_t count, uint8_t currentPage, uint8_t selected)
{
    for (int i = 5 * currentPage; i < min(count, 5 * currentPage + 5); i++)
    {
        if (selected == i)
        {
            Rectangle(YELLOW, 4, 69 + (i - 5 * currentPage) * 10, strlen(list[i]) * (FONT_WIDTHS[0] + 1) + 2, 9);
            WriteWord(list[i], strlen(list[i]), 5, 70 + (i - 5 * currentPage) * 10, 1, BLACK, TRANSPARENT);
        }
        else
        {
            WriteWord(list[i], strlen(list[i]), 5, 70 + (i - 5 * currentPage) * 10, 1, YELLOW, TRANSPARENT);
        }
    }
}

char *HandleShortcuts()
{
    bool refresh = true;
    uint8_t selected = 0;
    char *output = malloc(32);
    shortcutPage = 0;
    while (1)
    {
        uint8_t count = 0;

        if (refresh)
        {
            refresh = false;
            Rectangle(RGBTo16(0, 0, 60), 0, 68, 160, 60);
            switch (shortcutCategory)
            {
            case -1:
                PrintListOnKeyboard(SC_CATEGORY_NAMES, SHORTCUT_CATEGORY_COUNT, shortcutPage, selected);
                count = SHORTCUT_CATEGORY_COUNT;
                break;

            case SHORTCUT_CONDITIONAL:
                PrintListOnKeyboard(SC_CONDITION_SHORTCUTS, SC_CONDITION_COUNT, shortcutPage, selected);
                count = SC_CONDITION_COUNT;
                strcpy(output, SC_CONDITION_SHORTCUTS[selected]);
                break;

            case SHORTCUT_TYPES:
                PrintListOnKeyboard(SC_TYPE_SHORTCUTS, SC_TYPES_COUNT, shortcutPage, selected);
                count = SC_TYPES_COUNT;
                strcpy(output, SC_TYPE_SHORTCUTS[selected]);
                break;

            case SHORTCUT_MATH:
                PrintListOnKeyboard(SC_MATH_SHORTCUTS, SC_MATH_COUNT, shortcutPage, selected);
                count = SC_MATH_COUNT;
                strcpy(output, SC_MATH_SHORTCUTS[selected]);
                break;

            case SHORTCUT_OBJECT:
                PrintListOnKeyboard(SC_OBJECT_SHORTCUTS, SC_OBJECT_COUNT, shortcutPage, selected);
                count = SC_OBJECT_COUNT;
                strcpy(output, SC_OBJECT_SHORTCUTS[selected]);
                break;

            case SHORTCUT_IO:
                PrintListOnKeyboard(SC_IO_SHORTCUTS, SC_IO_COUNT, shortcutPage, selected);
                count = SC_IO_COUNT;
                strcpy(output, SC_IO_SHORTCUTS[selected]);
                break;
            }
            sleep_ms(100);
        }

        int buttonPress = GetButton();
        if (buttonPress != 0)
        {
            refresh = true;
            if (buttonPress == BUTTON_D && shortcutPage < ceil(count / 5))
            {
                shortcutPage++;
                selected = shortcutPage * 5;
            }
            if (buttonPress == BUTTON_A && shortcutPage > 0)
            {
                shortcutPage--;
                selected = shortcutPage * 5;
            }
            if (buttonPress == BUTTON_W && selected > shortcutPage * 5)
            {
                print("w");
                selected--;
            }
            if (buttonPress == BUTTON_S && selected < min((shortcutPage + 1) * 5, count) - 1)
            {
                print("s");
                selected++;
            }

            if (shortcutCategory == -1)
            {
                if (buttonPress == BUTTON_J)
                {
                    shortcutCategory = selected;
                    selected = 0;
                    shortcutPage = 0;
                }
                if (buttonPress == BUTTON_L)
                {
                    free(output);
                    return NULL;
                }
            }
            else
            {
                if (buttonPress == BUTTON_J)
                {
                    return output;
                }
                if (buttonPress == BUTTON_L)
                {
                    shortcutCategory = -1;
                    selected = 0;
                    shortcutPage = 0;
                }
            }
        }
    }
}
void RecalculateCaret(int *caretX, int *caretY, int *caretPos)
{
    (*caretX) = 0;
    (*caretY) = 0;
    for (int i = 0; i < (*caretPos); i++)
    {
        (*caretX)++;
        if (currentScriptText[i] == '\n')
        {
            (*caretX) = 0;
            (*caretY)++;
        }
    }
}
void EditScript(uint8_t scriptIndex)
{
    bool refresh = true;
    bool writing = false;
    char currentCharacter = '\0';
    uint8_t animateKeyboard = 0;

    int caretPosition = 0;

    int caretX = 0;
    int caretY = 0;

    uint16_t line = 0;
    unsigned long blinkTime = millis();
    bool drawCaret = true;
    int inputPause = 0;

    memset(currentScriptText, 0, sizeof(currentScriptText));

    printf("Original content: %s", scripts[scriptIndex].content);

    strcpy(currentScriptText, scripts[scriptIndex].content);

    printf("Loaded content: %s\n", currentScriptText);

    keyboard[0] = HAMBURGER_SYB;

    bool isSelecting = false;

    int startSelectCaret = 0;
    int startSelectX = 0;
    int startSelectY = 0;

    char copyBuffer[SCRIPT_LENGTH / 2 + 2];

    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();

            int contentStartIndex = 0;

            char content[SCRIPT_LENGTH + 1];

            uint16_t searchLine = 0;
            for (int i = 0; i < SCRIPT_LENGTH; i++)
            {

                if (i < caretPosition)
                {
                    content[i] = currentScriptText[i];
                    if (currentScriptText[i] == '\n')
                    {
                        searchLine++;
                        if (searchLine == line)
                        {
                            contentStartIndex = i + 1;
                        }
                    }
                }
                else if (i == caretPosition)
                {
                    content[i] = ' ';
                }
                else
                {
                    content[i] = currentScriptText[i - 1];
                    if (currentScriptText[i - 1] == '\n')
                    {
                        searchLine++;
                        if (searchLine == line)
                        {
                            contentStartIndex = i;
                        }
                    }
                }
            }
            content[SCRIPT_LENGTH] = '\0';

            if (isSelecting && caretPosition != startSelectCaret)
            {
                uint16_t selectColor = RGBTo16(0, 80, 150);
                if (caretY == startSelectY)
                {
                    if (startSelectCaret < caretPosition)
                    {
                        Rectangle(selectColor, startSelectX * 6, caretY * 8, abs(caretX * 6 - startSelectX * 6), 8);
                    }
                    else
                    {
                        Rectangle(selectColor, (caretX + 1) * 6, caretY * 8, abs(caretX - startSelectX) * 6, 8);
                    }
                }
                else
                {
                    if (startSelectCaret < caretPosition)
                    {
                        Rectangle(selectColor, startSelectX * 6, startSelectY * 8, 160 - startSelectX * 6, 8);
                        for (int y = startSelectY + 1; y < caretY; y++)
                        {
                            Rectangle(selectColor, 0, y * 8, 160, 8);
                        }
                        Rectangle(selectColor, 0, caretY * 8, caretX * 6, 8);
                    }
                    else
                    {
                        Rectangle(selectColor, caretX * 6, caretY * 8, 160 - caretX * 6, 8);
                        for (int y = caretY + 1; y < startSelectY; y++)
                        {
                            Rectangle(selectColor, 0, y * 8, 160, 8);
                        }
                        Rectangle(selectColor, 0, startSelectY * 8, startSelectX * 6, 8);
                    }
                }
            }

            WriteWord(content + contentStartIndex, min((SCRIPT_LENGTH + 1 - contentStartIndex), 416), 0, 0, 1, WHITE, TRANSPARENT);

            if (writing)
            {
                Rectangle(RGBTo16(0, 0, 60), 0, 128 - animateKeyboard, 160, animateKeyboard);
                currentCharacter = PrintKeyboard(keyboardX, keyboardY, 5, 65 - animateKeyboard);
                PrintKeybinds(animateKeyboard, KB_SCRIPT_EDIT, RGBTo16(100, 100, 100));
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
                if (isSelecting)
                {
                    PrintKeybinds(0, KB_SCRIPT_SELECTED, RGBTo16(100, 100, 100));
                }
                else
                {
                    PrintKeybinds(0, KB_SCRIPT_VIEW, RGBTo16(100, 100, 100));
                }
                sleep_ms(100);
            }
        }
        if (GetButton() != 0)
        {
            refresh = true;
            if (!writing)
            {
                if (GetButton() == BUTTON_D && caretPosition < SCRIPT_LENGTH - 1)
                {

                    if (caretPosition >= strlen(currentScriptText))
                    {
                        for (int i = strlen(currentScriptText) + 1; i > caretPosition; i--)
                        {
                            currentScriptText[i] = currentScriptText[i - 1];
                        }
                        currentScriptText[caretPosition] = '\n';
                        caretPosition++;
                        caretX = 0;
                        caretY++;
                    }
                    else
                    {
                        if (currentScriptText[caretPosition] == '\n')
                        {
                            caretPosition++;
                            caretX = 0;
                            caretY++;
                        }
                        else
                        {
                            caretPosition++;
                            caretX++;
                        }
                    }
                }
                if (GetButton() == BUTTON_A)
                {
                    if (caretPosition > 0)
                    {
                        caretPosition--;
                        caretX--;
                        if (currentScriptText[caretPosition] == '\n')
                        {

                            int lineLength = 0;

                            caretPosition--;
                            printf("char: %d\n", currentScriptText[caretPosition]);
                            while (currentScriptText[caretPosition] != '\n' && caretPosition > 0)
                            {
                                caretPosition--;
                                printf("char: %d\n", currentScriptText[caretPosition]);
                                lineLength++;
                            }

                            caretX = lineLength;
                            if (caretPosition == 0)
                            {
                                caretX++;
                            }

                            caretPosition += lineLength + 1;

                            caretY--;

                            printf("(%d,%d)\n", caretX, caretY);
                        }
                    }
                }
                if (GetButton() == BUTTON_S && caretPosition < SCRIPT_LENGTH - 24)
                {
                    int originalCaret = caretPosition;
                    while (currentScriptText[caretPosition] != '\n' && caretPosition < strlen(currentScriptText))
                    {
                        caretPosition++;
                    }
                    if (caretPosition >= strlen(currentScriptText))
                    {
                        for (int i = strlen(currentScriptText) + 1; i > caretPosition; i--)
                        {
                            currentScriptText[i] = currentScriptText[i - 1];
                        }
                        currentScriptText[caretPosition] = '\n';
                        caretPosition++;
                        caretX = 0;
                        caretY++;
                    }
                    else
                    {
                        originalCaret = ++caretPosition;
                        while (currentScriptText[caretPosition] != '\n' && caretPosition < strlen(currentScriptText))
                        {
                            caretPosition++;
                        }
                        int length = (caretPosition - originalCaret);

                        caretPosition = originalCaret + min(caretX, length);
                        caretX = min(caretX, length);

                        caretY++;
                    }
                }
                if (GetButton() == BUTTON_W)
                {
                    int originalCaretPos = caretPosition;
                    caretPosition--;
                    printf("char: %d\n", currentScriptText[caretPosition]);
                    while (currentScriptText[caretPosition] != '\n' && caretPosition > 0)
                    {
                        caretPosition--;
                        printf("char: %d\n", currentScriptText[caretPosition]);
                    }
                    if (caretPosition == 0)
                    {
                        caretPosition = originalCaretPos;
                    }
                    else
                    {

                        int lineLength = 0;

                        caretPosition--;
                        printf("char: %d\n", currentScriptText[caretPosition]);
                        while (currentScriptText[caretPosition] != '\n' && caretPosition > 0)
                        {
                            caretPosition--;
                            printf("char: %d\n", currentScriptText[caretPosition]);
                            lineLength++;
                        }

                        if (caretPosition > 0)
                            caretPosition++;

                        caretPosition += min(caretX, lineLength);
                        caretX = min(caretX, lineLength);
                        caretY--;
                    }
                }
                if (GetButton() == BUTTON_J)
                {
                    writing = true;
                    animateKeyboard = 0;
                }
                if (GetButton() == BUTTON_L)
                {
                    if (isSelecting)
                    {
                        isSelecting = false;
                    }
                    else
                    {
                        print("saving file");

                        FlushBuffer();

                        char *saveName = malloc(MAX_NAME_LENGTH + 1 + strlen(program->name) + strlen("`ENGINESCRIPT") + 1);

                        sprintf(saveName, "%s`%s`ENGINESCRIPT", scripts[scriptIndex].name, program->name);

                        File *file = GetFile(saveName);

                        if (file->startBlock == 0)
                        {
                            file = CreateFile(saveName, strlen(saveName), 10);
                        }
                        // currentScriptText[caretPosition] = '\0';
                        WriteFile(file, currentScriptText, SCRIPT_LENGTH - 1);

                        if (scripts[scriptIndex].content != NULL)
                        {
                            free(scripts[scriptIndex].content);
                        }
                        scripts[scriptIndex].content = malloc(strlen(currentScriptText) + 1);
                        strcpy(scripts[scriptIndex].content, currentScriptText);

                        free(saveName);
                        DisengageSD();

                        print("Save complete");

                        SaveProject(program);
                        keyboard[0] = '@';
                        return;
                    }
                }
                if (GetButton() == BUTTON_I)
                {
                    if (isSelecting)
                    {

                        memset(copyBuffer, 0, sizeof(copyBuffer));

                        if (startSelectCaret != caretPosition)
                        {
                            int start = min(startSelectCaret, caretPosition);
                            int end = max(startSelectCaret, caretPosition);

                            strncpy(copyBuffer, currentScriptText + start, end - start);
                            copyBuffer[end - start + 1] = '\0';
                        }
                    }
                    else
                    {
                        startSelectCaret = caretPosition;
                        startSelectX = caretX;
                        startSelectY = caretY;
                        isSelecting = true;
                    }
                }
                if (GetButton() == BUTTON_K)
                {
                    if (isSelecting)
                    {
                        int start = min(startSelectCaret, caretPosition);
                        int end = max(startSelectCaret, caretPosition);
                        int length = end - start;
                        caretX = min(startSelectX, caretX);
                        caretY = min(startSelectY, caretY);
                        caretPosition = start;

                        for (int i = caretPosition; i <= strlen(currentScriptText) - length; i++)
                        {
                            currentScriptText[i] = currentScriptText[i + length];
                        }
                        currentScriptText[strlen(currentScriptText) - length] = '\0';
                        isSelecting = false;
                    }
                    else if (strlen(copyBuffer) > 0)
                    {
                        for (int i = strlen(currentScriptText) + strlen(copyBuffer); i > caretPosition; i--)
                        {
                            currentScriptText[i] = currentScriptText[i - strlen(copyBuffer)];
                        }
                        strncpy(&currentScriptText[caretPosition], copyBuffer, strlen(copyBuffer));
                        caretPosition += strlen(copyBuffer);
                        RecalculateCaret(&caretX, &caretY, &caretPosition);
                    }
                }
            }
            else
            {
                HandleKeyboardInputs();
                if (GetButton() == BUTTON_J && caretPosition < SCRIPT_LENGTH && animateKeyboard > 55)
                {
                    if (currentCharacter == HAMBURGER_SYB)
                    {
                        char *shortcut = HandleShortcuts();
                        if (shortcut != NULL)
                        {
                            for (int i = strlen(currentScriptText) + strlen(shortcut); i > caretPosition; i--)
                            {
                                currentScriptText[i] = currentScriptText[i - strlen(shortcut)];
                            }
                            strncpy(&currentScriptText[caretPosition], shortcut, strlen(shortcut));
                            caretPosition += strlen(shortcut);
                            caretX += strlen(shortcut);
                            free(shortcut);
                        }
                        continue;
                    }
                    else if (currentCharacter == UPPERCASE_SYB)
                    {
                        uppercase = true;
                    }
                    else if (currentCharacter == LOWERCASE_SYB)
                    {
                        uppercase = false;
                    }
                    else
                    {
                        if (currentCharacter >= 65 && currentCharacter <= 90 && !uppercase)
                        {
                            currentCharacter += 32;
                        }
                        for (int i = strlen(currentScriptText) + 1; i > caretPosition; i--)
                        {
                            currentScriptText[i] = currentScriptText[i - 1];
                        }
                        currentScriptText[caretPosition] = currentCharacter;
                        caretPosition++;
                        caretX++;
                    }
                }
                if (GetButton() == BUTTON_L)
                {
                    writing = false;
                    sleep_ms(200);
                }
                if (GetButton() == BUTTON_K && caretPosition > 0)
                {
                    caretX--;
                    caretPosition--;

                    for (int i = caretPosition; i < strlen(currentScriptText); i++)
                    {
                        currentScriptText[i] = currentScriptText[i + 1];
                    }
                }

                if (GetButton() == BUTTON_I)
                {
                    for (int i = strlen(currentScriptText) + 1; i > caretPosition; i--)
                    {
                        currentScriptText[i] = currentScriptText[i - 1];
                    }
                    currentScriptText[caretPosition] = '\n';
                    caretPosition++;
                    caretX = 0;
                    caretY++;
                }
            }
        }

        if (caretX < 0)
        {
            caretY--;
            int length = 1;
            while (currentScriptText[caretPosition - length] != '\n')
            {
                if (caretPosition - length <= 0)
                {
                    break;
                }
                length++;
            }
            caretX = length;
        }
        if (caretX >= 24)
        {
            caretY += floor(caretX / 24);
            caretX = 0;
        }

        if (millis() - blinkTime < 10)
        {
            if (drawCaret)
            {
                Rectangle(WHITE, caretX * 6, (caretY + 1) * 8 - 2, 5, 1);
            }
            else
            {
                Rectangle(BLACK, caretX * 6, (caretY + 1) * 8 - 2, 5, 1);
            }
        }
        if (millis() - blinkTime > 250)
        {
            blinkTime = millis();
            drawCaret = !drawCaret;
        }
    }
}
int SelectScript()
{
    bool refresh = true;

    uint8_t totalPages = (uint8_t)((float)scriptCount / 5) + 1; // floor and then add 1 basically ceil
    uint8_t page = 0;

    uint8_t currentScript = 0;

    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("Load", strlen("Load"), 1, 1, 2, RGBTo16(0, 200, 0), TRANSPARENT);
            Rectangle(RGBTo16(0, 200, 0), 0, 18, 160, 1);

            WriteWord("Scripts", strlen("Scripts"), 2, 23, 1, GREEN, TRANSPARENT);
            Rectangle(WHITE, 2, 32, 156, 63);
            Rectangle(RGBTo16(30, 30, 30), 3, 33, 154, 61);

            for (int i = page * 5; i < min((page + 1) * 5, scriptCount); i++)
            {
                uint8_t yPos = 36 + (i - page * 5) * 10;
                if (i == currentScript)
                {
                    Rectangle(GREEN, 4, yPos - 1, 152, 9);
                    WriteWord(scripts[i].name, strlen(scripts[i].name), 6, yPos, 1, BLACK, TRANSPARENT);
                }
                else
                {
                    WriteWord(scripts[i].name, strlen(scripts[i].name), 6, yPos, 1, WHITE, TRANSPARENT);
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

            if (GetButton() == BUTTON_S && currentScript < min((page + 1) * 5, scriptCount) - 1)
            {
                currentScript++;
            }
            if (GetButton() == BUTTON_W && currentScript > page * 5)
            {
                currentScript--;
            }
            if (GetButton() == BUTTON_D && page < totalPages)
            {
                page++;
                currentScript = page * 5;
            }
            if (GetButton() == BUTTON_A && page > 0)
            {
                page--;
                currentScript = page * 5;
            }
            if (GetButton() == BUTTON_J)
            {
                return currentScript;
            }
            if (GetButton() == BUTTON_L)
            {
                return -1;
            }
        }
    }
}
charArray *CreateScriptMenu()
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
    bool scriptAlreadyExists = false;
    bool drawCaret = true;

    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("New Script", strlen("New Script"), 1, 1, 2, RGBTo16(0, 200, 0), TRANSPARENT);

            Rectangle(RGBTo16(0, 200, 0), 0, 18, 160, 1);

            WriteWord("Script Name", strlen("Script Name"), 2, 31, 1, WHITE, TRANSPARENT);

            Rectangle((option == 0) ? WHITE : RGBTo16(150, 150, 150), 2, 40, 156, 11);
            Rectangle((option == 0) ? RGBTo16(0, 0, 100) : BLACK, 3, 41, 154, 9);

            WriteWord(name, strlen(name), 4, 42, 1, WHITE, TRANSPARENT);

            if (scriptAlreadyExists)
            {
                WriteWord("Script name exists", strlen("Script name exists"), 2, 52, 1, RED, TRANSPARENT);
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
                        if (GetScriptByName(name) != -1)
                        {
                            scriptAlreadyExists = true;
                        }
                        else
                        {
                            charArray *array = malloc(sizeof(charArray));
                            array->array = name;
                            array->length = caretPosition;
                            return array;
                        }
                    }
                    else if (option == 0)
                    {
                        scriptAlreadyExists = false;
                        modifyName = true;
                    }
                }
                if (GetButton() == BUTTON_L)
                {
                    free(name);
                    charArray *array = malloc(sizeof(charArray));
                    array->array = NULL;
                    array->length = 0;
                    return array;
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

void ScriptMenu()
{
    bool refresh = true;
    uint8_t option = 0;
    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("Script", strlen("Script"), 1, 1, 2, RGBTo16(0, 200, 0), TRANSPARENT);
            Rectangle(RGBTo16(0, 200, 0), 0, 18, 160, 1);

            WriteWord("Create Script", strlen("Create Script"), (option == 0) ? 5 : 0, 40, 1, (option == 0) ? WHITE : RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("Open Script", strlen("Open Script"), (option == 1) ? 5 : 0, 50, 1, (option == 1) ? WHITE : RGBTo16(100, 100, 100), TRANSPARENT);

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
                    charArray *scriptName = CreateScriptMenu();

                    if (scriptName->length != 0)
                    {
                        uint8_t index = CreateScript(scriptName->array, scriptName->length);
                        free(scriptName->array);
                        free(scriptName);
                        EditScript(index);
                    }
                }
                else
                {
                    int index = SelectScript();
                    printf("scriptNumber: %d\n", index);
                    if (index != -1)
                    {
                        EditScript(index);
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

///////////////////////////////////////////////////////////////////////// MAIN SCREEN //////////////////////////////////////////////////////////////////

void EditorMainScreen()
{
    LoadProject(program);
    uint8_t option = 0;
    bool refresh = true;
    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("Edit Menu", strlen("Edit Menu"), 1, 1, 2, RGBTo16(0, 200, 0), TRANSPARENT);
            Rectangle(RGBTo16(0, 200, 0), 0, 18, 160, 1);

            WriteWord("Scenes", strlen("Scenes"), (option == 0) ? 7 : 1, 7 + (FONT_HEIGHTS[1] + 1) * 1, 2, (option == 0) ? WHITE : RGBTo16(90, 90, 90), TRANSPARENT);
            WriteWord("Sprites", strlen("Sprites"), (option == 1) ? 7 : 1, 7 + (FONT_HEIGHTS[1] + 1) * 2, 2, (option == 1) ? RGBTo16(255, 200, 0) : RGBTo16(90, 90, 90), TRANSPARENT);
            WriteWord("Scripts", strlen("Scripts"), (option == 2) ? 7 : 1, 7 + (FONT_HEIGHTS[1] + 1) * 3, 2, (option == 2) ? RGBTo16(0, 255, 0) : RGBTo16(90, 90, 90), TRANSPARENT);

            WriteWord("W/S - Navigate", strlen("W/S - Navigate"), 1, 99, 1, RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("J - Select", strlen("J - Select"), 1, 109, 1, RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("L - Exit", strlen("L - Exit"), 1, 119, 1, RGBTo16(100, 100, 100), TRANSPARENT);

            while (GetButton() != 0)
                sleep_ms(10);
        }
        int button = GetButton();
        if (button != 0)
        {
            refresh = true;
            if (button == BUTTON_W && option > 0)
            {
                option--;
            }
            if (button == BUTTON_S && option < 2)
            {
                option++;
            }

            if (button == BUTTON_J)
            {
                switch (option)
                {
                case 0:
                    editorView = SCENE_VIEW;
                    SceneMenu();
                    break;
                case 1:
                    editorView = SPRITE_VIEW;
                    SpriteMode();
                    break;
                case 2:
                    editorView = SCRIPT_VIEW;
                    ScriptMenu();
                    break;
                }
            }

            if (button == BUTTON_L)
            {
                SaveProject(program);
                ClearProject();
                return;
            }
            editorView = 0;
        }
    }
}

#endif