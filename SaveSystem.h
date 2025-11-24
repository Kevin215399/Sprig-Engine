#ifndef SAVE_SYSTEM
#define SAVE_SYSTEM

#include <stdio.h>
#include "pico/stdlib.h"

#include "SDSimple.h"
#include "TFT.h"

#include "EngineStructs.h"
#include <string.h>
#include <math.h>

char FILE_IDENTIFIER[] = "SPRIG-ENGINE1\n";

char *currentProjectName;

typedef struct
{
    char *array;
    uint8_t length;
} charArray;

void SoftReset()
{
    gpio_put(SD_CS, 0);
    uint8_t cmd0[] = {0x40, 0, 0, 0, 0, 0x95};
    spi_write_blocking(spi0, cmd0, 6);
    print("soft reset");
    uint8_t response = 0;
    do
    {
        spi_read_blocking(SPI_PORT, 0xFF, &response, 1);
        printf("Reset response: %u", response);
    } while (response != 0x01);
    print("reseted");
    gpio_put(SD_CS, 1);
}
void FlushBuffer()
{
    spi_set_baudrate(SPI_PORT, SDNormalSpeed);
    gpio_put(TFT_CS, 1);
    gpio_put(PIN_DC, 1);
    gpio_put(SD_CS, 1);
    // for (int i = 0; i < 10; i++)
    spi_write_blocking(SPI_PORT, (const uint8_t[]){0xFF}, 1);
    sleep_ms(200);

    // SoftReset();
}
void DisengageSD()
{
    gpio_put(SD_CS, 1);
    sleep_ms(200);
    spi_set_baudrate(SPI_PORT, 32 * 1000 * 1000);
}

bool FileExists(char *name)
{
    FlushBuffer();
    File *result = GetFile(name);
    bool exists = (result->startBlock != 0);
    if (exists)
    {
        free(result);
        print("file exists");
    }
    else
    {
        print("file does not exist");
    }
    DisengageSD();
    return exists;
}

void CreateProject(char *name)
{
    FlushBuffer();
    printf("Create project name len: %u\n", strlen(name));
    File *newFile = CreateFile(name, strlen(name), 40);
    print("allocated memory");
    char buffer[512];

    snprintf(buffer, sizeof(buffer), "%s|%d|%d|%d", FILE_IDENTIFIER, spriteCount, scriptCount, sceneCount);

    WriteFile(newFile, buffer, 512);

    print("wrote to project");
    free(newFile);
    DisengageSD();
}

bool FileFormattedCorrect(char *name)
{
    // FlushBuffer();
    print("FileFormatCheck");
    File *file = GetFile(name);
    print("FileFormatCheck: got file");

    if (file->startBlock == 0)
    {
        return false;
    }

    uint8_t *data = ReadBlock(file->startBlock);
    print("FileFormatCheck: got file block");

    uint8_t blocksOccupied = file->startBlock - file->endBlock + 1;

    bool validIdentifier = true;
    for (int i = 0; i < strlen(FILE_IDENTIFIER); i++)
    {
        if (FILE_IDENTIFIER[i] != data[i])
        {
            validIdentifier = false;
            break;
        }
    }
    print("FileFormatCheck: checked identifier");
    // DisengageSD();
    free(data);
    free(file);

    print("FileFormatCheck: freed data");

    return validIdentifier;
}

uint8_t GetProgramCount(bool flush)
{
    if (flush)
        FlushBuffer();
    print("GetProgramCount");
    uint8_t fileCount = GetFileCount();
    printf("GetProgramCount: got count %u\n", fileCount);
    uint8_t programCount = 0;
    for (int i = 0; i < fileCount; i++)
    {
        File *f = GetFileByIndex(i);
        if (FileFormattedCorrect(f->name))
        {
            programCount++;
        }
        free(f);
        print("GetProgramCount: checked file");
    }
    printf("Found %u programs\n", programCount);
    return programCount;
}
// uint8_t requestAmount, uint8_t requestOffset
File **GetAllPrograms()
{
    FlushBuffer();
    uint8_t programCount = GetProgramCount(false);
    printf("Allocating memory for %u programs\n", programCount);

    // FlushBuffer();
    File **programs = (File **)malloc(sizeof(File *) * programCount);

    // FlushBuffer();
    uint8_t fileCount = GetFileCount();
    printf("%u Files counted\n", fileCount);

    if (fileCount > 30)
    {
        print("TESTING HARDCOE STOP");
        return programs;
    }

    uint8_t programIndex = 0;

    for (int i = 0; i < fileCount; i++)
    {
        File *file = GetFileByIndex(i);
        if (FileFormattedCorrect(file->name))
        {
            programs[programIndex] = FileInit(file->name, file->nameLength, file->startBlock, file->endBlock);
            programIndex++;
        }
        free(file);
    }
    DisengageSD();
    return programs;
}

int IntLength(int value)
{
    printf("int length val %d\n", value);
    int numberLength = 1;
    if (abs(value) != 0)
    {
        numberLength = (int)(log10(abs(value))) + 1;
    }
    if (abs(value) != value)
    {
        numberLength += 1;
    }
    printf("int length %d\n", numberLength);
    return numberLength;
}
uint8_t GetIntDigit(int value, int place)
{
    place = IntLength(value) - place - 1;

    for (int i = 0; i < place; i++)
    {
        value /= 10;
    }
    value %= 10;

    return value;
}

char *SpriteToText(uint8_t spriteIndex)
{
    int charLen = 1 + (SPRITE_HEIGHT * SPRITE_WIDTH * 2);

    char *text = (char *)malloc(sizeof(char) * charLen);

    text[0] = (char)sprites[spriteIndex].ID;

    for (int y = 0; y < SPRITE_HEIGHT; y++)
    {
        for (int x = 0; x < SPRITE_WIDTH; x++)
        {

            text[(x + y * SPRITE_WIDTH) * 2 + 1] = (sprites[spriteIndex].sprite[x][y] & 0xFF00) >> 8;
            text[(x + y * SPRITE_WIDTH) * 2 + 2] = (sprites[spriteIndex].sprite[x][y] & 0xFF);
        }
    }

    return text;
}

void TextToSprite(char *text)
{
    uint8_t index = text[0];
    sprites[index].ID = index;
    printf("New Sprite to text: %d\n", text[0]);
    for (int y = 0; y < SPRITE_HEIGHT; y++)
    {
        for (int x = 0; x < SPRITE_WIDTH; x++)
        {
            printf(" %d %d", ((text[(x + y * SPRITE_WIDTH) * 2 + 1])), (text[(x + y * SPRITE_WIDTH) * 2 + 2]));
            sprites[index].sprite[x][y] = ((text[(x + y * SPRITE_WIDTH) * 2 + 1]) << 8) | (text[(x + y * SPRITE_WIDTH) * 2 + 2]);
        }
    }
}

void SaveProject(File *file)
{
    FlushBuffer();

    char buffer[512];
    for (int i = 0; i < 512; i++)
    {
        buffer[i] = 0;
    }

    snprintf(buffer, sizeof(buffer), "%s|%d|%d|%d", FILE_IDENTIFIER, spriteCount, scriptCount, sceneCount);

    WriteBlock(file->startBlock, buffer);

    for (int i = 0; i < 512; i++)
    {
        buffer[i] = 0;
    }

    int bufferIndex = 0;
    for (int i = 0; i < spriteCount; i++)
    {
        if (i != 0 && i % 3 == 0)
        {
            printf("\n%d blcok num\n", file->startBlock - i / 3);
            WriteBlock(file->startBlock - i / 3, buffer);
            bufferIndex = 0;
        }
        char *sprite = SpriteToText(i);
        print("block\n");
        for (int s = 0; s < 1 + (SPRITE_HEIGHT * SPRITE_WIDTH * 2); s++)
        {
            // printf(" ", sprite[s]);
            buffer[bufferIndex++] = sprite[s];
            printf("%d ", buffer[bufferIndex - 1]);
        }
        print("\n");
        free(sprite);
    }

    int blockChange = 0;
    if (bufferIndex > 0)
    {
        printf("\n%d blcok num\n", file->startBlock - (spriteCount - 1) / 3 - 1);
        WriteBlock(file->startBlock - (spriteCount - 1) / 3 - 1, buffer);

        blockChange = (spriteCount - 1) / 3 + 2;
    }
    else
    {
        blockChange = (spriteCount - 1) / 3 + 1;
    }

    file->startBlock -= blockChange;

    int scriptNameLength = 1;
    for (int i = 0; i < scriptCount; i++)
    {
        scriptNameLength += strlen(scripts[i].name) + 1;
    }

    char *scriptNames = malloc(scriptNameLength);

    int index = 0;
    for (int i = 0; i < scriptCount; i++)
    {
        printf("Save script: %s, index: %d\n", scripts[i].name, i);
        strncpy(scriptNames + index, scripts[i].name, strlen(scripts[i].name));

        scriptNames[index + strlen(scripts[i].name)] = '\n';

        index += strlen(scripts[i].name) + 1;
    }

    scriptNames[scriptNameLength - 1] = '\0';

    WriteFile(file, scriptNames, scriptNameLength);

    print("saved scripts to project");

    free(scriptNames);

    file->startBlock += blockChange;

    DisengageSD();
}

void LoadProject(File *file)
{
    currentProjectName = malloc(file->nameLength + 1);
    strcpy(currentProjectName, file->name);

    FlushBuffer();
    char *buffer = ReadBlock(file->startBlock);

    int index = 0;

    while (buffer[index++] != '|')
        ;

    printf("%s", buffer + index);

    int v1, v2, v3;

    sscanf(buffer + index, "%d|%d|%d", &v1, &v2, &v3);

    printf("v1: %d\n", v1);
    spriteCount = (uint8_t)v1;
    scriptCount = (uint8_t)v2;
    sceneCount = (uint8_t)v3;

    printf("Sprites: %d\n", spriteCount);

    free(buffer);

    buffer = ReadBlock(file->startBlock - 1);
    for (int i = 0; i < spriteCount; i++)
    {
        if (i % 3 == 0)
        {
            free(buffer);
            buffer = ReadBlock(file->startBlock - i / 3 - 1);
        }
        print("\nsprite\n");

        TextToSprite(buffer + (i % 3) * 129);
    }
    free(buffer);

    int readScriptOffest = ceil(spriteCount / 3) + 1;

    file->startBlock -= readScriptOffest;

    char *scriptsData = ReadFileUntil(file, '\0');
    printf("scripts include: %s\n", scriptsData);

    index = 0;

    for (int i = 0; i < scriptCount; i++)
    {
        char name[16];
        uint8_t nameIndex = 0;
        while (scriptsData[index] != '\n' && scriptsData[index] != '\0' && nameIndex < 15)
        {
            printf("script letter: %c\n", scriptsData[index]);
            name[nameIndex++] = scriptsData[index];
            index++;
        }
        name[nameIndex] = '\0';
        printf("Script name: %s\n", name);
        index++;

        strcpy(scripts[i].name, name);
        scripts[i].ID = i;

        char *fullScriptName = malloc(strlen(name) + 1 + strlen(program->name) + strlen(":ENGINESCRIPT") + 1);
        sprintf(fullScriptName, "%s:%s:ENGINESCRIPT", name, program->name);
        File *scriptFile = GetFile(fullScriptName);

        scripts[i].content = ReadFileUntil(scriptFile, '\0');

        printf("script content: %s\n", scripts[i].content);

        free(scriptFile->name);
        free(scriptFile);
        free(fullScriptName);
    }

    free(scriptsData);

    file->startBlock += readScriptOffest;

    DisengageSD();
}

#endif