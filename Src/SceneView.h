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
#include "PhysicsPackage.h"

#include "GUI.h"

#include "pico/multicore.h"
#include "pico/sync.h"

#include "DebugConsole.h"

#define CAMERA_SPRITE -1

// Func prototypes
uint32_t RunProgram();

///////////////////////// SCENE VIEW
#pragma region

#define OBJECT_MODE 0
#define LAYOUT_MODE 1
#define MODULE_MODE 2
#define CONSOLE_MODE 3
#define OPTIONS_MODE 4

#define SELECT_OBJECTS_UI 0
#define SELECT_LAYOUT_UI 1
#define SELECT_MODULE_UI 2
#define SELECT_CONSOLE_UI 3
#define SELECT_OPTIONS_UI 4

#define OPEN_NAV_PANEL 5

#define PLAY_SCENE_UI 6
#define EXIT_EDITOR_UI 7

#define RENDER_MODE_BUTTON 8

#define ADD_THING_BUTTON 9

#define OBJECT_BUTTONS 10

#define SCRIPT_DELETE_BUTTON (OBJECT_BUTTONS + MAX_OBJECTS + 1)
#define MODULE_BUTTONS (SCRIPT_DELETE_BUTTON + 1)

#define MAIN_PANEL 0
#define OBJECT_PANEL 1
#define NAV_PANEL 2

#define MODULE_PANELS 3

bool NavPanelOpen = 0;

UIButton buttons[MODULE_BUTTONS + 50];

UIPanel panels[4];

UIButton* currentButton = NULL;
bool sceneRefreshUI = true;

EngineScene* currentScene;

uint8_t currentObject = 0;

uint32_t errors[10];
uint8_t errorCount;

char* packageNames[] = {
    "Collision",
    "Physics" };
#define PACKAGE_COUNT 2

#define NORMAL_FIELD 0
#define VECTOR_X_FIELD 1
#define VECTOR_Y_FIELD 2

typedef struct
{
    int buttonNumber;
    EngineVar* link;
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

float sceneScale = 4;

void DrawSpriteCentered(int sprite, int x, int y, float scaleX, float scaleY, int angle)
{
    debugPrintf("drawSprite: pos: (%d,%d) scale: (%f,%f) = %d\n", x, y, scaleX, scaleY, sprite);
    if (sprite < 0)
    {
        const uint16_t(*icon)[16];

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
                // debugPrintf("Color: %d\n", sprites[sprite].sprite[x][y]);
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

    SmartSprite(sprites[sprite].sprite, topLeftX, topLeftY, scaleX, scaleY, -angle);

    /*for (int ix = 0; ix < SPRITE_WIDTH; ix++)
    {
        for (int iy = 0; iy < SPRITE_HEIGHT; iy++)
        {
            if (sprites[sprite].sprite[ix][iy] == TRANSPARENT)
            {
                continue;
            }
            debugPrintf("(%d,%d) %d\n", ix, iy, sprites[sprite].sprite[ix][iy]);

            int xPos = (ix)*cos(angle * (3.1415 / 180)) - (iy)*sin(angle * (3.1415 / 180));
            int yPos = (ix)*sin(angle * (3.1415 / 180)) + (iy)*cos(angle * (3.1415 / 180));



            SmartFilledRect(sprites[sprite].sprite[ix][iy], topLeftX + xPos * scaleX, topLeftY + yPos * scaleY, scaleX, scaleY, angle);
        }
    }*/
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

    if ((16 * sceneScale) >= 2) {
        for (int i = leftBound; i < rightBound; i++)
        {
            if ((i - offsetX - 80) % (int)(16 * sceneScale) == 0)
            {
                SmartRect(RGBTo16(70, 70, 70), i, topBound, 1, bottomBound - topBound);
            }
        }

        for (int i = topBound; i < bottomBound; i++)
        {
            if ((i - offsetY - 64) % (int)(16 * sceneScale) == 0)
            {
                SmartRect(RGBTo16(70, 70, 70), leftBound, i, rightBound - leftBound, 1);
            }
        }
    }

    EngineObject* camera = NULL;

    for (int i = 0; i < currentScene->objectCount; i++)
    {
        Vector2 scale;
        EngineObject* object = currentScene->objects[i];

        if (GetObjectDataByName(object, "scale")->currentType == TYPE_INT)
        {
            scale.x = GetObjectDataByName(object, "scale")->data.i;
            scale.y = GetObjectDataByName(object, "scale")->data.i;
        }
        else
        {
            scale.x = GetObjectDataByName(object, "scale")->data.XY.x;
            scale.y = GetObjectDataByName(object, "scale")->data.XY.y;
        }

        if (strcmp(object->name, "Camera") == 0)
        {
            camera = object;
        }

        DrawSpriteCentered(GetObjectDataByName(object, "sprite")->data.i,

            80 + offsetX + sceneScale * GetObjectDataByName(object, "position")->data.XY.x,
            64 + offsetY + sceneScale * -GetObjectDataByName(object, "position")->data.XY.y,
            sceneScale * scale.x,
            sceneScale * scale.y,
            (int)GetObjectDataByName(object, "angle")->data.f);

        for (int x = 0; x < object->colliderRects.count; x++)
        {

            Rect* rect = ListGetIndex(&object->colliderRects, x);
            //objectScale * cs * (topLeft + colliderCenter) + object pos
            DrawRectOutline(
                GREEN,
                80 + offsetX + sceneScale * (scale.x * GetObjectDataByName(object, "colliderSize")->data.XY.x * (rect->localCenter.x - rect->localScale.x / 2 + GetObjectDataByName(object, "colliderCenter")->data.XY.x) + GetObjectDataByName(object, "position")->data.XY.x),
                64 + offsetY + sceneScale * (scale.y * GetObjectDataByName(object, "colliderSize")->data.XY.y * -(rect->localCenter.y + rect->localScale.y / 2 + GetObjectDataByName(object, "colliderCenter")->data.XY.y) + -GetObjectDataByName(object, "position")->data.XY.y),
                sceneScale * scale.x * rect->localScale.x * GetObjectDataByName(object, "colliderSize")->data.XY.x,
                sceneScale * scale.y * rect->localScale.y * GetObjectDataByName(object, "colliderSize")->data.XY.y);
        }
    }

    EngineVar* cameraPosition = GetObjectDataByName(camera, "position");

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

bool Modal(char* text, bool confirmButton) {
    int left = 160 * 1 / 8;
    int right = 160 * 7 / 8;
    int top = 128 * 1 / 5;
    int bottom = 128 * 4 / 5;

    Rectangle(RGBTo16(255, 255, 255), left, top, right - left, bottom - top);
    Rectangle(RGBTo16(100, 100, 100), left + 1, top + 1, right - left - 2, bottom - top - 2);

    WriteWord(text, strlen(text), left + 5, top + 5, 1, WHITE, TRANSPARENT);

    bool position = false;
    bool refresh = true;

    sleep_ms(160);
    while (1) {
        if (refresh) {
            Rectangle(position ? WHITE : BLUE, left + 5, bottom - 25, 40, 15);
            Rectangle(BLACK, left + 5 + 1, bottom - 25 + 1, 40 - 2, 15 - 2);
            WriteWord("Cancel", strlen("Cancel"), left + 5 + 3, bottom - 25 + 4, 1, WHITE, TRANSPARENT);

            if (confirmButton) {
                Rectangle(position ? BLUE : WHITE, left + 5 + 45, bottom - 25, 47, 15);
                Rectangle(BLACK, left + 5 + 1 + 45, bottom - 25 + 1, 47 - 2, 15 - 2);
                WriteWord("Confirm", strlen("Confirm"), left + 5 + 45 + 3, bottom - 25 + 4, 1, WHITE, TRANSPARENT);
            }

            refresh = false;
        }

        if (GetButton() == BUTTON_D && !position && confirmButton) {
            position = true;
            refresh = true;
        }
        if (GetButton() == BUTTON_A && position && confirmButton) {
            position = false;
            refresh = true;
        }

        if (GetButton() == BUTTON_J) {
            return position;
        }
        if (GetButton() == BUTTON_L) {
            return false;
        }
    }
}

void DecompileScene()
{
    for (int i = 0; i < currentScene->objectCount; i++)
    {
        EngineObject* object = currentScene->objects[i];
        // clear all colliders from object
        while (object->colliderRects.count > 0)
        {
            debugPrint("pop rect");
            free(PopList(&object->colliderRects));
        }

        for (int s = 0; s < object->scriptData.count; s++) {
            debugPrintf("obj: %d\n", i);
            ScriptData* scrData = (ScriptData*)ListGetIndex(&object->scriptData, s);
            debugPrint("popped script");
            debug_sleep(100);

            if (scrData == NULL) {
                debugPrint("scr is null");
                continue;
            }
            else {
                debugPrint("not null");
            }
            debug_sleep(100);

            FreeScriptData(scrData, true);
            debugPrint("cleared scrdata");
            debug_sleep(100);
        }
    }
}

void RecompileScene()
{

    for (int i = 0; i < currentScene->objectCount; i++)
    {
        debugPrintf("recomp obj %d\n", i);
        EngineObject* object = currentScene->objects[i];
        // clear all colliders from object
        while (object->colliderRects.count > 0)
        {
            debugPrint("pop rect");
            free(PopList(&object->colliderRects));
        }

        for (int s = 0; s < object->scriptData.count; s++)
        {
            debugPrintf("set scr %d\n", s);
            ScriptData* scrData = (ScriptData*)ListGetIndex(&object->scriptData, s);

            scrData->linkedObject = object;
            scrData->objectIndex = i;
            scrData->scriptIndex = s;

            // Function prototypes
            uint32_t SetScriptData(EngineScript * script, ScriptData * output, uint8_t scopeLevel);
            uint16_t UnpackErrorMessage(uint32_t error);

            debugPrintf("obj: %d, scr: %d\n", i, s);
            uint32_t errorNum = SetScriptData(
                scrData->script,
                scrData,
                0);

            debugPrintf("data set reutn %d\n", errorNum);

            debugPrintf("scene name: %s\n", currentScene->name);
            debugPrintf("obj name: %s\n", object->name);
            debugPrintf("var count: %d\n", scrData->variables.count);

            for (int t = 0; t < scrData->variables.count; t++)
            {
                EngineVar* var = (EngineVar*)ListGetIndex(&scrData->variables, t);
                debugPrintf("set scr variable out: %s\n", var->name);

                if (var->currentType == TYPE_FLOAT)
                {
                    debugPrintf("is float: %f\n", var->data.f);
                }
            }

            if (errorNum != 0)
            {
                errors[errorCount++] = errorNum;

                void UI_PrintError(ScriptData * scrData, uint32_t error);

                UI_PrintError(scrData, errorNum);
            }


        }
        debugPrint("object's scripts set");

        if (object->packages[0])
        {
            debugPrint("is collider");
            RecalculateObjectColliders(object);
        }
        else
        {
            debugPrint("no collider");
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
    buttons[SELECT_CONSOLE_UI].visible = visible;

    buttons[OPEN_NAV_PANEL].visible = !visible;

    if (visible) {
        buttons[SELECT_CONSOLE_UI].fontColor = ConsoleHasError() ? RED : WHITE;
    }

    Clear();
    // Draw background panels
    for (int i = 0; i < sizeof(panels) / sizeof(UIPanel); i++)
    {
        DrawPanel(&panels[i]);
    }
    // draw background buttons
    for (int i = 5; i < sizeof(buttons) / sizeof(UIButton); i++)
    {
        DrawButton(&buttons[i]);
    }

    // draw front buttons
    DrawPanel(&panels[NAV_PANEL]);
    for (int i = 0; i < 5; i++)
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

void RefocusButton(UIButton* focusTo, bool redraw)
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
                current--;
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

void DrawModulePage(char* moduleName, uint8_t totalVariableCount, uint8_t* variableCount, EngineVar** linkVars, uint8_t* buttonIndex, InputFieldLink* variableLinks)
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
        debugPrintf("Var button: %d\n", x);
        debugPrintf("var link: %s\n", linkVars[x]->name);

        variableLinks[*variableCount].link = linkVars[x];

        debugPrintf("var: %s\n", linkVars[x]->name);

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
            sprintf(buffer, "%.3f", linkVars[x]->data.XY.x);
            AddTextToButton(&buttons[*buttonIndex], buffer, WHITE, 1);
            DrawButton(&buttons[(*buttonIndex)++]);
            (*variableCount)++;

            buttonStart += (145 - buttonStart) / 2;

            variableLinks[*variableCount].link = linkVars[x];
            variableLinks[*variableCount].buttonNumber = *buttonIndex;
            variableLinks[*variableCount].specialMode = VECTOR_Y_FIELD;

            SetButton(&buttons[*buttonIndex], buttonStart, height + 15 * (x + 1), (145 - buttonStart), 12, 3, BLACK, RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, NULL, NULL, NULL, NULL);
            buttons[*buttonIndex].textAnchor = LEFT_ANCHOR;
            sprintf(buffer, "%.3f", linkVars[x]->data.XY.y);
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
            uint16_t data = SerializeVar(variableLinks[*variableCount].link, true);
            AddTextToButton(&buttons[*buttonIndex], stringPool[data], WHITE, 1);
            FreeString(&data);
            DrawButton(&buttons[(*buttonIndex)++]);
            (*variableCount)++;
        }
    }
}

void TriangleTopBase(uint16_t color, int b1X, int b2X, int bY, int vX, int vY)
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
        Rectangle(color, x1, y, x2 - x1, 1);
        x1 -= slope1;
        x2 -= slope2;
    }
}


char* NewObject()
{
    bool refresh = true;
    bool modifyName = false;
    uint8_t option = 0;
    char* name = (char*)malloc((MAX_NAME_LENGTH + 1) * sizeof(char));
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

                        char* output = malloc(caretPosition + 1);
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

    SetButton(&buttons[SELECT_OBJECTS_UI], 85, 5, 70, 15, 7, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, &buttons[SELECT_OPTIONS_UI], NULL, &buttons[SELECT_LAYOUT_UI], NULL);
    SetButton(&buttons[SELECT_LAYOUT_UI], 85, 22, 70, 15, 7, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, &buttons[SELECT_OBJECTS_UI], NULL, &buttons[SELECT_MODULE_UI], NULL);
    SetButton(&buttons[SELECT_MODULE_UI], 85, 39, 70, 15, 7, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, &buttons[SELECT_LAYOUT_UI], NULL, &buttons[SELECT_CONSOLE_UI], NULL);
    SetButton(&buttons[SELECT_CONSOLE_UI], 85, 56, 70, 15, 7, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, &buttons[SELECT_MODULE_UI], NULL, &buttons[SELECT_OPTIONS_UI], NULL);
    SetButton(&buttons[SELECT_OPTIONS_UI], 85, 73, 70, 15, 7, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, &buttons[SELECT_CONSOLE_UI], NULL, &buttons[SELECT_OBJECTS_UI], NULL);

    SetButton(&buttons[PLAY_SCENE_UI], 10, 10, 60, 20, 6, RGBTo16(0, 167, 0), WHITE, BLACK, GREEN, NULL, &buttons[OPEN_NAV_PANEL], &buttons[EXIT_EDITOR_UI], NULL);
    SetButton(&buttons[EXIT_EDITOR_UI], 10, 35, 60, 20, 6, RGBTo16(255, 0, 0), WHITE, BLACK, GREEN, &buttons[PLAY_SCENE_UI], &buttons[OPEN_NAV_PANEL], &buttons[RENDER_MODE_BUTTON], NULL);
    SetButton(&buttons[RENDER_MODE_BUTTON], 10, 60, 60, 20, 6, RGBTo16(100, 100, 100), WHITE, BLACK, GREEN, &buttons[EXIT_EDITOR_UI], &buttons[OPEN_NAV_PANEL], NULL, NULL);

    SetButton(&buttons[OPEN_NAV_PANEL], 150, 0, 10, 20, 5, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, NULL, NULL, NULL, NULL);

    SetButton(&buttons[ADD_THING_BUTTON], 0, 118, 20, 10, 5, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, NULL, NULL, NULL, NULL);

    AddTextToButton(&buttons[SELECT_OBJECTS_UI], "Objects", WHITE, 1);
    AddTextToButton(&buttons[SELECT_LAYOUT_UI], "Layout", WHITE, 1);
    AddTextToButton(&buttons[SELECT_MODULE_UI], "Modules", WHITE, 1);
    AddTextToButton(&buttons[SELECT_CONSOLE_UI], "Console", WHITE, 1);
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

    char* modifyVariable = (char*)malloc(16);

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
    float ShuntYard(char* equation, uint16_t equationLength, EngineVar * output, ScriptData * scriptData);
    bool EqualType(EngineVar * var1, EngineVar * var2, uint8_t type);

    uint8_t modulePage = 0;

    uint8_t objectPackageCount = 1;

    uint8_t objectListOffset = 0;

    while (1)
    {


        if (!showKeyboard)
            UpdateUIButtons();



        if ((buttons[OPEN_NAV_PANEL].onPressDown && currentMode != LAYOUT_MODE) || (GetButton() == BUTTON_L && currentMode == LAYOUT_MODE))
        {
            for (int i = SELECT_LAYOUT_UI; i <= SELECT_OPTIONS_UI; i++)
            {
                buttons[i].isFocused = false;
            }
            RefocusButton(&buttons[currentMode], false);
            SetNavPanelVisibility(true);

            buttons[OPEN_NAV_PANEL].leftNext = NULL;
            buttons[OPEN_NAV_PANEL].downNext = NULL;
            sleep_ms(100);
        }


        // Exit Nav Panel
        if (GetButton() == BUTTON_L && panels[NAV_PANEL].visible)
        {
            refresh = true;
        }

        // Objects mode
        if (buttons[SELECT_OBJECTS_UI].onPressDown || (refresh && currentMode == OBJECT_MODE))
        {
            if (buttons[SELECT_OBJECTS_UI].onPressDown) {
                objectListOffset = 0;
                RefocusButton(&buttons[OPEN_NAV_PANEL], false);
            }
            panels[MAIN_PANEL].fillColor = RGBTo16(80, 80, 80);
            currentMode = OBJECT_MODE;
            HideAllButtons();



            panels[OBJECT_PANEL].visible = true;

            for (int i = objectListOffset; i < min(currentScene->objectCount, objectListOffset + 6); i++)
            {
                UIButton* top = NULL;
                UIButton* bottom = NULL;

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
                SetButton(&buttons[OBJECT_BUTTONS + i], 10, 10 + (i - objectListOffset) * 17, 140, 15, 6, RGBTo16(0, 0, 0), RGBTo16(100, 100, 100), RGBTo16(120, 120, 120), GREEN, top, &buttons[OPEN_NAV_PANEL], bottom, NULL);
                AddTextToButton(&buttons[OBJECT_BUTTONS + i], currentScene->objects[i]->name, WHITE, 1);
            }

            if (currentScene->objectCount > 0)
            {
                buttons[OPEN_NAV_PANEL].leftNext = &buttons[OBJECT_BUTTONS];
            }




            SetNavPanelVisibility(false);
            buttons[ADD_THING_BUTTON].visible = true;
            DrawButton(&buttons[ADD_THING_BUTTON]);

            if (currentScene->objectCount >= objectListOffset + 6) {
                TriangleTopBase(WHITE, 160 / 2 - 5, 160 / 2 + 5, 112, 160 / 2, 120);
            }
            sleep_ms(100);
        }

        // Layout mode
        if (buttons[SELECT_LAYOUT_UI].onPressDown || (refresh && currentMode == LAYOUT_MODE))
        {
            panels[MAIN_PANEL].fillColor = RGBTo16(80, 80, 80);
            currentMode = LAYOUT_MODE;
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
        if (buttons[SELECT_MODULE_UI].onPressDown || (refresh && currentMode == MODULE_MODE))
        {
            panels[MAIN_PANEL].fillColor = RGBTo16(80, 80, 80);
            currentMode = MODULE_MODE;
            HideAllButtons();

            RefocusButton(&buttons[OPEN_NAV_PANEL], false);

            panels[MAIN_PANEL].fillColor = RGBTo16(0, 0, 0);

            SetNavPanelVisibility(false);

            WriteWord(currentScene->objects[currentObject]->name, strlen(currentScene->objects[currentObject]->name), 3, 3, 1, WHITE, BLUE);

            int height = 13;
            uint8_t buttonIndex = MODULE_BUTTONS;
            variableCount = 0;
            debugPrintf("Script count: %d\n", currentScene->objects[currentObject]->scriptData.count);

            // Serialize var prototype
            uint16_t SerializeVar(EngineVar * variable, bool limitDecimals);

            objectPackageCount = 1;
            bool doCollider = false;
            bool doPhysics = false;

            if (currentScene->objects[currentObject]->packages[0])
            {
                doCollider = true;
                objectPackageCount++;
            }

            if (currentScene->objects[currentObject]->packages[1])
            {
                doPhysics = true;
                objectPackageCount++;
            }

            uint8_t scriptCount = currentScene->objects[currentObject]->scriptData.count + objectPackageCount;
            if (modulePage >= scriptCount)
            {
                modulePage = 0;
            }

            if (modulePage == 0)
            {
                EngineVar** drawVarList = ObjectDataToList(currentScene->objects[currentObject]);
                DrawModulePage("Base", BASE_VARS, &variableCount, drawVarList, &buttonIndex, variableLinks);
                free(drawVarList);
            }
            else if (doCollider && modulePage == 1)
            {
                EngineVar** drawVarList = ObjectDataToList(currentScene->objects[currentObject]);
                DrawModulePage("Collision", COLLIDER_VARS, &variableCount, &drawVarList[GetObjectDataIndex(currentScene->objects[currentObject],"colliderCenter")], &buttonIndex, variableLinks);
                free(drawVarList);
            }
            else if ((doPhysics && !doCollider && modulePage == 1) || (doPhysics && doCollider && modulePage == 2))
            {

                EngineVar** drawVarList = ObjectDataToList(currentScene->objects[currentObject]);
                DrawModulePage("Phyiscs", PHYSICS_VARS, &variableCount, &drawVarList[GetObjectDataIndex(currentScene->objects[currentObject],"gravityScale")], &buttonIndex, variableLinks);
                free(drawVarList);
            }
            else
            {
                uint8_t scriptIndex = modulePage - objectPackageCount;

                EngineObject* obj = currentScene->objects[currentObject];
                ScriptData* scrData = (ScriptData*)ListGetIndex(&obj->scriptData, scriptIndex);

                debugPrintf("var count: %d\n", scrData->variables.count);
                uint8_t scriptVariables = (scrData->variables.count);

                EngineVar** passVars = (EngineVar**)malloc(sizeof(EngineVar*) * scriptVariables);

                for (int i = 0; i < scriptVariables; i++)
                {

                    EngineVar* var = (EngineVar*)ListGetIndex(&scrData->variables, i);

                    passVars[i] = var;

                    debugPrintf("passing data: %s\n", passVars[i]->name);

                    if (passVars[i]->currentType == TYPE_FLOAT)
                    {
                        debugPrintf("is float: %f\n", var->data.f);
                    }
                    else
                    {
                        debugPrint("isn't float");
                    }
                }
                DrawModulePage(scrData->script->name,
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
                buttons[OPEN_NAV_PANEL].downNext = &buttons[SCRIPT_DELETE_BUTTON];

                buttons[SCRIPT_DELETE_BUTTON].downNext = &buttons[ADD_THING_BUTTON];
                buttons[SCRIPT_DELETE_BUTTON].upNext = &buttons[OPEN_NAV_PANEL];

                buttons[ADD_THING_BUTTON].upNext = &buttons[SCRIPT_DELETE_BUTTON];
            }
            buttons[ADD_THING_BUTTON].visible = true;
            DrawButton(&buttons[ADD_THING_BUTTON]);
            sleep_ms(100);
        }

        // Settings mode
        if (buttons[SELECT_OPTIONS_UI].onPressDown || (refresh && currentMode == OPTIONS_MODE))
        {
            panels[MAIN_PANEL].fillColor = RGBTo16(80, 80, 80);
            currentMode = OPTIONS_MODE;
            HideAllButtons();

            RefocusButton(&buttons[OPEN_NAV_PANEL], false);

            buttons[OPEN_NAV_PANEL].leftNext = &buttons[PLAY_SCENE_UI];
            buttons[PLAY_SCENE_UI].visible = true;
            buttons[EXIT_EDITOR_UI].visible = true;
            buttons[RENDER_MODE_BUTTON].visible = true;
            SetNavPanelVisibility(false);

            sleep_ms(100);
        }

        // Console mode
        if (buttons[SELECT_CONSOLE_UI].onPressDown || (refresh && currentMode == CONSOLE_MODE))
        {
            currentMode = CONSOLE_MODE;
            HideAllButtons();

            RefocusButton(&buttons[OPEN_NAV_PANEL], false);

            buttons[OPEN_NAV_PANEL].leftNext = NULL;
            buttons[OPEN_NAV_PANEL].downNext = NULL;

            panels[MAIN_PANEL].fillColor = BLACK;

            SetNavPanelVisibility(false);

            uint8_t startY = 5;

            uint8_t messages = 0;
            for (int i = 0; i < CONSOLE_POOL;i++) {
                if (startY > 128 - 5)
                    break;
                if (!console[i].used)
                    continue;
                char* message = TranslateConsoleMessage(i);
                debugPrintf("console message: %s\n", message);



                uint8_t lines = WriteWordLimited(message, strlen(message), 5, startY, 1, console[i].isError ? RED : WHITE, TRANSPARENT, 23);

                Rectangle(RGBTo16(100, 100, 100), 5, startY + lines * (FONT_HEIGHTS[0] + 1) + 1, 150, 1);

                startY += lines * (FONT_HEIGHTS[0] + 1) + 3;

                free(message);
                messages++;
            }

            if (messages == 0) {
                WriteWordLimited("No Messages", strlen("No Messages"), 5, 5, 1, RGBTo16(0, 150, 0), TRANSPARENT, 23);
            }



            sleep_ms(100);
        }






        refresh = false;




        // Manage Scene View
        if (currentMode == LAYOUT_MODE && !panels[NAV_PANEL].visible)
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
                if (sceneScale > 1) {
                    sceneScale += 1;
                }
                else {
                    sceneScale *= 2;
                }
                refreshSceneView = true;
                delay = 100;
            }

            if (GetButton() == BUTTON_K)
            {
                if (sceneScale > 1) {
                    sceneScale -= 1;
                }
                else {
                    sceneScale /= 2;
                }
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
        if (currentMode == MODULE_MODE)
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
                        InputFieldLink* inputLink = &variableLinks[variableBeingModified];

                        if (inputLink->specialMode == VECTOR_X_FIELD)
                        {
                            EngineVar* output = malloc(sizeof(EngineVar));
                            ScriptData* emptyScripData = malloc(sizeof(ScriptData));
                            uint32_t error = ShuntYard(modifyVariable, strlen(modifyVariable), output, emptyScripData);

                            if (error == 0)
                            {
                                if (output->currentType == TYPE_FLOAT)
                                {
                                    inputLink->link->data.XY.x = output->data.f;
                                }
                                if (output->currentType == TYPE_INT)
                                {
                                    inputLink->link->data.XY.x = (float)output->data.i;
                                }
                            }
                            free(output);
                            free(emptyScripData);
                        }
                        else if (inputLink->specialMode == VECTOR_Y_FIELD)
                        {
                            EngineVar* output = malloc(sizeof(EngineVar));
                            ScriptData* emptyScripData = malloc(sizeof(ScriptData));
                            uint32_t error = ShuntYard(modifyVariable, strlen(modifyVariable), output, emptyScripData);

                            if (error == 0)
                            {
                                if (output->currentType == TYPE_FLOAT)
                                {
                                    inputLink->link->data.XY.y = output->data.f;
                                }
                                if (output->currentType == TYPE_INT)
                                {
                                    inputLink->link->data.XY.y = (float)output->data.i;
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

                                EngineVar* output = malloc(sizeof(EngineVar));
                                ScriptData* emptyScripData = malloc(sizeof(ScriptData));
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
                                        inputLink->link->data.objIndex = output->data.objIndex;
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
                        memset(modifyVariable, 0, sizeof(modifyVariable));
                        caretPosition = 0;
                    }
                }

                if (buttons[ADD_THING_BUTTON].isPressed)
                {
                    int script = SelectScriptToAdd();

                    if (script == 0)
                    {
                        AddColliderToObject(currentScene->objects[currentObject], COLLIDER_MESH, true, 1);
                        script = -1;
                    }
                    else if (script == 1)
                    {
                        AddPhysicsToObject(currentScene->objects[currentObject], DEFAULT_GRAV);
                        script = -1;
                    }
                    else
                    {
                        script -= PACKAGE_COUNT;
                    }

                    if (script != -1)
                    {
                        EngineObject* obj = currentScene->objects[currentObject];
                        debugPrintf("script ID: %d\n", scripts[script].ID);
                        ScriptData* scrData = ScriptDataConstructor(&scripts[script]);

                        debugPrintf("script ID from data: %d\n", scrData->script->ID);

                        PushList(&obj->scriptData, scrData);

                        scrData->linkedObject = obj;
                        scrData->objectIndex = currentObject;
                        scrData->scriptIndex = obj->scriptData.count;

                        /////// Function prototypes
                        uint32_t SetScriptData(EngineScript * script, ScriptData * output, uint8_t scopeLevel);
                        uint16_t UnpackErrorMessage(uint32_t error);
                        /////////////////////////

                        uint32_t errorNum = SetScriptData(
                            &scripts[script],
                            scrData,
                            0);

                        debugPrint("set scr data");

                        for (int t = 0; t < scrData->variables.count; t++)
                        {
                            debugPrintf("set scr variable out: %s\n", ((EngineVar*)ListGetIndex(&scrData->variables, t))->name);
                        }

                        if (errorNum != 0)
                        {
                            errors[errorCount++] = errorNum;

                            void UI_PrintError(ScriptData * scrData, uint32_t error);

                            UI_PrintError(scrData, errorNum);
                        }

                    }
                    refresh = true;
                    sleep_ms(160);
                }

                if (buttons[SCRIPT_DELETE_BUTTON].isPressed) {
                    char buffer[strlen("Are you sure you\nwant to delete\n") + 20];

                    EngineObject* object = currentScene->objects[currentObject];

                    if (modulePage == 0) {
                        strcpy(buffer, "Base can not be deleted");
                        Modal(buffer, false);
                    }
                    else if (modulePage == 1 && object->packages[0]) {
                        strcpy(buffer, "Are you sure you\nwant to delete\nCollider");
                        if (Modal(buffer, true)) {
                            while (object->colliderRects.count > 0)
                            {
                                int* ID = (int*)PopList(&object->colliderRects);
                                DeleteRectFromAll(*ID);
                                free(ID);
                            }
                            object->packages[0] = false;
                        }
                    }
                    else {
                        ScriptData* scrData = (ScriptData*)ListGetIndex(&object->scriptData, modulePage - objectPackageCount);
                        sprintf("Are you sure you\nwant to delete\n%s", scrData->script->name);
                        if (Modal(buffer, true)) {
                            FreeScriptData(scrData, true);
                            free((ScriptData*)DeleteListElement(&object->scriptData, modulePage - objectPackageCount));
                        }
                    }

                    refresh = true;
                    sleep_ms(160);
                }




                uint8_t scriptCount = currentScene->objects[currentObject]->scriptData.count + 1;
                if (currentScene->objects[currentObject]->packages[0])
                {
                    scriptCount++;
                }
                if (currentScene->objects[currentObject]->packages[1])
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
        if (currentMode == OPTIONS_MODE && buttons[RENDER_MODE_BUTTON].isPressed)
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
            ClearConsole();
            RunProgram();
            refresh = true;
        }

        // exit
        if (buttons[EXIT_EDITOR_UI].onPressDown)
        {

            ClearConsole();

            debugPrint("clear console");

            LoadingScreen();

            free(modifyVariable);

            FlushBuffer();

            char* fileName = malloc(strlen(FILE_IDENTIFIER) + strlen(currentProjectName) + strlen(currentScene->name) + 4);
            sprintf(fileName, "%s`%s`%s",
                FILE_IDENTIFIER,
                currentProjectName,
                currentScene->name);

            File* scene = GetFile(fileName);

            if (scene->startBlock == 0)
            {
                scene = CreateFile(fileName, strlen(fileName), MAX_OBJECTS * 2 + 2);
            }

            debugPrint("starting object save");

            int startBlock = scene->startBlock;
            char buffer[4];
            sprintf(buffer, "%d", currentScene->objectCount);
            WriteFile(scene, buffer, strlen(buffer) + 1);
            scene->startBlock -= 1;
            for (int obj = 0; obj < currentScene->objectCount; obj++)
            {
                char* serializedObject = SerializeObject(currentScene->objects[obj]);
                debugPrint("obj serialized");
                WriteFile(scene, serializedObject, strlen(serializedObject) + 1);
                scene->startBlock -= 2;
                free(serializedObject);
            }

            free(scene);
            free(fileName);

            DisengageSD();

            debugPrint("saved scene to project");

            SaveProject(program);


            EngineObject* testobj = currentScene->objects[1];
            if (testobj != NULL && testobj->scriptData.count > 0) {
                debugPrintf("script ID before decomp: %d\n", ((ScriptData*)ListGetIndex(&testobj->scriptData, 0))->script->ID);
            }

            DecompileScene();

            debugPrint("decomp scene");
            debug_sleep(100);

            return;
        }

        if (currentMode == OBJECT_MODE)
        {
            int highlighedObject = -1;
            for (int i = 0; i < currentScene->objectCount; i++)
            {
                if (buttons[i + OBJECT_BUTTONS].isFocused) {
                    highlighedObject = i;
                }
                if (buttons[i + OBJECT_BUTTONS].isPressed)
                {
                    currentObject = i;
                    break;
                }
            }

            if (buttons[ADD_THING_BUTTON].isPressed)
            {
                char* name = NewObject();
                debugPrint("returned");
                if (name != NULL)
                {
                    debugPrint("adding obj");
                    currentScene->objects[currentScene->objectCount] = ObjectConstructor(currentScene->objectCount, name, strlen(name));
                    currentScene->objectCount++;
                    free(name);
                }
                else
                {
                    debugPrint("is null");
                }
                sleep_ms(120);
                refresh = true;
            }

            if (highlighedObject < currentScene->objectCount && highlighedObject >= 0) {
                if (objectListOffset < currentScene->objectCount - 5 && highlighedObject - objectListOffset > 4) {
                    refresh = true;
                    objectListOffset++;
                }
                if (objectListOffset > 0 && highlighedObject - objectListOffset < 1) {
                    refresh = true;
                    objectListOffset--;
                }
            }
        }
    }
}


void EditScene(int sceneIndex)
{
    currentScene = &scenes[sceneIndex];

    ClearConsole();

    RecompileScene();

    ManageSceneUI();
}

char* CreateNewScene()
{
    bool refresh = true;
    bool modifyName = false;
    uint8_t option = 0;
    char* name = (char*)malloc((MAX_NAME_LENGTH + 1) * sizeof(char));
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
                    if (option == 1 && strlen(name) > 0)
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
            if (GetButton() == BUTTON_J && sceneCount > 0)
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
                    char* sceneName = CreateNewScene();

                    if (sceneName != NULL)
                    {
                        debugPrint("creating scene");
                        uint8_t index = CreateScene(sceneName, strlen(sceneName));
                        debugPrint("created scene");
                        scenes[index].objects[scenes[index].objectCount] = ObjectConstructor(scenes[index].objectCount, "Camera", strlen("Camera"));
                        debugPrint("created camera");
                        GetObjectDataByName(scenes[index].objects[scenes[index].objectCount], "scale")->currentType = TYPE_INT;
                        GetObjectDataByName(scenes[index].objects[scenes[index].objectCount], "scale")->data.i = 2;
                        GetObjectDataByName(scenes[index].objects[scenes[index].objectCount], "sprite")->data.i = CAMERA_SPRITE;
                        scenes[index].objectCount++;
                        debugPrint("set cam data");



                        scenes[index].objects[scenes[index].objectCount] = ObjectConstructor(scenes[index].objectCount, "Object1", strlen("Object1"));
                        scenes[index].objectCount++;
                        sceneIndex = index;
                        debugPrint("created obj1");

                        EditScene(index);
                    }
                    free(sceneName);
                }
                else
                {
                    int index = SelectScene();
                    sceneIndex = index;
                    debugPrintf("sceneNumber: %d\n", index);
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



#define RENDER_POSITION 0
#define RENDER_SPRITE 1
#define RENDER_SCALE_TYPE 2
#define RENDER_SCALE 3
#define RENDER_ANGLE 4

#define MULTI_STEP_PROGRAM 0

uint8_t cameraScale = 0;
int cameraX = 0;
int cameraY = 0;

typedef struct CoreState {
    bool busy;
    bool partitioning;
    bool collisionPartitioned;

    bool flagCalled;
    bool startRendering;
    bool checkInit;
    bool offloadCollision;

    bool exit;
} CoreState;

CoreState core0State;
CoreState core1State;

critical_section_t  collisionSection;

void InitCollisionLock() {
    critical_section_init(&collisionSection);
}

void GameRenderThread()
{
    core0State.checkInit = true;



    while (1)
    {
        sleep_ms(1);
        debugPrintf("............ Core 1 loop");


        if (core1State.exit)
            break;

        if (!core1State.flagCalled) {
            core1State.busy = false;
            continue;
        }

        core1State.flagCalled = false;
        core1State.busy = true;

        debugPrint("........... Core 1 recieved flag ");

        if (core1State.startRendering)
        {
            core1State.startRendering = false;

            debugPrint("............ Core 1 start render...");
            SmartClear();
            for (int i = 0; i < currentScene->objectCount; i++)
            {
                EngineObject* object = currentScene->objects[i];

                if (object->renderData[RENDER_SPRITE].i < 0)
                {
                    continue;
                }
                Vector2 scale;
                if (object->renderData[RENDER_SCALE_TYPE].i == TYPE_INT)
                {
                    scale.x = object->renderData[RENDER_SCALE].i;
                    scale.y = object->renderData[RENDER_SCALE].i;
                }
                else
                {
                    scale.x = object->renderData[RENDER_SCALE].XY.x;
                    scale.y = object->renderData[RENDER_SCALE].XY.y;
                }

                // debugPrintf("Scale: (%f,%f)\n", scale.x, scale.y);
                // debugPrintf("Position: (%f,%f)\n", GetObjectDataByName(object, "position")->data.XY.x, GetObjectDataByName(object, "position")->data.XY.y);
                // debugPrintf("Sprite: %d\n", GetObjectDataByName(object, "sprite")->data.i);
                DrawSpriteCentered(
                    object->renderData[RENDER_SPRITE].i,
                    80 + cameraScale * object->renderData[RENDER_POSITION].XY.x - cameraX,
                    64 + cameraScale * -object->renderData[RENDER_POSITION].XY.y + cameraY,
                    cameraScale * scale.x,
                    cameraScale * scale.y,
                    (int)object->renderData[RENDER_ANGLE].f);
            }
            SmartShow();

            debugPrint("........ CORE 1 FINISHED RENDER");




            critical_section_enter_blocking(&collisionSection);
            core1State.startRendering = false;

            // lock the other core to prevent race conditions

            if (!core0State.collisionPartitioned && !core0State.partitioning) {


                debugPrint("........ CORE 1 TAKING OVER COLLISION PARTITIONMENT");
                core1State.partitioning = true;
                //allow other core to continue after setting flag
                debugPrint("........ CORE 1 releasing other core");
                critical_section_exit(&collisionSection);

                PartitionColliders();
                core1State.collisionPartitioned = true;
            }
            else {
                debugPrint("........ CORE 1 releasing other core");
                critical_section_exit(&collisionSection);
            }

        }
        if (core1State.offloadCollision) {

            debugPrint("............ Core 1 executing offloaded collisions");
            ColliderStep(COLLIDER_SECOND);
            core1State.offloadCollision = false;
        }
    }

    debugPrint("......... CORE 1 EXITED");



    while (1)
        tight_loop_contents();
}

uint32_t RunProgram()
{
    debugPrint("run prog");


    EngineObject* camera = NULL;

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

    debugPrint("got cam");

    for (int obj = 0; obj < currentScene->objectCount; obj++)
    {
        for (int scr = 0; scr < currentScene->objects[obj]->scriptData.count; scr++)
        {
            ScriptData* scrData = (ScriptData*)ListGetIndex(&currentScene->objects[obj]->scriptData, scr);


            debugPrintf("backup: obj:%d, scr:%d\n", obj, scr);
            debugPrintf("scrdata lines: %d\n", scrData->lineCount);
            debugPrint("got scr data");
            if (scrData == NULL)
            {
                debugPrint("null scr data");
            }

            debugPrint("freed");
            if (scrData->variables.count > 0)
                scrData->backupData = malloc(sizeof(EngineVar) * scrData->variables.count);
            debugPrint("malloced");
            scrData->backupVarCount = scrData->variables.count;
            for (int var = 0; var < scrData->variables.count; var++)
            {
                EngineVar* varPointer = (EngineVar*)ListGetIndex(&scrData->variables, var);
                debugPrintf("backup: %d\n", var);

                scrData->backupData[var].currentType = varPointer->currentType;
                scrData->backupData[var].data = varPointer->data;


                InitializeList(&scrData->backupData[var].listData);

                CpyList(&scrData->backupData[var].listData, &varPointer->listData, sizeof(VariableUnion));

            }

            debugPrint("backed up!");
        }
        GeneralListNode* currentNode = currentScene->objects[obj]->objectData.firstElement;
        while (currentNode != NULL) {
            VariableUnion* backup = malloc(sizeof(VariableUnion));
            *backup = ((EngineVar*)currentNode->content)->data;
            PushList(&currentScene->objects[obj]->backupData, backup);
            currentNode = currentNode->next;
        }
        debugPrintf("object %d fin\n", obj);
    }
    
    debugPrint("fin");

    Clear();
    SmartClear();
    SmartShowAll();



    multicore_reset_core1();


    debugPrint("reset core");

    memset(&core0State, 0, sizeof(CoreState));
    memset(&core1State, 0, sizeof(CoreState));

    debugPrint("reset data");

    multicore_launch_core1(GameRenderThread);

    debugPrint("launch core");
    sleep_ms(50);
    if (core0State.checkInit)
    {
        debugPrint("............ Core 0 got confirmation!");
    }
    else
    {
        debugPrint("............ Core 0 incorrect confirmation");

        core1State.flagCalled = true;

        return CreateError(RENDERER, COULD_NOT_START_CORE_1, 0);
    }


    core0State.busy = true;

    while (1)
    {





        // debugPrint("///////////////////////////////////////////////////////////////////// APPLY OBJECT PHYSICS");

        for (int i = 0; i < currentScene->objectCount; i++)
        {

            EngineObject* object = currentScene->objects[i];

            debugPrint("phys step");

            PhysicsStep(object);
            object->didCollide = false;

            GetObjectDataByName(object, "position")->data.XY.x += GetObjectDataByName(object, "velocity")->data.XY.x;
            GetObjectDataByName(object, "position")->data.XY.y += GetObjectDataByName(object, "velocity")->data.XY.y;
            debugPrintf("added velocity to position, %f,%d\n",
                GetObjectDataByName(object, "velocity")->data.XY.x,
                GetObjectDataByName(object, "velocity")->data.XY.y);
            float multiplier = ((float)(100 - (abs(GetObjectDataByName(object, "drag")->data.i) % 101))) / 100;

            GetObjectDataByName(object, "velocity")->data.XY.x *= multiplier;
            GetObjectDataByName(object, "velocity")->data.XY.y *= multiplier;
            // debugPrint("modified velocity");


            uint8_t scrCount = object->scriptData.count;

            for (int scr = 0; scr < scrCount; scr++)
            {
                char funcName[32] = "main";
                ScriptData* scrData = (ScriptData*)ListGetIndex(&object->scriptData, scr);
                JumpToFunction(scrData, funcName);
            }
        }



        // debugPrint("showed");
        // debugPrintf("count: %d\n", currentScene->objectCount);

        // debugPrint("/////////////////////////////////////////////////////////////////////EXECUTE SCRIPTS");

        ExecuteInstructionStack();

        critical_section_enter_blocking(&collisionSection);

        //lock the other core during critical section

        if (core1State.partitioning || core1State.collisionPartitioned) {
            critical_section_exit(&collisionSection);
            debugPrint("............ Core 0 waiting partition to finish");
            while (!core1State.collisionPartitioned) sleep_ms(1);
            debugPrint("............ Core 0 saw collision partitioned");
        }
        else {
            core0State.partitioning = true;
            critical_section_exit(&collisionSection);
            debugPrint("............ Core 0 partitioning colliders");
            PartitionColliders();
            core0State.collisionPartitioned = true;
        }

        // debugPrint("/////////////////////////////////////////////////////////////////// DO COLLISION");


        if (!core1State.busy) {
            debugPrint("............ Core 0 offloading collisions");

            core1State.offloadCollision = true;
            core1State.flagCalled = true;

            while (!core1State.busy) {
                sleep_ms(1);
                debugPrint("............ Core 0 waiting for core 1 to start");
            }

            ColliderStep(COLLIDER_FIRST);



            while (core1State.busy) {
                sleep_ms(1);
                debugPrint("............ Core 0 finished collisions, awaiting core 1");
            }
            debugPrint("............ Core 0 got core 1 finished");
            debug_sleep(100);

        }
        else {
            debugPrint("............ Core 0 doing all collision");
            ColliderStep(COLLIDER_ALL);
        }


        FreeCells();
        while (resolvedCollisions.count > 0) {
            CollisionChecked* check = PopList(&resolvedCollisions);
            free(check);
        }

        // debugPrint("/////////////////////////////////////////////////////////////////////DRAW SPRITES");

        if (!core1State.busy || MULTI_STEP_PROGRAM == 0)
        {
            debugPrint("................. Core 0 waiting for core ready");
            while (core1State.busy) sleep_ms(1);
            debugPrint("................. Core 0 saving critical object data");

            ////////////////////////////////////////////////////////////////////// GET CAMERA DATA

            cameraScale = GetObjectDataByName(camera, "scale")->data.i;

            cameraX = GetObjectDataByName(camera, "position")->data.XY.x;
            cameraY = GetObjectDataByName(camera, "position")->data.XY.y;

            for (int i = 0; i < currentScene->objectCount; i++)
            {
                EngineObject* object = currentScene->objects[i];

                object->renderData[RENDER_POSITION].XY.x = GetObjectDataByName(object, "position")->data.XY.x;
                object->renderData[RENDER_POSITION].XY.y = GetObjectDataByName(object, "position")->data.XY.y;

                object->renderData[RENDER_SPRITE].i = GetObjectDataByName(object, "sprite")->data.i;

                if (GetObjectDataByName(object, "scale")->currentType == TYPE_INT)
                {
                    object->renderData[RENDER_SCALE_TYPE].i = TYPE_INT;

                    object->renderData[RENDER_SCALE].i = GetObjectDataByName(object, "scale")->data.i;
                }
                else
                {
                    object->renderData[RENDER_SCALE_TYPE].i = TYPE_VECTOR;

                    object->renderData[RENDER_SCALE].XY.x = GetObjectDataByName(object, "scale")->data.XY.x;
                    object->renderData[RENDER_SCALE].XY.y = GetObjectDataByName(object, "scale")->data.XY.y;
                }

                object->renderData[RENDER_ANGLE].f = GetObjectDataByName(object, "angle")->data.f;
            }
            debugPrint("............ Core 0 starting renderer");

            core1State.startRendering = true;
            core1State.flagCalled = true;

        }
        else
        {
            debugPrint(".... MULTI STEPPING PROGRAM");
        }

        // debugPrint("/////////////////////////////////////////////////////////////////////// EXIT");

        if (gpio_get(BUTTON_S) == 0 && gpio_get(BUTTON_K) == 0)
        {
            if (millis() - exitHoldTime > 2000)
            {
                debugPrint("exiting");
                core1State.flagCalled = false;
                core1State.exit = true;

                while (core1State.busy)sleep_ms(1);
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
        EngineObject* object = currentScene->objects[obj];
        for (int scr = 0; scr < object->scriptData.count; scr++)
        {

            ScriptData* scrData = (ScriptData*)ListGetIndex(&object->scriptData, scr);

            ResetScriptData(scrData);

            scrData->variables.count = scrData->backupVarCount;
            for (int var = 0; var < scrData->variables.count; var++)
            {
                debugPrintf("get var index: %d\n", var);
                EngineVar* varPointer = (EngineVar*)ListGetIndex(&scrData->variables, var);
                debugPrintf("var name: %s\n", varPointer->name);
                varPointer->currentType = scrData->backupData[var].currentType;
                varPointer->data = scrData->backupData[var].data;
                debugPrint("set data");

                TransferList(&varPointer->listData, &scrData->backupData[var].listData);
                debugPrint("transfered list");
            }
            if (scrData->variables.count > 0)
                free(scrData->backupData);
            debugPrint("freedbackup");
        }

        GeneralListNode* backupNode = object->backupData.firstElement;
        GeneralListNode* dataNode = object->objectData.firstElement;
        while (backupNode != NULL && dataNode != NULL) {
            ((EngineVar*)dataNode->content)->data = *((VariableUnion*)backupNode->content);
            backupNode = backupNode->next;
            dataNode = dataNode->next;
        }

        while (object->backupData.count > 0) {
            free(PopList(&object->backupData));
        }

    }

    GeneralListNode* currentNode = allColliders.firstElement;

    while (currentNode != NULL) {
        ((Rect*)currentNode->content)->didCollide = false;
        ((Rect*)currentNode->content)->couldExit = false;

        currentNode = currentNode->next;
    }

    while (instructionStack.count > 0)
    {
        free(PopList(&instructionStack));
    }

    core0State.collisionPartitioned = false;
    core0State.partitioning = false;
    core1State.collisionPartitioned = false;
    core1State.partitioning = false;

    gpio_put(LEFT_LIGHT, 0);
    gpio_put(RIGHT_LIGHT, 0);

    debugPrint("exit");

    return 0;
}

#pragma endregion

#endif