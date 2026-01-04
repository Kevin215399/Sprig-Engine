#ifndef KEYBINDS
#define KEYBINDS

#include <stdio.h>
#include "pico/stdlib.h"
#include "TFT.h"
#include <string.h>
#include <math.h>

#define KB_TYPING 0
#define KB_SCRIPT_VIEW 1
#define KB_SCRIPT_EDIT 2
#define KB_SCRIPT_SELECTED 3

char *KEYBIND_TEXT[] = {
    "J-Select K-Delete L-Exit",
    "J-Type I-Select L-Exit\nK-Paste",
    "J-Select K-Delete L-ExitI-Return",
    "I-Copy K-Delete L-Cancel"};

void PrintKeybinds(int yOffset, int keybindMode, uint16_t color)
{
    int length = strlen(KEYBIND_TEXT[keybindMode]);
    WriteWord(KEYBIND_TEXT[keybindMode],
              length,
              0,
              120 - yOffset - floor(length / 26) * 8,
              1,
              color,
              TRANSPARENT);
}

#endif