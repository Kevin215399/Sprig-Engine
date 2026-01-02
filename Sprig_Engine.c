#include <stdio.h>
#include "pico/stdlib.h"

#include "SDSimple.h"
#include "TFT.h"

#include "Keyboard.h"

#include "InputManager.h"

#include "EngineStructs.h"

#include <string.h>

File *program;

#include "Menu.h"
#include "ProgramCreate.h"
#include "ProgramSelect.h"
#include "Editor.h"
#include "Extras/FireWork.h"


#include "Interpreter.h"

void RenderTest()
{
    for (int x = 0; x < 360; x += 1)
    {
        SmartClear();
        SmartRect(RED, 80 + cos(x * M_PI / 180) * 20, 64 + sin(x * M_PI / 180) * 20, 20, 20);
        SmartRect(BLUE, 80 + cos((x + 120) * M_PI / 180) * 20, 64 + sin((x + 120) * M_PI / 180) * 20, 20, 20);
        SmartRect(YELLOW, 80 + cos((x + 240) * M_PI / 180) * 20, 64 + sin((x + 240) * M_PI / 180) * 20, 20, 20);
        SmartShow();
    }
    renderMode = FAST_BUT_FLICKER;
    SmartClear();
    sleep_ms(1000);
}

int main()
{
    stdio_init_all();


    InitializeButtons();
    InitializeKeyboard();

    SetupColors();
    CreateNullStructs();

    gpio_init(SD_CS);
    gpio_set_dir(SD_CS, GPIO_OUT);
    gpio_put(SD_CS, 1);

    InitializeTFT();

    spi_set_baudrate(SPI_PORT, SDInitSpeed);

    // FlushBuffer();

    InitializeSDCard();

    DisengageSD();

    spi_set_baudrate(SPI_PORT, 32 * 1000 * 1000);

    Clear();

    // sleep_ms(5000);

    //sleep_ms(4000);

    InitializeLights();

    //sleep_ms(1000);
    //FireworkShow();

    editorView = DEBUG_VIEW;
    UI_ClearDebug();

    /*EngineScript *script = ScriptConstructor(0, "script1",
                                             "int x = 0; setPosition(Vector(cos(x*PI/180)*5,0));  x+=1;");

    // int x=0; while(x<8){ x+=1; if(x%2==0){ print(\"even\"); } if(x%2!=0){ print(\"odd\"); } }

    ScriptData *testData = ScriptDataConstructor(script);
    testData->linkedObject = NULL;

    uint32_t errorNum = SetScriptData(script, testData, 0);

    uint16_t error = UnpackErrorMessage(errorNum);

    printf("set script error: %s\n", error);

    if (errorNum == 0)
    {
        FreeString(&error);

        int iteration = 0;
        while (iteration < 1)
        {
            testData->currentLine = 0;

            while (testData->currentLine < testData->lineCount)
            {
                errorNum = ExecuteLine(script, testData);
                error = UnpackErrorMessage(errorNum);
                printf("Execute line result: %s\n", stringPool[error]);
                if (errorNum != 0)
                {
                    UI_PrintToScreen(stringPool[error], true);

                    break;
                }
                FreeString(&error);
            }
            printf("\n////////////////////////////// Iteration %d\n\n", iteration);
            iteration++;
        }
        FreeScriptData(testData, false);
    }
    else
    {
        UI_PrintToScreen(stringPool[error], true);
        FreeString(&error);
    }

    editorView = 0;

    sleep_ms(3000);*/

    /*ScriptData *output = ScriptDataConstructor(0, 0, NULL, 0);
    EngineScript *script = ScriptConstructor(0, "test", "{\nhi;\ntest()\n}");

    SplitScript(script,output);

    for(int i = 0; i < output->lineCount; i++){
        printf("%s\n",output->lines[i]);
    }


    uint8_t errorCode = CalculateBrackets(script, output);
    print("calculated brackets");
    if (errorCode == 0)
    {
        printf("BracketCount: %d\n", output->bracketPairs);
        for (int i = 0; i < output->bracketPairs; i++)
        {
            printf("Brack pair: (%d,%d)\n", output->brackets[i].start, output->brackets[i].end);
        }
    }
    else if (errorCode == 1)
    {
        print("error: parenthesis count incorrect\n");
    }
    else if (errorCode == 2)
    {
        print("error: parenthesis mismatch\n");
    }

    for(int i = 0; i < output->lineCount;i++){
        free(output->lines[i]);
    }
    free(output->lines);
    free(script);
    free(output->brackets);
    free(output);*/

    // SceneMenu();

    // sleep_ms(5000);

    while (1)
    {
        uint8_t menuOption = OpenMenu();

        if (menuOption == CREATE_PROGRAM)
        {
            char *projectName = OpenCreationMenu();
            if (projectName == NULL)
            {
                continue;
            }
            CreateProject(projectName);

            /*program = GetFile(projectName);

            if (program != NULL_FILE)
            {
                EditorMainScreen();
            }*/

            free(projectName);
        }
        else if (menuOption == OPEN_PROGRAM)
        {
            program = SelectProgram();
            if (program != NULL_FILE)
            {
                EditorMainScreen();
            }
        }
        else
        {
            FlushBuffer();
            SetupTable(true);
            sleep_ms(500);
            DisengageSD();
        }
    }

    print("end of program");
}