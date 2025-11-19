#include <stdio.h>
#include "pico/stdlib.h"

#include "SaveSystem.h"
#include "C:\Users\kphoh\Documents\RP pico\lib\TFT.h"

#include "Keyboard.h"

#include "InputManager.h"

#include "EngineStructs.h"
#include <string.h>

#include "pico/time.h"

unsigned long millis()
{
    return to_ms_since_boot(get_absolute_time());
}

char *OpenCreationMenu()
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

    bool fileAlreadyExists = false;

    bool drawCaret = true;

    while (1)
    {
        if (refresh)
        {
            refresh = false;
            Clear();
            WriteWord("Create", 6, 1, 1, 2, RGBTo16(255, 200, 0), TRANSPARENT);

            Rectangle(RGBTo16(255, 200, 0), 0, 18, 160, 1);

            WriteWord("Project Name", strlen("Project Name"), 2, 31, 1, WHITE, TRANSPARENT);

            Rectangle((option == 0) ? WHITE : RGBTo16(150, 150, 150), 2, 40, 156, 11);
            Rectangle((option == 0) ? RGBTo16(0, 0, 100) : BLACK, 3, 41, 154, 9);

            WriteWord(name, strlen(name), 4, 42, 1, WHITE, TRANSPARENT);

            if (fileAlreadyExists)
            {
                WriteWord("File name exists", strlen("File name exists"), 2, 52, 1, RED, TRANSPARENT);
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
                WriteWord(">Done", strlen(">Done"), (option == 1) ? 10 : 2, 70, 2, (option == 1) ? GREEN : RGBTo16(0, 100, 0), TRANSPARENT);

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
                        if (FileExists(name))
                        {
                            fileAlreadyExists = true;
                        }
                        else
                        {
                            return name;
                        }
                    }
                    else
                    {
                        fileAlreadyExists = false;
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