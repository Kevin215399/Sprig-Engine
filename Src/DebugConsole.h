// Saves console messages from the interpreter
#ifndef DEBUG_CONSOLE
#define DEBUG_CONSOLE

#include <stdio.h>
#include "pico/stdlib.h"

#include <string.h>

typedef struct ErrorMessage {
    bool used;
    uint32_t error;
    uint16_t message;
    ScriptData* scrData;
    bool isError;
} ErrorMessage;

#define CONSOLE_POOL 12

ErrorMessage console[CONSOLE_POOL];

void ShiftDebugConsole() {
    if (console[CONSOLE_POOL - 1].used) {
        if (!console[CONSOLE_POOL - 1].isError) {
            FreeString(&console[CONSOLE_POOL - 1].message);
            debugPrint("DEBUGGER CONSOLE cleared last");
        }
        console[CONSOLE_POOL - 1].used = false;
    }


    for (int i = CONSOLE_POOL - 1; i > 0; i--) {
        console[i].isError = console[i - 1].isError;
        if (console[i - 1].isError) {
            console[i].error = console[i - 1].error;
        }
        else {
            console[i].message = console[i - 1].message;
        }
        console[i].used = console[i - 1].used;
        console[i].scrData = console[i - 1].scrData;
    }
}

void UI_PrintError(ScriptData* scrData, uint32_t error)
{
    ShiftDebugConsole();

    console[0].error = error;
    console[0].isError = true;
    console[0].used = true;
    console[0].scrData = scrData;
    debugPrint("saved error");
}
void UI_PrintMessage(ScriptData* scrData, uint16_t message) {
    ShiftDebugConsole();

    console[0].message = message;
    console[0].isError = false;
    console[0].used = true;
    console[0].scrData = scrData;
    debugPrint("saved message");
}

//output is malloc, does not clear from console
char* TranslateConsoleMessage(uint8_t index) {
    debugPrint("DEBUGGER: translating message!!!!!!!");

    uint16_t UnpackErrorMessage(uint32_t error);


    uint8_t messageLength = 0;
    messageLength += 1;//parenthesis "("
    messageLength += strlen(console[index].scrData->linkedObject->name);//object name
    messageLength += 2;//parenthesis ")("
    messageLength += strlen(console[index].scrData->script->name);//script name
    messageLength += 3;//parenthesis and end "): "

    uint16_t mainContentPoolIndex = 0;
    if (console[index].isError) {
        mainContentPoolIndex = UnpackErrorMessage(console[index].error);
    }
    else {
        mainContentPoolIndex = console[index].message;
    }
    messageLength += strlen(stringPool[mainContentPoolIndex]);// message length
    messageLength += 1; // "\0"

    debugPrintf("length: %d\n", messageLength);
    debugPrintf("main content: %s\n", stringPool[mainContentPoolIndex]);

    char* output = malloc(messageLength);
    sprintf(output, "(%s)(%s): %s",
        console[index].scrData->linkedObject->name,
        console[index].scrData->script->name,
        stringPool[mainContentPoolIndex]
    );
    debugPrintf("final message: %s\n", output);

    return output;
}
void ClearConsole() {

    for (int i = 0; i < CONSOLE_POOL;i++) {
        if (console[i].used) {
            if (!console[i].isError) {
                FreeString(&console[i].message);
            }
        }
    }

    memset(console, 0, sizeof(console));
}
bool ConsoleHasError() {
    for (int i = 0; i < CONSOLE_POOL;i++) {
        if (console[i].used) {
            if (console[i].isError) {
                return true;
            }
        }
    }
    return false;
}


/*
bool UI_PrintToScreen(char* message, bool isError)
{

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
}*/


#endif