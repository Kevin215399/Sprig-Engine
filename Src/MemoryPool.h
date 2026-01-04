#ifndef MEMORY_POOL
#define MEMORY_POOL

#include "EngineStructs.h"

#define VAR_POOL_COUNT 120
#define STRING_POOL_COUNT 400
#define STRING_POOL_WIDTH 80

#define NULL_POOL 65535

EngineVar varPool[VAR_POOL_COUNT];
uint8_t varPoolFree[VAR_POOL_COUNT / 8];
char stringPool[STRING_POOL_COUNT][STRING_POOL_WIDTH];
uint8_t stringPoolFree[STRING_POOL_COUNT / 8];

bool BoolFromByte(uint8_t byte, uint8_t indexOfByte)
{
    return (byte & (0x01 << indexOfByte)) != 0;
}
void WriteBoolToByte(uint8_t *byte, bool value, uint8_t indexOfByte)
{
    if (value)
    {
        *byte |= (1 << indexOfByte);
    }
    else
    {
        *byte &= ~(1 << indexOfByte);
    }
}

uint16_t PoolVar(char *varName)
{
    bool isFilled = BoolFromByte(varPoolFree[0], 0);
    uint16_t index = 0;
    while (isFilled)
    {
        index++;
        if (index >= VAR_POOL_COUNT)
        {
            printf("%s\n", "Var Pool Overflow!");
            while (1)
                sleep_ms(1000);
        }
        isFilled = BoolFromByte(varPoolFree[index / 8], index % 8);
    }

    varPool[index].currentType = NO_TYPE;
    strncpy(varPool[index].name, varName, 16);
    varPool[index].name[15] = '\0';

    printf("Pooled var to index: %d\n", index);

    WriteBoolToByte(&varPoolFree[index / 8], true, index % 8);

    return index;
}

void FreeVar(uint16_t *index)
{
    printf("Freed var from pool: %d\n",*index);
    WriteBoolToByte(&varPoolFree[*index / 8], false, *index % 8);
    *index = NULL_POOL;
}

uint16_t PoolString()
{
    bool isFilled = BoolFromByte(stringPoolFree[0], 0);
    uint16_t index = 0;
    while (isFilled)
    {
        index++;
        if (index >= STRING_POOL_COUNT)
        {
            printf("%s\n", "String Pool Overflow!");
            while (1)
                sleep_ms(1000);
        }
        isFilled = BoolFromByte(stringPoolFree[index / 8], index % 8);
    }

    stringPool[index][0] = '\0';

    printf("Pooled string to index: %d\n", index);

    WriteBoolToByte(&stringPoolFree[index / 8], true, index % 8);

    return index;
}

void FreeString(uint16_t *index)
{
    printf("Freed string from pool: %d\n",*index);
    WriteBoolToByte(&stringPoolFree[*index / 8], false, *index % 8);
    *index = NULL_POOL;
}

#endif
