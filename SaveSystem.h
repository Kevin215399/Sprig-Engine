#ifndef SAVE_SYSTEM
#define SAVE_SYSTEM

#include <stdio.h>
#include "pico/stdlib.h"

#include "SDSimple.h"
#include "TFT.h"

#include "EngineStructs.h"
#include <string.h>
#include <math.h>
#include "MemoryPool.h"

#include "ColliderPackage.h"

char FILE_IDENTIFIER[] = "SPRIG-ENGINE1";

char *currentProjectName;

typedef struct
{
    char *array;
    uint8_t length;
} charArray;

uint16_t SerializeVar(EngineVar *variable);

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
        free(result->name);
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

    snprintf(buffer, sizeof(buffer), "%s`%d`%d`%d`0", FILE_IDENTIFIER, spriteCount, scriptCount, sceneCount);

    WriteFile(newFile, buffer, 512);

    print("wrote to project");
    free(newFile->name);
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

    Block data = ReadBlock(file->startBlock);
    print("FileFormatCheck: got file block");

    uint8_t blocksOccupied = file->startBlock - file->endBlock + 1;

    bool validIdentifier = true;
    for (int i = 0; i < strlen(FILE_IDENTIFIER); i++)
    {
        if (FILE_IDENTIFIER[i] != data.data[i])
        {
            validIdentifier = false;
            break;
        }
    }
    print("FileFormatCheck: checked identifier");
    // DisengageSD();

    free(file->name);
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
        free(f->name);
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
        free(file->name);
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

char *SerializeObject(EngineObject *object)
{
#define VAR_SERIALIZE_LENGTH 64

    char *output = malloc(strlen(object->name) + 7 + (object->objectDataCount * VAR_SERIALIZE_LENGTH) + (object->scriptCount * 4) + 1);

    sprintf(output, "%s\n%d`%d`%c`%c", object->name, object->scriptCount, object->objectDataCount, (object->packages[0]) ? 't' : 'f', 'f');

    printf("obj %s\n", output);

    int index = strlen(output);
    for (int i = 0; i < object->objectDataCount; i++)
    {
        EngineVar* objectData = GetObjectDataByIndex(object,i);
        uint16_t value = SerializeVar(objectData);

        sprintf(output + index, "`%s\n`%d`%s\n",
                objectData->name,
                objectData,
                stringPool[value]);

        printf("obj %s\n", output);

        index = strlen(output);

        FreeString(&value);
    }

    for (int i = 0; i < object->scriptCount; i++)
    {
        sprintf(output + index,
                "`%d",
                object->scriptIndexes[i]);

        index = strlen(output);
        printf("obj %s\n", output);
    }

    output[index] = '\0';

    printf("Object serialized: %s\n", output);

    return output;
}

EngineVar *DeserializeVar(uint8_t type, char *serializedVar)
{
    EngineVar *output = VarConstructor("", 0, NO_TYPE);
    switch (type)
    {
    case TYPE_BOOL:
        output->currentType = TYPE_BOOL;
        output->data.b = serializedVar[0] == '0' ? false : true;
        break;
    case TYPE_FLOAT:
        output->currentType = TYPE_FLOAT;
        output->data.f = atof(serializedVar);
        break;
    case TYPE_INT:
        output->currentType = TYPE_INT;
        output->data.i = (int)floor(atof(serializedVar));
        break;
    case TYPE_STRING:
        output->currentType = TYPE_STRING;
        output->data.s = PoolString();
        strcpy(stringPool[output->data.s], serializedVar);
        break;
    case TYPE_VECTOR:
        output->currentType = TYPE_VECTOR;
        sscanf(serializedVar, "(%d, %d)", &output->data.XY.x, &output->data.XY.y);
        break;

    default:
        break;
    }
    return output;
}

EngineObject *DeserializeObject(char *serializedObject)
{
    printf("Deserialize: %s\n", serializedObject);

    int scriptCount = 0;
    int dataCount = 0;
    char colliderAdded = 'f';
    char physicsAdded = 'f';
    char name[16];
    sscanf(serializedObject, "%[^\n]\n%d`%d`%c`%c", &name, &scriptCount, &dataCount, &colliderAdded, &physicsAdded);

    printf("header: %s, %d, %d\n", name, scriptCount, dataCount);

    int index = strlen(name) + IntLength(scriptCount) + IntLength(dataCount) + 6;

    EngineObject *objectOut = (EngineObject *)malloc(sizeof(EngineObject));
    strcpy(objectOut->name, name);
    objectOut->name[15] = '\0';

    objectOut->scriptCount = scriptCount;
    objectOut->objectDataCount = dataCount;
    objectOut->colliderCount = 0;

    for (int i = 0; i < dataCount; i++)
    {
        int type = 0;
        char dataName[32];
        char data[64];
        printf("index: %d: %c\n", index, serializedObject[index]);
        sscanf(serializedObject + index, "`%[^\n]\n`%d`%[^\n]\n",&dataName, &type, &data);
        printf("data type: %d, %s\n", type, data);

        EngineVar *var = DeserializeVar(type, data);
        strcpy(var->name,dataName);
        AddDataToObject(objectOut,var);
        
        printf("deserialized type: %d\n", GetObjectDataByIndex(objectOut,i)->currentType);

        index += 5 + IntLength(type) + strlen(data) + strlen(dataName);
    }

    if (colliderAdded == 't')
    {
        objectOut->packages[0] = true;
        RecalculateObjectColliders(objectOut);
    }

    for (int i = 0; i < scriptCount; i++)
    {
        sscanf(serializedObject + index,
               "`%d",
               objectOut->scriptIndexes[i]);

        index = IntLength(objectOut->scriptIndexes[i]) + 1;
        printf("added script %d\n", objectOut->scriptIndexes[i]);
    }

    return objectOut;
}

void SaveProject(File *file)
{

    FlushBuffer();

    int originalBlock = file->startBlock;

    char buffer[512];

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

    int scriptBlock = file->startBlock;

    print("saved scripts to project");

    free(scriptNames);

    file->startBlock -= ceil((float)scriptNameLength / 512) + 1;

    printf("scene block: %d\n", file->startBlock);

    int sceneNameLength = 1;
    for (int i = 0; i < sceneCount; i++)
    {
        sceneNameLength += strlen(scenes[i].name) + 1;
    }

    char *sceneNames = malloc(sceneNameLength);

    index = 0;
    for (int i = 0; i < sceneCount; i++)
    {
        printf("Save scene: %s, index: %d\n", scenes[i].name, i);
        strncpy(sceneNames + index, scenes[i].name, strlen(scenes[i].name));

        sceneNames[index + strlen(scenes[i].name)] = '\n';

        index += strlen(scenes[i].name) + 1;
    }

    int sceneBlock = file->startBlock;

    sceneNames[sceneNameLength - 1] = '\0';

    printf("scene names: %s\n", sceneNames);

    WriteFile(file, sceneNames, sceneNameLength);

    print("saved scene names to project");

    free(sceneNames);

    for (int i = 0; i < 512; i++)
    {
        buffer[i] = 0;
    }

    file->startBlock = originalBlock;

    snprintf(buffer, sizeof(buffer), "%s`%d`%d`%d`%d`%d", FILE_IDENTIFIER, spriteCount, scriptCount, sceneCount, scriptBlock, sceneBlock);

    WriteBlock(file->startBlock, buffer);

    DisengageSD();
}

void LoadProject(File *file)
{
    currentProjectName = malloc(file->nameLength + 1);
    strcpy(currentProjectName, file->name);

    FlushBuffer();
    Block buffer = ReadBlock(file->startBlock);

    int index = 0;

    while (buffer.data[index++] != '`')
        ;

    printf("%s", buffer.data + index);

    int v1, v2, v3, scriptBlockStart, sceneBlockStart;

    sscanf(buffer.data + index, "%d`%d`%d`%d`%d", &v1, &v2, &v3, &scriptBlockStart, &sceneBlockStart);

    printf("v1: %d\n", v1);
    spriteCount = (uint8_t)v1;
    scriptCount = (uint8_t)v2;
    sceneCount = (uint8_t)v3;

    printf("Sprites: %d\n", spriteCount);

    memcpy(buffer.data, ReadBlock(file->startBlock - 1).data, 512);
    for (int i = 0; i < spriteCount; i++)
    {
        if (i % 3 == 0)
        {
            memcpy(buffer.data, ReadBlock(file->startBlock - i / 3 - 1).data, 512);
        }
        print("\nsprite\n");

        TextToSprite(buffer.data + (i % 3) * 129);
    }

    int originalStartBlock = file->startBlock;

    file->startBlock = scriptBlockStart;

    printf("script block: %d\n", file->startBlock);

    if (scriptBlockStart != 0)
    {
        char *scriptsData = ReadFileUntil(file, '\0');
        printf("scripts include: %s\n", scriptsData);

        index = 0;

        int scriptsNameLength = 1;
        
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
            scriptsNameLength += strlen(name) + 1;
            printf("Script name: %s\n", name);
            index++;

            strcpy(scripts[i].name, name);
            scripts[i].ID = i;

            char *fullScriptName = malloc(strlen(name) + 1 + strlen(program->name) + strlen("`ENGINESCRIPT") + 1);
            sprintf(fullScriptName, "%s`%s`ENGINESCRIPT", name, program->name);
            File *scriptFile = GetFile(fullScriptName);

            scripts[i].content = ReadFileUntil(scriptFile, '\0');

            printf("script content: %s\n", scripts[i].content);

            free(scriptFile->name);
            free(scriptFile);
            free(fullScriptName);
        }
        free(scriptsData);
    }

    if (sceneBlockStart != 0)
    {
        file->startBlock = sceneBlockStart;

        printf("scene block: %d\n", file->startBlock);

        char *scenesData = ReadFileUntil(file, '\0');
        printf("scenes include: %s\n", scenesData);

        index = 0;
        printf("scene count %d\n", sceneCount);
        for (int i = 0; i < sceneCount; i++)
        {
            printf("Scene loop %d\n", i);
            char name[16];
            uint8_t nameIndex = 0;
            while (scenesData[index] != '\n' && scenesData[index] != '\0' && nameIndex < 15)
            {
                printf("scene letter: %c\n", scenesData[index]);
                name[nameIndex++] = scenesData[index];
                index++;
            }
            name[nameIndex] = '\0';
            printf("scene name: %s\n", name);
            index++;

            strncpy(scenes[i].name, name, sizeof(scenes[i].name));
            scenes[i].name[sizeof(scenes[i].name) - 1] = '\0';

            char *sceneFileName = malloc(32);
            sprintf(sceneFileName, "%s`%s`%s", FILE_IDENTIFIER, currentProjectName, name);
            File *sceneFile = GetFile(sceneFileName);

            free(sceneFileName);

            char *objectsSerialized = ReadFileUntil(sceneFile, '\0');
            printf("object header: %s\n", objectsSerialized);
            int objectCount = 0;
            sscanf(objectsSerialized, "%d", &objectCount);
            free(objectsSerialized);

            scenes[i].objectCount = objectCount;
            scenes[i].objects = malloc(sizeof(EngineObject) * MAX_OBJECTS);

            sceneFile->startBlock -= 1;

            printf("Object count: %d\n", objectCount);

            for (int x = 0; x < objectCount; x++)
            {
                printf("setting object %d\n", x);
                char *objectData = ReadFileUntil(sceneFile, '\0');
                scenes[i].objects[x] = *DeserializeObject(objectData);
                free(objectData);
                sceneFile->startBlock -= 2;
            }

            free(sceneFile);
        }

        free(scenesData);
    }
    file->startBlock = originalStartBlock;

    DisengageSD();
}

#endif