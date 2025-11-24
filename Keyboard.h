#ifndef KEYBOARD
#define KEYBOARD

#include <stdio.h>
#include "pico/stdlib.h"
#include "TFT.h"
#include "InputManager.h"

uint8_t keyboardX = 0;
uint8_t keyboardY = 0;

char keyboard[] = {
    '|',
    '~',
    '!',
    '@',
    '#',
    '$',
    '%',
    '^',
    '&',
    '*',
    '[',
    ']',
    '{',
    '}',
    '\n',
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    '-',
    '7',
    '8',
    '9',
    '\n',
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    '+',
    '=',
    '4',
    '5',
    '6',
    '\n',
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    '(',
    ')',
    '\\',
    '\"',
    '1',
    '2',
    '3',
    '\n',
    ' ',
    '\0',
    '\0',
    '\0',
    '\0',
    ',',
    '.',
    '?',
    '!',
    '/',
    '\'',
    '_',
    '0',
    UPPERCASE_SYB,
    '\n',
};

bool uppercase = false;

#define KEYBOARD_LINES 5

uint8_t maxX[KEYBOARD_LINES];

// Gathers line data for each row of the keyboard
void InitializeKeyboard()
{
    uint8_t currentLine = 0;

    for (int i = 0; i < sizeof(keyboard) / sizeof(char); i++)
    {
        if (keyboard[i] == '\0')
        {
            maxX[currentLine] += 1;
        }
        if (keyboard[i] == '\n')
        {
            maxX[currentLine] = 13 - maxX[currentLine];
            currentLine++;
        }
    }
}

void HandleKeyboardInputs()
{
    if (GetButton() == BUTTON_W && keyboardY > 0)
    {
        if (keyboardY == 4 && keyboardX >= 1)
        {
            keyboardX += 4;
        }
        keyboardY--;
    }
    if (GetButton() == BUTTON_S && keyboardY < 4)
    {
        if (keyboardY == 3)
        {
            if (keyboardX <= 4)
            {
                keyboardX = 0;
            }
            else
            {
                keyboardX -= 4;
            }
        }
        keyboardY++;
    }
    if (GetButton() == BUTTON_D && keyboardX < maxX[keyboardY])
    {
        keyboardX++;
    }
    if (keyboardX > maxX[keyboardY])
    {
        keyboardX = maxX[keyboardY] - 1;
    }
    if (GetButton() == BUTTON_A && keyboardX > 0)
    {
        keyboardX--;
    }
}

char PrintKeyboard(uint8_t cursX, uint8_t cursY, uint8_t offsetX, uint8_t offsetY)
{
    keyboard[73] = uppercase ? LOWERCASE_SYB : UPPERCASE_SYB;
    // Clear();
    /*uint8_t lines = 0;
    for (int i = 0; i < sizeof(keyboard) / sizeof(char); i++)
    {
        if (keyboard[i] == '\n')
            lines++;
    }

    uint8_t *spaces = (uint8_t *)malloc(lines * sizeof(uint8_t));
    for (int i = 0; i < lines; i++)
    {
        spaces[i] = 0;
    }
    lines = 0;
    for (int i = 0; i < sizeof(keyboard) / sizeof(char); i++)
    {
        if (keyboard[i] == '\0')
            spaces[lines] += 1;
        if (keyboard[i] == '\n')
            lines++;
    }*/

    int maxXPos = maxX[cursY];

    // free(spaces);

    if (cursX > maxXPos)
    {
        cursX = maxXPos - 1;
    }

    uint8_t x = 5;
    uint8_t y = 120 - KEYBOARD_LINES * 10;

    uint8_t row = 0;
    uint8_t col = 0;
    char selectedCharacter;

    for (int i = 0; i < sizeof(keyboard) / sizeof(char); i++)
    {

        if (keyboard[i] == '\0')
        {
            x += 10;
            continue;
        }
        if (keyboard[i] == '\n')
        {
            col = 0;
            row++;
            x = 5;
            y += 10;
        }
        else
        {
            if (col == cursX && row == cursY)
            {
                selectedCharacter = keyboard[i];
                if (keyboard[i] == ' ')
                {
                    Rectangle(WHITE, x + offsetX - 1, y + offsetY - 1, 50, 9);
                    WriteWord("SPACE(i)", 8, x + offsetX, y + offsetY, 1, BLACK, WHITE);
                    x += 10;
                }
                else
                {
                    uint8_t character = keyboard[i];
                    if (character >= 65 && character <= 90 && !uppercase)
                    {
                        character += 32;
                    }
                    Rectangle(WHITE, x + offsetX - 1, y + offsetY - 1, 7, 9);
                    WriteLetter(character, x + offsetX, y + offsetY, 1, BLACK, WHITE);
                    x += 10;
                }
            }
            else
            {
                if (keyboard[i] == ' ')
                {
                    WriteWord("SPACE(i)", 8, x + offsetX, y + offsetY, 1, WHITE, TRANSPARENT);
                    x += 10;
                }
                else
                {
                    uint8_t character = keyboard[i];
                    if (character >= 65 && character <= 90 && !uppercase)
                    {
                        character += 32;
                    }
                    WriteLetter(character, x + offsetX, y + offsetY, 1, WHITE, TRANSPARENT);
                    x += 10;
                }
            }
            col++;
        }
    }

    return selectedCharacter;
}

#endif