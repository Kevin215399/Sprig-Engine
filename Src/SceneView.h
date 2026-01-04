#ifndef SCENE_VIEWER
#define SCENE_VIEWER

#include <stdio.h>
#include "pico/stdlib.h"

#include "SaveSystem.h"
#include "TFT.h"

#include "Keyboard.h"

#include "InputManager.h"

#include "EngineStructs.h"
#include <string.h>

#include "pico/time.h"
#include "GUI_Icons.h"

#include "SmartRender.h"

#include "Interpreter.h"

#include "ColliderPackage.h"

#include "GUI.h"

#define CAMERA_SPRITE -1

// Func prototypes
uint32_t RunProgram();

///////////////////////// SCENE VIEW
#pragma region

#define SELECT_OBJECTS_UI 0
#define SELECT_LAYOUT_UI 1
#define SELECT_MODULE_UI 2
#define SELECT_OPTIONS_UI 3

#define OPEN_NAV_PANEL 4

#define PLAY_SCENE_UI 5
#define EXIT_EDITOR_UI 6

#define RENDER_MODE_BUTTON 7

#define ADD_THING_BUTTON 8

#define OBJECT_BUTTONS 9

#define SCRIPT_DELETE_BUTTON (OBJECT_BUTTONS + MAX_OBJECTS + 1)
#define MODULE_BUTTONS (SCRIPT_DELETE_BUTTON + 1)

#define MAIN_PANEL 0
#define OBJECT_PANEL 1
#define NAV_PANEL 2

#define MODULE_PANELS 3

bool NavPanelOpen = 0;

UIButton buttons[MODULE_BUTTONS + 50];

UIPanel panels[4];

UIButton *currentButton = NULL;
bool sceneRefreshUI = true;

EngineScene *currentScene;

uint8_t currentObject = 0;

uint32_t errors[10];
uint8_t errorCount;

char *packageNames[] = {
    "Collision"};
#define PACKAGE_COUNT 1

#define NORMAL_FIELD 0
#define VECTOR_X_FIELD 1
#define VECTOR_Y_FIELD 2

typedef struct
{
    int buttonNumber;
    EngineVar *link;
    uint8_t specialMode;
} InputFieldLink;

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

// Scene renderers
#pragma region

uint8_t sceneScale = 4;

void DrawSpriteCentered(int sprite, int x, int y, float scaleX, float scaleY)
{
    printf("drawSprite: pos: (%d,%d) scale: (%f,%f) = %d\n", x, y, scaleX, scaleY, sprite);
    if (sprite < 0)
    {
        const uint16_t (*icon)[16];

        switch (sprite)
        {
        case CAMERA_SPRITE:
            icon = CAMERA_ICON;
            scaleX /= 2;
            scaleY /= 4;
            break;

        default:
            return;
            break;
        }

        scaleX = ceil(scaleX);
        scaleY = ceil(scaleY);

        int topLeftX = x - scaleX * 16 / 2;
        int topLeftY = y - scaleY * 16 / 2;

        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 16; y++)
            {
                // printf("Color: %d\n", sprites[sprite].sprite[x][y]);
                if (icon[y][x] == TRANSPARENT)
                {
                    continue;
                }
                SmartRect(icon[y][x], topLeftX + x * scaleX, topLeftY + y * scaleY, scaleX, scaleY);
            }
        }
        return;
    }
    int topLeftX = x - scaleX * SPRITE_WIDTH / 2;
    int topLeftY = y - scaleY * SPRITE_HEIGHT / 2;

    for (int x = 0; x < SPRITE_WIDTH; x++)
    {
        for (int y = 0; y < SPRITE_HEIGHT; y++)
        {
            if (sprites[sprite].sprite[x][y] == TRANSPARENT)
            {
                continue;
            }
            printf("(%d,%d) %d\n", x, y, sprites[sprite].sprite[x][y]);
            SmartRect(sprites[sprite].sprite[x][y], topLeftX + x * scaleX, topLeftY + y * scaleY, scaleX, scaleY);
        }
    }
}

void DrawRectOutline(uint16_t color, int x, int y, int w, int h)
{
    SmartRect(color, x, y, w, 1);
    SmartRect(color, x, y + h, w, 1);
    SmartRect(color, x, y, 1, h);
    SmartRect(color, x + w, y, 1, h);
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

    EngineObject *camera = NULL;

    for (int i = 0; i < currentScene->objectCount; i++)
    {
        Vector2 scale;

        if (GetObjectDataByName(currentScene->objects[i], "scale")->currentType == TYPE_INT)
        {
            scale.x = GetObjectDataByName(currentScene->objects[i], "scale")->data.i;
            scale.y = GetObjectDataByName(currentScene->objects[i], "scale")->data.i;
        }
        else
        {
            scale.x = GetObjectDataByName(currentScene->objects[i], "scale")->data.XY.x;
            scale.y = GetObjectDataByName(currentScene->objects[i], "scale")->data.XY.y;
        }

        if (strcmp(currentScene->objects[i]->name, "Camera") == 0)
        {
            camera = currentScene->objects[i];
        }

        DrawSpriteCentered(GetObjectDataByName(currentScene->objects[i], "sprite")->data.i,

                           80 + offsetX + sceneScale * GetObjectDataByName(currentScene->objects[i], "position")->data.XY.x,
                           64 + offsetY + sceneScale * -GetObjectDataByName(currentScene->objects[i], "position")->data.XY.y,
                           sceneScale * scale.x,
                           sceneScale * scale.y);

        for (int x = 0; x < currentScene->objects[i]->colliderCount; x++)
        {

            Rect *rect = GetRectByID(currentScene->objects[i]->colliderBoxes[x], colliderRects);

            DrawRectOutline(
                GREEN,
                80 + offsetX + sceneScale * (scale.x * GetObjectDataByName(currentScene->objects[i], "colliderSize")->data.XY.x * (rect->topLeft.x + GetObjectDataByName(currentScene->objects[i], "colliderCenter")->data.XY.x) + GetObjectDataByName(currentScene->objects[i], "position")->data.XY.x),
                64 + offsetY + sceneScale * (scale.y * GetObjectDataByName(currentScene->objects[i], "colliderSize")->data.XY.y * -(rect->topLeft.y + GetObjectDataByName(currentScene->objects[i], "colliderCenter")->data.XY.y) + -GetObjectDataByName(currentScene->objects[i], "position")->data.XY.y),
                sceneScale * scale.x * rect->scale.x * GetObjectDataByName(currentScene->objects[i], "colliderSize")->data.XY.x,
                sceneScale * scale.y * rect->scale.y * GetObjectDataByName(currentScene->objects[i], "colliderSize")->data.XY.y);
        }
    }

    EngineVar *cameraPosition = GetObjectDataByName(camera, "position");

    int width = 160 * sceneScale / GetObjectDataByName(camera, "scale")->data.i;
    int height = 128 * sceneScale / GetObjectDataByName(camera, "scale")->data.i;

    DrawRectOutline(
        WHITE,
        80 + offsetX + sceneScale * cameraPosition->data.XY.x - width / 2,
        64 + offsetY + sceneScale * -cameraPosition->data.XY.y - height / 2,
        width,
        height);

    SmartShow();

    leftBound = 0;
    rightBound = 160;
    topBound = 0;
    bottomBound = 128;
}

#pragma endregion

void DecompileScene()
{
    for (int i = 0; i < currentScene->objectCount; i++)
    {
        for (int s = 0; s < currentScene->objects[i]->scriptCount; s++)
        {
            printf("obj: %d, scr: %d\n", i, s);
            FreeScriptData(currentScene->objects[i]->scriptData[s], false);
        }
    }
}

void RecompileScene()
{
    for (int i = 0; i < currentScene->objectCount; i++)
    {
        for (int s = 0; s < currentScene->objects[i]->scriptCount; s++)
        {

            currentScene->objects[i]->scriptData[s] = ScriptDataConstructor(&scripts[currentScene->objects[i]->scriptIndexes[s]]);
            currentScene->objects[i]->scriptData[s]->linkedObject = currentScene->objects[i];

            // Function prototypes
            uint32_t SetScriptData(EngineScript * script, ScriptData * output, uint8_t scopeLevel);
            uint16_t UnpackErrorMessage(uint32_t error);

            printf("obj: %d, scr: %d\n", i, s);
            uint32_t errorNum = SetScriptData(
                &scripts[currentScene->objects[i]->scriptData[s]->script->ID],
                currentScene->objects[i]->scriptData[s],
                0);

            printf("data set reutn %d\n", errorNum);

            printf("scene name: %s\n", currentScene->name);
            printf("obj name: %s\n", currentScene->objects[i]->name);
            printf("var count: %d\n", currentScene->objects[i]->scriptData[s]->variableCount);

            for (int t = 0; t < currentScene->objects[i]->scriptData[s]->variableCount; t++)
            {
                printf("set scr variable out: %s\n", currentScene->objects[i]->scriptData[s]->data[t].name);

                if (currentScene->objects[i]->scriptData[s]->data[t].currentType == TYPE_FLOAT)
                {
                    printf("is float: %f\n", currentScene->objects[i]->scriptData[s]->data[t].data.f);
                }
            }

            if (errorNum != 0)
            {
                errors[errorCount++] = errorNum;
            }

            uint16_t error = UnpackErrorMessage(errorNum);
            printf("set script error: %s\n", stringPool[error]);
            FreeString(&error);
            print("freed error");
        }
        print("object's scripts set");

        if (currentScene->objects[i]->packages[0])
        {
            print("is collider");
            RecalculateObjectColliders(currentScene->objects[i]);
        }
        else
        {
            print("no collider");
        }
    }
}

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

int SelectScriptToAdd()
{
    uint8_t current = 0;
    bool refresh = true;
    while (1)
    {
        if (refresh)
        {
            Clear();
            WriteWord("Add Script", strlen("Add Script"), 1, 1, 2, RGBTo16(0, 255, 0), TRANSPARENT);
            Rectangle(RGBTo16(255, 200, 0), 0, 18, 160, 1);

            if (scriptCount + PACKAGE_COUNT > 0)
            {
                if (current < PACKAGE_COUNT)
                {
                    WriteWord(packageNames[current], strlen(packageNames[current]), 5, 64, 2, YELLOW, TRANSPARENT);
                }
                else
                {
                    WriteWord(scripts[current - PACKAGE_COUNT].name, strlen(scripts[current - PACKAGE_COUNT].name), 5, 64, 2, WHITE, TRANSPARENT);
                }
            }
            else
                WriteWord("No Scripts", strlen("No Scripts"), 5, 64, 2, RGBTo16(100, 100, 100), TRANSPARENT);

            refresh = false;
            sleep_ms(100);
        }

        if (GetButton() == BUTTON_D)
        {
            current++;
            if (current >= scriptCount + PACKAGE_COUNT)
            {
                current = 0;
            }
            refresh = true;
        }
        if (GetButton() == BUTTON_A)
        {
            if (current == 0)
            {
                current = scriptCount + PACKAGE_COUNT - 1;
            }
            else
            {
                current++;
            }
            refresh = true;
        }

        if (GetButton() == BUTTON_L)
        {
            return -1;
        }

        if (GetButton() == BUTTON_J)
        {
            return current;
        }
    }
}

void DrawModulePage(char *moduleName, uint8_t totalVariableCount, uint8_t *variableCount, EngineVar **linkVars, uint8_t *buttonIndex, InputFieldLink *variableLinks)
{
    int height = 13;
    panels[MODULE_PANELS].visible = true;
    SetPanel(&panels[MODULE_PANELS], 3, height, 154, 15 * (1 + totalVariableCount), 6, RGBTo16(70, 70, 70), RGBTo16(120, 120, 120));
    DrawPanel(&panels[MODULE_PANELS]);
    panels[MODULE_PANELS].visible = false;

    WriteWord(
        (moduleName),
        strlen(moduleName),
        31,
        height + 3,
        1,
        WHITE,
        ORANGE);

    SetButton(&buttons[SCRIPT_DELETE_BUTTON], 5, 15, 23, 11, 5, BLACK, WHITE, RED, WHITE, NULL, NULL, NULL, NULL);
    AddTextToButton(&buttons[SCRIPT_DELETE_BUTTON], "DEL", RED, 1);
    DrawButton(&buttons[SCRIPT_DELETE_BUTTON]);

    for (int x = 0; x < totalVariableCount; x++)
    {
        printf("Var button: %d\n", x);
        printf("var link: %s\n", linkVars[x]->name);

        variableLinks[*variableCount].link = linkVars[x];

        printf("var: %s\n", linkVars[x]->name);

        WriteWord(
            (linkVars[x]->name),
            strlen(linkVars[x]->name),
            6,
            height + 15 * (x + 1) + 2,
            1,
            WHITE,
            RGBTo16(0, 0, 80));

        uint8_t buttonStart = 15 + (FONT_WIDTHS[0] + 1) * strlen(linkVars[x]->name);

        if (variableLinks[*variableCount].link->currentType == TYPE_VECTOR)
        {
            variableLinks[*variableCount].link = linkVars[x];
            variableLinks[*variableCount].buttonNumber = *buttonIndex;
            variableLinks[*variableCount].specialMode = VECTOR_X_FIELD;

            SetButton(&buttons[*buttonIndex], buttonStart, height + 15 * (x + 1), (145 - buttonStart) / 2 - 5, 12, 3, BLACK, RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, NULL, NULL, NULL, NULL);
            buttons[*buttonIndex].textAnchor = LEFT_ANCHOR;
            char buffer[32];
            sprintf(buffer, "%f", linkVars[x]->data.XY.x);
            AddTextToButton(&buttons[*buttonIndex], buffer, WHITE, 1);
            DrawButton(&buttons[(*buttonIndex)++]);
            (*variableCount)++;

            buttonStart += (145 - buttonStart) / 2;

            variableLinks[*variableCount].link = linkVars[x];
            variableLinks[*variableCount].buttonNumber = *buttonIndex;
            variableLinks[*variableCount].specialMode = VECTOR_Y_FIELD;

            SetButton(&buttons[*buttonIndex], buttonStart, height + 15 * (x + 1), (145 - buttonStart), 12, 3, BLACK, RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, NULL, NULL, NULL, NULL);
            buttons[*buttonIndex].textAnchor = LEFT_ANCHOR;
            sprintf(buffer, "%f", linkVars[x]->data.XY.y);
            AddTextToButton(&buttons[*buttonIndex], buffer, WHITE, 1);
            DrawButton(&buttons[(*buttonIndex)++]);
            (*variableCount)++;
        }
        else
        {

            variableLinks[*variableCount].buttonNumber = *buttonIndex;
            variableLinks[*variableCount].specialMode = NORMAL_FIELD;

            SetButton(&buttons[*buttonIndex], buttonStart, height + 15 * (x + 1), 145 - buttonStart, 12, 3, BLACK, RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, NULL, NULL, NULL, NULL);
            buttons[*buttonIndex].textAnchor = LEFT_ANCHOR;
            uint16_t data = SerializeVar(variableLinks[*variableCount].link);
            AddTextToButton(&buttons[*buttonIndex], stringPool[data], WHITE, 1);
            FreeString(&data);
            DrawButton(&buttons[(*buttonIndex)++]);
            (*variableCount)++;
        }
    }
}

char *NewObject()
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
    bool drawCaret = true;

    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("New Object", strlen("New Object"), 1, 1, 2, RGBTo16(0, 255, 200), TRANSPARENT);

            Rectangle(RGBTo16(0, 255, 200), 0, 18, 160, 1);

            WriteWord("Object Name", strlen("Object Name"), 2, 31, 1, WHITE, TRANSPARENT);

            Rectangle((option == 0) ? WHITE : RGBTo16(150, 150, 150), 2, 40, 156, 11);
            Rectangle((option == 0) ? RGBTo16(0, 0, 100) : BLACK, 3, 41, 154, 9);

            WriteWord(name, strlen(name), 4, 42, 1, WHITE, TRANSPARENT);

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

                        char *output = malloc(caretPosition + 1);
                        strcpy(output, name);
                        free(name);
                        return output;
                    }
                    else if (option == 0)
                    {
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

    SetButton(&buttons[ADD_THING_BUTTON], 0, 118, 20, 10, 5, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, NULL, NULL, NULL, NULL);

    AddTextToButton(&buttons[SELECT_OBJECTS_UI], "Objects", WHITE, 1);
    AddTextToButton(&buttons[SELECT_LAYOUT_UI], "Layout", WHITE, 1);
    AddTextToButton(&buttons[SELECT_MODULE_UI], "Modules", WHITE, 1);
    AddTextToButton(&buttons[SELECT_OPTIONS_UI], "Options", WHITE, 1);

    AddTextToButton(&buttons[PLAY_SCENE_UI], "Play", WHITE, 1);
    AddTextToButton(&buttons[EXIT_EDITOR_UI], "Exit", WHITE, 1);

    AddTextToButton(&buttons[ADD_THING_BUTTON], "Add", WHITE, 1);

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

    currentObject = 0;

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

    // EngineVar *variableList[MAX_SCRIPTS_PER_OBJECT * 10];
    InputFieldLink variableLinks[10];
    uint8_t variableCount = 0;

    // Prototype functions
    bool EqualType(EngineVar * var1, EngineVar * var2, uint8_t type);
    float ShuntYard(char *equation, uint16_t equationLength, EngineVar *output, ScriptData *scriptData);
    bool EqualType(EngineVar * var1, EngineVar * var2, uint8_t type);

    uint8_t modulePage = 0;

    while (1)
    {
        if (!showKeyboard)
            UpdateUIButtons();

        if ((buttons[OPEN_NAV_PANEL].onPressDown && currentMode != 1) || (GetButton() == BUTTON_L && currentMode == 1))
        {
            for (int i = SELECT_LAYOUT_UI; i <= SELECT_OPTIONS_UI; i++)
            {
                buttons[i].isFocused = false;
            }
            RefocusButton(&buttons[SELECT_OBJECTS_UI], false);
            SetNavPanelVisibility(true);

            buttons[OPEN_NAV_PANEL].leftNext = NULL;
            buttons[OPEN_NAV_PANEL].downNext = NULL;
            sleep_ms(100);
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
                else
                {
                    bottom = &buttons[ADD_THING_BUTTON];
                    buttons[ADD_THING_BUTTON].upNext = &buttons[OBJECT_BUTTONS + i];
                }
                SetButton(&buttons[OBJECT_BUTTONS + i], 10, 10 + i * 17, 140, 15, 6, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, top, &buttons[OPEN_NAV_PANEL], bottom, NULL);
                AddTextToButton(&buttons[OBJECT_BUTTONS + i], currentScene->objects[i]->name, WHITE, 1);
            }

            if (currentScene->objectCount > 0)
            {
                buttons[OPEN_NAV_PANEL].leftNext = &buttons[OBJECT_BUTTONS];
            }

            SetNavPanelVisibility(false);
            buttons[ADD_THING_BUTTON].visible = true;
            DrawButton(&buttons[ADD_THING_BUTTON]);
            sleep_ms(100);
        }

        // Layout mode
        if (buttons[SELECT_LAYOUT_UI].onPressDown || (refresh && currentMode == 1))
        {
            currentMode = 1;
            HideAllButtons();

            RefocusButton(&buttons[OPEN_NAV_PANEL], false);

            panels[MAIN_PANEL].fillColor = RGBTo16(0, 0, 0);

            SetNavPanelVisibility(false);
            buttons[OPEN_NAV_PANEL].visible = false;

            refreshSceneView = true;
            Clear();
            SmartShowAll();

            scenePosX = 0;
            scenePosY = 0;

            sleep_ms(100);
        }

        // Module mode
        if (buttons[SELECT_MODULE_UI].onPressDown || (refresh && currentMode == 2))
        {
            currentMode = 2;
            HideAllButtons();

            RefocusButton(&buttons[OPEN_NAV_PANEL], false);

            panels[MAIN_PANEL].fillColor = RGBTo16(0, 0, 0);

            SetNavPanelVisibility(false);

            WriteWord(currentScene->objects[currentObject]->name, strlen(currentScene->objects[currentObject]->name), 3, 3, 1, WHITE, BLUE);

            int height = 13;
            uint8_t buttonIndex = MODULE_BUTTONS;
            variableCount = 0;
            printf("Script count: %d\n", currentScene->objects[currentObject]->scriptCount);

            // Serialize var prototype
            uint16_t SerializeVar(EngineVar * variable);

            uint8_t objectPackageCount = 1;
            bool doCollider = false;
            bool doPhysics = false;

            if (currentScene->objects[currentObject]->packages[0])
            {
                doCollider = true;
                objectPackageCount++;
            }

            uint8_t scriptCount = currentScene->objects[currentObject]->scriptCount + 1 + objectPackageCount;
            if (modulePage >= scriptCount)
            {
                modulePage = 0;
            }

            if (modulePage == 0)
            {
                EngineVar **drawVarList = ObjectDataToList(currentScene->objects[currentObject]);
                DrawModulePage("Base", BASE_VARS, &variableCount, drawVarList, &buttonIndex, variableLinks);
                free(drawVarList);
            }
            else if (doCollider && modulePage == 1)
            {
                EngineVar **drawVarList = ObjectDataToList(currentScene->objects[currentObject]);
                DrawModulePage("Collision", COLLIDER_VARS, &variableCount, &drawVarList[BASE_VARS], &buttonIndex, variableLinks);
                free(drawVarList);
            }
            else
            {
                uint8_t scriptIndex = modulePage - objectPackageCount;
                printf("count: %d\n", currentScene->objects[currentObject]->scriptData[scriptIndex]->variableCount);
                uint8_t scriptVariables = (currentScene->objects[currentObject]->scriptData[scriptIndex]->variableCount);

                EngineVar **passVars = (EngineVar **)malloc(sizeof(EngineVar *) * scriptVariables);

                for (int i = 0; i < scriptVariables; i++)
                {
                    passVars[i] = &currentScene->objects[currentObject]->scriptData[scriptIndex]->data[i];

                    printf("passing data: %s\n", passVars[i]->name);

                    if (passVars[i]->currentType == TYPE_FLOAT)
                    {
                        printf("is float: %f\n", currentScene->objects[currentObject]->scriptData[scriptIndex]->data[i].data.f);
                    }
                    else
                    {
                        print("isn't float");
                    }
                }
                DrawModulePage(scripts[currentScene->objects[currentObject]->scriptIndexes[scriptIndex]].name,
                               scriptVariables,
                               &variableCount,
                               passVars,
                               &buttonIndex,
                               variableLinks);
                free(passVars);
            }

            if (buttonIndex > MODULE_BUTTONS)
            {
                buttons[OPEN_NAV_PANEL].downNext = &buttons[SCRIPT_DELETE_BUTTON];

                for (int i = MODULE_BUTTONS; i < buttonIndex; i++)
                {
                    if (i > MODULE_BUTTONS)
                    {
                        buttons[i].upNext = &buttons[i - 1];
                    }
                    else
                    {
                        buttons[i].upNext = &buttons[SCRIPT_DELETE_BUTTON];
                        buttons[SCRIPT_DELETE_BUTTON].downNext = &buttons[i];
                    }
                    if (i < buttonIndex - 1)
                    {
                        buttons[i].downNext = &buttons[i + 1];
                    }
                    else
                    {
                        buttons[i].downNext = &buttons[ADD_THING_BUTTON];
                        buttons[ADD_THING_BUTTON].upNext = &buttons[i];
                    }
                }
            }
            else
            {
                buttons[OPEN_NAV_PANEL].downNext = &buttons[ADD_THING_BUTTON];
                buttons[ADD_THING_BUTTON].upNext = &buttons[OPEN_NAV_PANEL];
            }
            buttons[SCRIPT_DELETE_BUTTON].upNext = &buttons[OPEN_NAV_PANEL];
            buttons[ADD_THING_BUTTON].visible = true;
            DrawButton(&buttons[ADD_THING_BUTTON]);
            sleep_ms(100);
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

            sleep_ms(100);
        }

        if (buttons[EXIT_EDITOR_UI].onPressDown)
        {
            DecompileScene();
            free(modifyVariable);

            FlushBuffer();

            char *fileName = malloc(strlen(FILE_IDENTIFIER) + strlen(currentProjectName) + strlen(currentScene->name) + 4);
            sprintf(fileName, "%s`%s`%s",
                    FILE_IDENTIFIER,
                    currentProjectName,
                    currentScene->name);

            File *scene = GetFile(fileName);

            if (scene->startBlock == 0)
            {
                scene = CreateFile(fileName, strlen(fileName), MAX_OBJECTS * 2 + 2);
            }

            print("starting object save");

            int startBlock = scene->startBlock;
            char buffer[4];
            sprintf(buffer, "%d", currentScene->objectCount);
            WriteFile(scene, buffer, strlen(buffer) + 1);
            scene->startBlock -= 1;
            for (int obj = 0; obj < currentScene->objectCount; obj++)
            {
                char *serializedObject = SerializeObject(currentScene->objects[obj]);
                print("obj serialized");
                WriteFile(scene, serializedObject, strlen(serializedObject) + 1);
                scene->startBlock -= 2;
                free(serializedObject);
            }

            free(scene);
            free(fileName);

            DisengageSD();

            print("saved scene to project");

            SaveProject(program);

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
                scenePosX -= 3;
                refreshSceneView = true;
            }

            if (GetButton() == BUTTON_A)
            {
                scenePosX += 3;
                refreshSceneView = true;
            }

            if (GetButton() == BUTTON_W)
            {
                scenePosY += 3;
                refreshSceneView = true;
            }

            if (GetButton() == BUTTON_S)
            {
                scenePosY -= 3;
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

            if (gpio_get(BUTTON_J) == 0)
            {
                refreshSceneView = true;
                GetObjectDataByName(currentScene->objects[currentObject], "position")->data.XY.x = -scenePosX / sceneScale;
                GetObjectDataByName(currentScene->objects[currentObject], "position")->data.XY.y = scenePosY / sceneScale;
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
                        InputFieldLink *inputLink = &variableLinks[variableBeingModified];

                        if (inputLink->specialMode == VECTOR_X_FIELD)
                        {
                            EngineVar *output = malloc(sizeof(EngineVar));
                            ScriptData *emptyScripData = malloc(sizeof(ScriptData));
                            emptyScripData->variableCount = 0;
                            uint32_t error = ShuntYard(modifyVariable, strlen(modifyVariable), output, emptyScripData);

                            if (error == 0)
                            {
                                if (output->currentType == TYPE_FLOAT)
                                {
                                    inputLink->link->data.XY.x = (int)output->data.f;
                                }
                                if (output->currentType == TYPE_INT)
                                {
                                    inputLink->link->data.XY.x = output->data.i;
                                }
                            }
                            free(output);
                            free(emptyScripData);
                        }
                        else if (inputLink->specialMode == VECTOR_Y_FIELD)
                        {
                            EngineVar *output = malloc(sizeof(EngineVar));
                            ScriptData *emptyScripData = malloc(sizeof(ScriptData));
                            emptyScripData->variableCount = 0;
                            uint32_t error = ShuntYard(modifyVariable, strlen(modifyVariable), output, emptyScripData);

                            if (error == 0)
                            {
                                if (output->currentType == TYPE_FLOAT)
                                {
                                    inputLink->link->data.XY.y = (int)output->data.f;
                                }
                                if (output->currentType == TYPE_INT)
                                {
                                    inputLink->link->data.XY.y = output->data.i;
                                }
                            }
                            free(output);
                            free(emptyScripData);
                        }
                        else
                        {

                            if (inputLink->link->currentType == TYPE_STRING)
                            {
                                if (inputLink->link->data.s)
                                {
                                    FreeString(&inputLink->link->data.s);
                                }
                                inputLink->link->data.s = PoolString();
                                strcpy(stringPool[inputLink->link->data.s], modifyVariable);
                                stringPool[inputLink->link->data.s][STRING_POOL_WIDTH - 1] = '\0';
                            }
                            else
                            {

                                EngineVar *output = malloc(sizeof(EngineVar));
                                ScriptData *emptyScripData = malloc(sizeof(ScriptData));
                                emptyScripData->variableCount = 0;
                                uint32_t error = ShuntYard(modifyVariable, strlen(modifyVariable), output, emptyScripData);

                                if (error == 0)
                                {
                                    if (EqualType(inputLink->link, output, TYPE_BOOL))
                                    {
                                        inputLink->link->data.b = output->data.b;
                                    }
                                    if (EqualType(inputLink->link, output, TYPE_VECTOR))
                                    {
                                        inputLink->link->data.XY.x = output->data.XY.x;
                                        inputLink->link->data.XY.y = output->data.XY.y;
                                    }
                                    if (EqualType(inputLink->link, output, TYPE_FLOAT))
                                    {
                                        inputLink->link->data.f = output->data.f;
                                    }
                                    if (inputLink->link->currentType == TYPE_FLOAT && output->currentType == TYPE_INT)
                                    {
                                        inputLink->link->data.f = output->data.i;
                                    }
                                    if (inputLink->link->currentType == TYPE_INT && output->currentType == TYPE_FLOAT)
                                    {
                                        inputLink->link->data.i = (int)output->data.f;
                                    }
                                    if (EqualType(inputLink->link, output, TYPE_INT))
                                    {
                                        inputLink->link->data.i = output->data.i;
                                    }
                                    if (EqualType(inputLink->link, output, TYPE_STRING))
                                    {
                                        uint16_t temp = PoolString();
                                        sprintf(stringPool[temp], "%s", output->data.s);

                                        FreeString(&inputLink->link->data.s);
                                        FreeString(&output->data.s);

                                        inputLink->link->data.s = temp;
                                    }
                                    if (EqualType(inputLink->link, output, TYPE_OBJ))
                                    {
                                        inputLink->link->data.objID = output->data.objID;
                                    }
                                }
                                free(output);
                                free(emptyScripData);
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
                        memset(modifyVariable,0,sizeof(modifyVariable));
                        caretPosition = 0;
                    }
                }

                if (buttons[ADD_THING_BUTTON].isPressed)
                {
                    int script = SelectScriptToAdd();

                    if (script == 0)
                    {
                        AddColliderToObject(currentScene->objects[currentObject], COLLIDER_MESH, true);
                        script = -1;
                    }
                    else
                    {
                        script -= PACKAGE_COUNT;
                    }

                    if (script != -1)
                    {
                        currentScene->objects[currentObject]->scriptData[currentScene->objects[currentObject]->scriptCount] = ScriptDataConstructor(&scripts[script]);
                        currentScene->objects[currentObject]->scriptData[currentScene->objects[currentObject]->scriptCount]->linkedObject = currentScene->objects[currentObject];
                        currentScene->objects[currentObject]->scriptIndexes[currentScene->objects[currentObject]->scriptCount] = script;
                        // Function prototypes
                        uint32_t SetScriptData(EngineScript * script, ScriptData * output, uint8_t scopeLevel);
                        uint16_t UnpackErrorMessage(uint32_t error);
                        //

                        uint32_t errorNum = SetScriptData(
                            &scripts[script],
                            currentScene->objects[currentObject]->scriptData[currentScene->objects[currentObject]->scriptCount],
                            0);

                        for (int t = 0; t < currentScene->objects[currentObject]->scriptData[currentScene->objects[currentObject]->scriptCount]->variableCount; t++)
                        {
                            printf("set scr variable out: %s\n", currentScene->objects[currentObject]->scriptData[currentScene->objects[currentObject]->scriptCount]->data[t].name);
                        }

                        if (errorNum != 0)
                        {
                            errors[errorCount++] = errorNum;
                        }

                        uint16_t error = UnpackErrorMessage(errorNum);
                        printf("set script error: %s\n", stringPool[error]);
                        FreeString(&error);

                        currentScene->objects[currentObject]->scriptCount++;
                    }
                    refresh = true;
                    sleep_ms(160);
                }

                uint8_t scriptCount = currentScene->objects[currentObject]->scriptCount + 1;
                if (currentScene->objects[currentObject]->packages[0])
                {
                    scriptCount++;
                }
                if (GetButton() == BUTTON_D)
                {
                    modulePage++;
                    if (modulePage >= scriptCount)
                    {
                        modulePage = 0;
                    }
                    refresh = true;
                }
                if (GetButton() == BUTTON_A)
                {
                    if (modulePage == 0)
                    {
                        modulePage = scriptCount - 1;
                    }
                    else
                    {
                        modulePage--;
                    }
                    refresh = true;
                }
            }
        }

        // Manage settings
        if (currentMode == 3 && buttons[RENDER_MODE_BUTTON].isPressed)
        {
            renderMode++;
            if (renderMode == 3)
            {
                renderMode = 0;
            }
            UpdateRenderButton(true);
            sleep_ms(120);
        }

        // Play scene
        if (buttons[PLAY_SCENE_UI].isPressed)
        {
            RunProgram();
            refresh = true;
        }

        if (currentMode == 0)
        {
            for (int i = 0; i < currentScene->objectCount; i++)
            {
                if (buttons[i + OBJECT_BUTTONS].isPressed)
                {
                    currentObject = i;
                    break;
                }
            }

            if (buttons[ADD_THING_BUTTON].isPressed)
            {
                char *name = NewObject();
                print("returned");
                if (name != NULL)
                {
                    print("adding obj");
                    currentScene->objects[currentScene->objectCount] = ObjectConstructor(0, name, strlen(name));
                    currentScene->objectCount++;
                    free(name);
                }
                else
                {
                    print("is null");
                }
                sleep_ms(120);
                refresh = true;
            }
        }
    }
}

void EditScene(int sceneIndex)
{
    currentScene = &scenes[sceneIndex];

    RecompileScene();

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
                        scenes[index].objects[scenes[index].objectCount] = ObjectConstructor(0, "Camera", strlen("Camera"));
                        GetObjectDataByName(scenes[index].objects[scenes[index].objectCount], "scale")->currentType = TYPE_INT;
                        GetObjectDataByName(scenes[index].objects[scenes[index].objectCount], "scale")->data.i = 2;
                        GetObjectDataByName(scenes[index].objects[scenes[index].objectCount], "sprite")->data.i = CAMERA_SPRITE;

                        scenes[index].objectCount++;

                        scenes[index].objects[scenes[index].objectCount] = ObjectConstructor(0, "Object1", strlen("Object1"));
                        scenes[index].objectCount++;

                        EditScene(index);
                    }
                    free(sceneName);
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

#pragma endregion

///////////////////////// Run program
#pragma region

uint32_t RunProgram()
{
    bool UI_PrintToScreen(char *message, bool isError);

    EngineObject *camera = NULL;

    unsigned long exitHoldTime = millis();

    for (int i = 0; i < currentScene->objectCount; i++)
    {
        if (strcmp(currentScene->objects[i]->name, "Camera") == 0)
        {
            camera = currentScene->objects[i];
            break;
        }
    }

    if (camera == NULL)
    {
        return CreateError(SCENE_ERROR, NO_CAMERA_DEFINED, 0);
    }

    for (int obj = 0; obj < currentScene->objectCount; obj++)
    {
        for (int scr = 0; scr < currentScene->objects[obj]->scriptCount; scr++)
        {

            printf("backup: obj:%d, scr:%d\n", obj, scr);
            printf("scrdata lines: %d\n", currentScene->objects[obj]->scriptData[scr]->lineCount);
            ScriptData *scrData = currentScene->objects[obj]->scriptData[scr];
            print("got scr data");
            if (scrData == NULL)
            {
                print("null scr data");
            }
            if (scrData->backupData != NULL)
            {
                print("free backup data");
                free(scrData->backupData);
            }
            print("freed");
            scrData->backupData = malloc(sizeof(EngineVar) * scrData->variableCount);
            print("malloced");
            scrData->backupVarCount = scrData->variableCount;
            for (int var = 0; var < scrData->variableCount; var++)
            {
                printf("backup: %d\n", var);
                strcpy(scrData->backupData[var].name, scrData->data[var].name);
                scrData->backupData[var].currentType = scrData->data[var].currentType;

                scrData->backupData[var].data = scrData->data[var].data;
            }
            print("backed up!");
        }
        printf("object %d fin\n", obj);
    }
    print("fin");

    Clear();
    SmartClear();
    SmartShowAll();

    while (1)
    {
        uint8_t cameraScale;
        cameraScale = GetObjectDataByName(camera, "scale")->data.i;

        int cameraX = GetObjectDataByName(camera, "position")->data.XY.x;
        int cameraY = GetObjectDataByName(camera, "position")->data.XY.y;

        SmartClear();
        for (int i = 0; i < currentScene->objectCount; i++)
        {
            GetObjectDataByName(currentScene->objects[i], "position")->data.XY.x += GetObjectDataByName(currentScene->objects[i], "velocity")->data.XY.x;
            GetObjectDataByName(currentScene->objects[i], "position")->data.XY.y += GetObjectDataByName(currentScene->objects[i], "velocity")->data.XY.y;
            print("added velocity to position");
            float multiplier = ((float)(100 - (abs(GetObjectDataByName(currentScene->objects[i], "drag")->data.i) % 101))) / 100;

            GetObjectDataByName(currentScene->objects[i], "velocity")->data.XY.x *= multiplier;
            GetObjectDataByName(currentScene->objects[i], "velocity")->data.XY.y *= multiplier;
            print("modified velocity");

            if (GetObjectDataByName(currentScene->objects[i], "sprite")->data.i < 0)
            {
                continue;
            }

            Vector2 scale;
            EngineVar *objScale = GetObjectDataByName(currentScene->objects[i], "scale");
            if (objScale->currentType == TYPE_INT)
            {
                scale.x = objScale->data.i;
                scale.y = objScale->data.i;
            }
            else
            {
                scale.x = objScale->data.XY.x;
                scale.y = objScale->data.XY.y;
            }

            printf("Scale: (%f,%f)\n", scale.x, scale.y);
            printf("Position: (%f,%f)\n", GetObjectDataByName(currentScene->objects[i], "position")->data.XY.x, GetObjectDataByName(currentScene->objects[i], "position")->data.XY.y);
            printf("Sprite: %d\n", GetObjectDataByName(currentScene->objects[i], "sprite")->data.i);
            DrawSpriteCentered(
                GetObjectDataByName(currentScene->objects[i], "sprite")->data.i,
                80 + cameraScale * GetObjectDataByName(currentScene->objects[i], "position")->data.XY.x - cameraX,
                64 + cameraScale * -GetObjectDataByName(currentScene->objects[i], "position")->data.XY.y - cameraY,
                cameraScale * scale.x,
                cameraScale * scale.y);
        }
        print("showing");
        SmartShow();
        print("showed");
        printf("count: %d\n", currentScene->objectCount);
        for (int obj = 0; obj < currentScene->objectCount; obj++)
        {
            printf("obj: %d\n", obj);
            EngineObject *testObj = currentScene->objects[obj];
            print("got object");
            if (testObj == NULL)
            {
                print("is null");
            }
            uint8_t testCount = currentScene->objects[obj]->scriptCount;
            printf("got count %d\n", testCount);

            for (int scr = 0; scr < testCount; scr++)
            {
                printf("ex script: obj: %d, scr: %d\n", obj, scr);
                currentScene->objects[obj]->scriptData[scr]->currentLine = 0;
                while (currentScene->objects[obj]->scriptData[scr]->currentLine < currentScene->objects[obj]->scriptData[scr]->lineCount)
                {
                    uint32_t errorNum = ExecuteLine(currentScene->objects[obj]->scriptData[scr]->script, currentScene->objects[obj]->scriptData[scr]);
                    uint16_t error = UnpackErrorMessage(errorNum);
                    printf("Execute line result: %s\n", stringPool[error]);
                    if (errorNum != 0)
                    {
                        UI_PrintToScreen(stringPool[error], true);
                        sleep_ms(3000);
                        FreeString(&error);
                        return errorNum;
                    }
                    FreeString(&error);
                }
            }
            print("executed scripts for this object");
            print("count: ");
            printf("%d\n", currentScene->objectCount);
        }
        print("/////////////////////////////////////////////////// script done execute");

        ColliderStep();

        print("///////////////////////////////////////////// COLLISION DONE");

        if (gpio_get(BUTTON_S) == 0 && gpio_get(BUTTON_K) == 0)
        {
            print("exiting");
            if (millis() - exitHoldTime > 2000)
            {

                break;
            }
        }
        else
        {
            exitHoldTime = millis();
        }
    }

    for (int obj = 0; obj < currentScene->objectCount; obj++)
    {
        for (int scr = 0; scr < currentScene->objects[obj]->scriptCount; scr++)
        {

            ScriptData *scrData = currentScene->objects[obj]->scriptData[scr];

            scrData->variableCount = scrData->backupVarCount;
            for (int var = 0; var < scrData->variableCount; var++)
            {
                strcpy(scrData->data[var].name, scrData->backupData[var].name);
                scrData->data[var].currentType = scrData->backupData[var].currentType;

                scrData->data[var].data = scrData->backupData[var].data;
            }

            free(scrData->backupData);
        }
    }

    gpio_put(LEFT_LIGHT, 0);
    gpio_put(RIGHT_LIGHT, 0);

    return 0;
}

#pragma endregion

#endif