#ifndef ENGINE_STRUCTS
#define ENGINE_STRUCTS

#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>
// #include "LinkedList.h"

#define COLLIDER_VARS 6
#define BASE_VARS 5

#define MAX_SCRIPTS_PER_OBJECT 10
#define MAX_NAME_LENGTH 16
#define MAX_PARAMETERS 10

#define SCRIPT_LENGTH 512 * 10 - 1

#define NO_TYPE 0
#define TYPE_BOOL 1
#define TYPE_INT 2
#define TYPE_FLOAT 3
#define TYPE_STRING 4
#define TYPE_VECTOR 5
#define TYPE_OBJ 6

#define SPRITE_WIDTH 8
#define SPRITE_HEIGHT 8

#define CURLY_BRACKET 0
#define PARENTHESIS 1

#define MAX_OBJECTS 40
#define VARS_PER_SCRIPT 80

bool CharCmpr(char *str1, char *str2)
{
    int index = 0;
    while (str1[index] != '\0' && str2[index] != '\0')
    {
        if (str1[index] != str2[index])
        {
            return false;
        }
        index++;
    }

    if ((str1[index] != '\0') ^ (str2[index] != '\0'))
    {
        return false; // If one string is longer than the other
    }

    return true;
}

// A struct for position
typedef struct
{
    float x;
    float y;
} Vector2;

// A union that holds all supported types
typedef union
{
    bool b;
    int i;
    float f;
    uint16_t s;

    Vector2 XY;
    uint8_t objID;
} VariableUnion;

// A struct that defines an engine variable. Can be added to objects
typedef struct
{
    char name[MAX_NAME_LENGTH];
    uint8_t currentType;
    VariableUnion data;
    bool serialized;
} EngineVar;

typedef struct
{
    char *parameterName;
    uint8_t parameterType;
} EngineParameter;

typedef struct
{
    char *name;
    EngineParameter parameters[MAX_PARAMETERS];
    uint8_t parameterIndex;

    int bracketStart;
    int bracketEnd;

} EngineFunction;

// A struct that defines an engine script. Can be added to objects
typedef struct
{
    uint8_t ID;
    char name[MAX_NAME_LENGTH];
    char *content;
} EngineScript;

// A struct that defines an engine sprite. Can be added to objects
typedef struct
{
    uint8_t ID;
    // char name[MAX_NAME_LENGTH];
    uint16_t sprite[SPRITE_WIDTH][SPRITE_HEIGHT];
} EngineSprite;

// Contains the start and end of a bracket pair
typedef struct
{
    int start;
    int startPos;
    int end;
    int endPos;
    bool bracketType;
} BracketPair;

struct EngineObject;

// A struct that keeps script data. Can be added to objects
typedef struct ScriptData
{
    EngineScript *script;

    uint16_t currentLine;

    char **lines;
    int *lineIndexes;
    uint16_t lineCount;

    EngineVar *data;
    uint8_t variableCount;

    EngineVar *backupData;
    uint8_t backupVarCount;

    EngineFunction *functions[40];
    uint8_t functionCount;

    BracketPair *brackets;
    uint8_t bracketPairs;

    struct EngineObject *linkedObject;

} ScriptData;

typedef struct VarNode
{
    struct VarNode *next;
    struct VarNode *previous;
    uint8_t index;
    EngineVar *link;
} VarNode;

// A struct that defines an engine object. holds scripts, a sprite, name, and data
typedef struct EngineObject
{
    uint8_t ID;
    char name[MAX_NAME_LENGTH];

    uint8_t scriptCount;

    ScriptData *scriptData[MAX_SCRIPTS_PER_OBJECT];
    uint8_t scriptIndexes[MAX_SCRIPTS_PER_OBJECT];

    // Each object has position, sprite, and scale
    VarNode *objectDataTail;
    uint8_t objectDataCount;

    uint8_t colliderCount;
    uint16_t *colliderBoxes;

    bool packages[2];

} EngineObject;

typedef struct
{
    char name[MAX_NAME_LENGTH];
    EngineObject **objects;
    uint8_t objectCount;
} EngineScene;

/////////////////////////////// DATA //////////////////////////////////////

EngineScript *NULL_SCRIPT;
EngineSprite *NULL_SPRITE;
EngineObject *NULL_OBJECT;
EngineVar *NULL_VAR;
ScriptData *NULL_SCRIPT_DATA;

// Old Linked list handler
/*LinkedList *scripts;
LinkedList *sprites;
LinkedList *objects;

// Set up linked lists for scripts, sprites, and objects
void InitializeLists()
{
    printf("Script size: %u\n", sizeof(EngineScript));
    scripts = InitializeList(sizeof(EngineScript));
    sprites = InitializeList(sizeof(EngineSprite));
    objects = InitializeList(sizeof(EngineObject));
}

void CreateScript(char *name, uint8_t nameLength)
{
    EngineScript *newScript = malloc(sizeof(EngineScript));
    print("Created script\n");
    for (int i = 0; i < MAX_NAME_LENGTH; i++)
    {
        if (i >= MAX_NAME_LENGTH)
        {
            newScript->name[i] = '\0';
        }
        newScript->name[i] = name[i];
    }
    newScript->ID = scripts->length;
    print("Set script name\n");
    AddToList(scripts, newScript, sizeof(EngineScript));
    print("Added script\n");
    free(newScript);
}

EngineScript *GetScriptByID(uint8_t ID)
{
    ListNode *currentNode = scripts->start;
    print("GetScriptStart\n");
    if ((*((EngineScript *)(currentNode->contents))).ID == ID)
    {
        print("First Script is goal");
        EngineScript *out = (EngineScript *)currentNode->contents;
        free(currentNode);
        return out;
    }
    while (currentNode->next != NULL)
    {
        if (((EngineScript *)(currentNode->contents))->ID == ID)
        {
            print("found Script is goal");
            EngineScript *out = (EngineScript *)currentNode->contents;
            free(currentNode);
            return out;
        }
        currentNode = currentNode->next;
    }
    free(currentNode);
    print("did not find");
    return &NULL_SCRIPT;
}*/

#define MAX_SCRIPTS 25
#define MAX_SPRITES 30
#define MAX_SCENES 10

EngineScript scripts[MAX_SCRIPTS];
uint8_t scriptCount = 0;

EngineSprite sprites[MAX_SPRITES];
uint8_t spriteCount = 0;

EngineScene scenes[MAX_SCENES];
uint8_t sceneCount;

/////////////////////////////// HELPER FUNCTIONS /////////////////////////////////

EngineVar *VarConstructor(char *name, uint8_t nameLength, uint8_t dataType, bool serialized)
{
    EngineVar *output = (EngineVar *)malloc(sizeof(EngineVar));
    for (int i = 0; i < MAX_NAME_LENGTH; i++)
    {
        if (i < nameLength)
        {
            output->name[i] = name[i];
        }
        else
        {
            output->name[i] = '\0';
        }
    }
    output->currentType = dataType;
    output->serialized = serialized;
    return output;
}

EngineScript *ScriptConstructor(uint8_t ID, char *name, char *content)
{
    EngineScript *output = (EngineScript *)malloc(sizeof(EngineScript));
    for (int i = 0; i < MAX_NAME_LENGTH; i++)
    {
        if (i < strlen(name))
        {
            output->name[i] = name[i];
        }
        else
        {
            output->name[i] = '\0';
        }
    }
    output->content = (char *)malloc((strlen(content) + 1) * sizeof(char));
    for (int i = 0; i < strlen(content); i++)
    {
        // printf("Script: %c\n",content[i]);
        output->content[i] = content[i];
    }
    output->content[strlen(content)] = '\0';
    output->ID = ID;
    return output;
}

EngineSprite *SpriteConstructor(uint8_t ID, uint16_t sprite[SPRITE_WIDTH][SPRITE_HEIGHT])
{
    EngineSprite *output = (EngineSprite *)malloc(sizeof(EngineSprite));
    output->ID = ID;

    for (int i = 0; i < SPRITE_HEIGHT * SPRITE_WIDTH; i++)
    {
        output->sprite[i % SPRITE_WIDTH][i / SPRITE_WIDTH] = sprite[i % SPRITE_WIDTH][i / SPRITE_WIDTH];
    }
    return output;
}
ScriptData *ScriptDataConstructor(EngineScript *script)
{
    ScriptData *output = (ScriptData *)malloc(sizeof(ScriptData));

    output->data = malloc(sizeof(EngineVar) * VARS_PER_SCRIPT);
    output->backupData = NULL;

    output->script = script;

    // output->data = variables;
    output->variableCount = 0;
    output->backupVarCount = 0;
    output->functionCount = 0;
    output->bracketPairs = 0;
    output->currentLine = 0;

    output->linkedObject = NULL;

    return output;
}

void AddDataToObject(EngineObject *object, EngineVar *data)
{
    if (object->objectDataTail == NULL)
    {
        object->objectDataTail = malloc(sizeof(VarNode));
        object->objectDataTail->link = data;
        object->objectDataTail->next = NULL;
        object->objectDataTail->previous = NULL;
        object->objectDataTail->index = object->objectDataCount;
        object->objectDataCount++;
        return;
    }

    VarNode *newNode = malloc(sizeof(VarNode));

    object->objectDataTail->next = newNode;
    newNode->previous = object->objectDataTail;

    newNode->next = NULL;
    newNode->link = data;
    newNode->index = object->objectDataCount;

    object->objectDataTail = newNode;
    object->objectDataCount++;
}

void ClearDataFromObject(EngineObject *object)
{
    printf("clear obj %s\n", object->name);
    VarNode *currentData = object->objectDataTail;
    while (currentData->previous != NULL)
    {
        currentData = currentData->previous;
        if (currentData->next->link != NULL)
        {
            printf("clearing %s\n", currentData->next->link->name);
            free(currentData->next->link);
        }
        free(currentData->next);
    }
    if (currentData != NULL)
    {
        free(currentData->link);
        free(currentData);
    }
    currentData = NULL;
    object->objectDataCount = 0;
}

EngineVar *GetObjectDataByName(EngineObject *obj, char *name)
{
    VarNode *current = obj->objectDataTail;
    while (current != NULL)
    {
        if (strcmp(current->link->name, name) == 0)
        {
            return current->link;
        }
        current = current->previous;
    }
    return NULL;
}
EngineVar *GetObjectDataByIndex(EngineObject *obj, uint8_t index)
{
    VarNode *current = obj->objectDataTail;
    while (current != NULL)
    {
        if (current->index == index)
        {
            return current->link;
        }
        current = current->previous;
    }
    return NULL;
}
EngineVar **ObjectDataToList(EngineObject *object)
{
    EngineVar **list = (EngineVar **)malloc(sizeof(EngineVar *) * object->objectDataCount);
    for (int i = 0; i < object->objectDataCount; i++)
    {
        list[i] = GetObjectDataByIndex(object, i);
    }
    return list;
}

EngineObject *ObjectConstructor(uint8_t ID, char *name, uint8_t nameLength)
{
    EngineObject *output = (EngineObject *)malloc(sizeof(EngineObject));
    output->ID = ID;

    for (int i = 0; i < MAX_NAME_LENGTH; i++)
    {
        if (i < strlen(name))
        {
            output->name[i] = name[i];
        }
        else
        {
            output->name[i] = '\0';
        }
    }

    // output->scriptData = (ScriptData **)malloc(sizeof(ScriptData *) * MAX_SCRIPTS_PER_OBJECT);
    output->scriptCount = 0;
    output->objectDataCount = 0;
    output->objectDataTail = NULL;
    output->colliderBoxes = NULL;

    memset(output->packages, 0, sizeof(output->packages));

    AddDataToObject(output, VarConstructor("position", strlen("position"), TYPE_VECTOR, true));
    AddDataToObject(output, VarConstructor("sprite", strlen("sprite"), TYPE_INT, true));
    AddDataToObject(output, VarConstructor("scale", strlen("scale"), TYPE_VECTOR, true));

    AddDataToObject(output, VarConstructor("velocity", strlen("velocity"), TYPE_VECTOR, false));
    AddDataToObject(output, VarConstructor("drag", strlen("drag"), TYPE_INT, true));

    print("Added data");

    GetObjectDataByName(output, "sprite")->data.i = 0;
    GetObjectDataByName(output, "scale")->data.XY.x = 1;
    GetObjectDataByName(output, "scale")->data.XY.y = 1;

    GetObjectDataByName(output, "position")->data.XY.x = 0;
    GetObjectDataByName(output, "position")->data.XY.y = 0;

    GetObjectDataByName(output, "velocity")->data.XY.x = 0;
    GetObjectDataByName(output, "velocity")->data.XY.y = 0;

   // GetObjectDataByName(output, "drag")->data.i = 10;

    print("set data");

    output->colliderCount = 0;

    return output;
}

// Put dummy data into NULL_SCRIPT, NULL_SPRITE, NULL_OBJECT, and NULL_VAR
void CreateNullStructs()
{
    NULL_VAR = VarConstructor("", 0, 0, false);
    NULL_SCRIPT = ScriptConstructor(0, "", "");
    NULL_SCRIPT_DATA = ScriptDataConstructor(NULL_SCRIPT);
    uint16_t emptySprite[SPRITE_WIDTH][SPRITE_HEIGHT];
    for (int i = 0; i < SPRITE_HEIGHT * SPRITE_WIDTH; i++)
    {
        emptySprite[i % SPRITE_WIDTH][i / SPRITE_WIDTH] = 0;
    }
    NULL_SPRITE = SpriteConstructor(0, emptySprite);
    NULL_OBJECT = ObjectConstructor(0, "", 0);
}

// Returns the index of the sprite
uint8_t CreateSprite()
{
    sprites[spriteCount].ID = spriteCount;
    spriteCount++;
    return spriteCount - 1;
}

// Returns the index of the script
uint8_t CreateScript(char *name, uint8_t nameLength)
{
    for (int i = 0; i < MAX_NAME_LENGTH; i++)
    {
        if (i < nameLength)
        {
            scripts[scriptCount].name[i] = name[i];
        }
        else
        {
            scripts[scriptCount].name[i] = '\0';
        }
    }
    scripts[scriptCount].ID = scriptCount;
    return scriptCount++;
}

// Returns the index of the scene
uint8_t CreateScene(char *name, uint8_t nameLength)
{
    for (int i = 0; i < MAX_NAME_LENGTH; i++)
    {
        if (i < nameLength)
        {
            scenes[sceneCount].name[i] = name[i];
        }
        else
        {
            scenes[sceneCount].name[i] = '\0';
        }
    }
    scenes[sceneCount].objectCount = 0;
    scenes[sceneCount].objects = (EngineObject **)malloc(sizeof(EngineObject*) * MAX_OBJECTS);
    return sceneCount++;
}

int GetScriptIndexByID(uint8_t ID)
{
    for (int i = 0; i < scriptCount; i++)
    {
        if (ID == scripts[i].ID)
            return i;
    }
    return -1;
}

int GetScriptByName(char *name)
{
    for (int i = 0; i < scriptCount; i++)
    {
        if (CharCmpr(name, scripts[i].name))
            return i;
    }
    return -1;
}

int GetSceneByName(char *name)
{
    for (int i = 0; i < sceneCount; i++)
    {
        if (CharCmpr(name, scenes[i].name))
            return i;
    }
    return -1;
}

#endif