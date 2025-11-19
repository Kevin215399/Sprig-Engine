#include <stdio.h>
#include "pico/stdlib.h"

#include "C:\Users\kphoh\Documents\RP pico\lib\TFT.h"

#include "Keyboard.h"

#include "InputManager.h"

#include "EngineStructs.h"
#include <string.h>




#define CREATE_PROGRAM 0
#define OPEN_PROGRAM 1

int OpenMenu()
{
    bool refresh = true;
    uint8_t option = CREATE_PROGRAM;
    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("Menu", 4, 1, 1, 3, WHITE, TRANSPARENT);

            WriteWord("Create Program", strlen("Create Program"), (option == 0) ? 5 : 0, 40, 1, (option == 0) ? WHITE : RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("Load Program", strlen("Load Program"), (option == 1) ? 5 : 0, 50, 1, (option == 1) ? WHITE : RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("DELETE ALL", strlen("DELETE ALL"), (option == 2) ? 5 : 0, 60, 1, (option == 2) ? WHITE : RGBTo16(100, 100, 100), TRANSPARENT);

            WriteWord("W/S - Navigate", strlen("W/S - Navigate"), 1, 109, 1, RGBTo16(100, 100, 100), TRANSPARENT);
            WriteWord("J - Select", strlen("J - Select"), 1, 119, 1, RGBTo16(100, 100, 100), TRANSPARENT);

            while(GetButton() != 0) sleep_ms(10);
        }

        if (GetButton() != 0)
        {
            if (GetButton() == BUTTON_W && option > 0)
            {
                option--;
            }
            if (GetButton() == BUTTON_S && option <2)
            {
                option++;
            }
            if (GetButton() == BUTTON_J)
            {
                return option;
            }
            refresh = true;
        }
    }
}