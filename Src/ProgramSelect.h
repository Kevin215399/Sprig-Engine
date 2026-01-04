#include <stdio.h>
#include "pico/stdlib.h"

#include "SaveSystem.h"
#include "TFT.h"

#include "Keyboard.h"

#include "InputManager.h"

#include "EngineStructs.h"
#include <string.h>

File *SelectProgram()
{
    bool refresh = true;
    uint8_t currentFile = 0;

    FlushBuffer();
    print("flushed");
    File **programs = GetAllPrograms();
    print("Got programs");
    uint8_t programCount = GetProgramCount(true);
    print("got program count");
    DisengageSD();

    uint8_t totalPages = (uint8_t)((float)programCount / 5) + 1; // floor and then add 1 basically ceil
    uint8_t page = 0;
    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("Load", 6, 1, 1, 2, RGBTo16(255, 200, 0), TRANSPARENT);
            Rectangle(RGBTo16(255, 200, 0), 0, 18, 160, 1);

            WriteWord("Projects", strlen("Projects"), 2, 23, 1, WHITE, TRANSPARENT);
            Rectangle(WHITE, 2, 32, 156, 63);
            Rectangle(RGBTo16(30, 30, 30), 3, 33, 154, 61);

            for (int i = page * 5; i < min((page + 1) * 5, programCount); i++)
            {
                uint8_t yPos = 36 + (i - page * 5) * 10;
                if (i == currentFile)
                {
                    Rectangle(GREEN, 4, yPos - 1, 152, 9);
                    WriteWord(programs[i]->name, strlen(programs[i]->name), 6, yPos, 1, BLACK, TRANSPARENT);
                }
                else
                {
                    WriteWord(programs[i]->name, strlen(programs[i]->name), 6, yPos, 1, WHITE, TRANSPARENT);
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

            if (GetButton() == BUTTON_S && currentFile < min((page + 1) * 5, programCount) - 1)
            {
                currentFile++;
            }
            if (GetButton() == BUTTON_W && currentFile > page * 5)
            {
                currentFile--;
            }
            if (GetButton() == BUTTON_D && page < totalPages)
            {
                page++;
                currentFile = page * 5;
            }
            if (GetButton() == BUTTON_A && page > 0)
            {
                page--;
                currentFile = page * 5;
            }
            if (GetButton() == BUTTON_J)
            {
                File *output = programs[currentFile];
                free(programs);
                return output;
            }
            if (GetButton() == BUTTON_L)
            {
                free(programs);
                return NULL_FILE;
            }
        }

        
    }
}