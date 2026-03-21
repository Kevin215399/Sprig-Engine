#ifndef INTERPRETER
#define INTERPRETER

#include <stdio.h>
#include "pico/stdlib.h"

#include "SaveSystem.h"

// #include "Editor.h"

#include "EngineStructs.h"
#include <string.h>

#include <math.h>

#include "MemoryPool.h"

#include "DebugPrint.h"

#include "DebugConsole.h"

#define LEFT_LIGHT 28
#define RIGHT_LIGHT 4

#define EQUATION_ERROR 1
#define FUNCTION_ERROR 2
#define VARIABLE_ERROR 3
#define PARENTHESIS_ERROR 4
#define DATATYPE_ERROR 5
#define GENERAL_ERROR 6
#define SCENE_ERROR 7
#define RENDERER 8

#define INCORRECT_PAIRS 1
#define BRACKET_MISMATCH 2
#define SYNTAX_UNKNOWN 3
#define MALLOC_FAIL 4
#define EQUATION_UNSOLVED 5
#define TYPE_MISMATCH 6
#define TOKEN_NOT_RECOGNIZED 7
#define UNKNOWN_ASSIGNMENT_OPERATOR 8
#define OPERATOR_CANNOT_BE_USED_WITH_TYPE 9
#define NO_CAMERA_DEFINED 10
#define COULD_NOT_START_CORE_1 11
#define COMPARISON_TYPE_WRONG 12

#define OPPERATION_SUCCESS 0x8000

const char* ERROR_NAMES[] = {
    "Missing Parenthesis",
    "Parenthesis Type Mismatch",
    "Syntax Unknown",
    "Allocation Error",
    "Equation was not solved",
    "Type mismatch",
    "Token was not recognized",
    "Assignment attribute unknown",
    "Type cannot be transformed using this operator",
    "No camera, add an object named \"Camera\"",
    "CORE 1 DID NOT RESPOND",
    "\'==\' can only be used on numbers"
};

const char* CATEGORY_ERRORS[] = {
    "EQUATION",
    "FUNCTION",
    "VARIABLE",
    "PARENTHESIS",
    "DATATYPE",
    "GENERAL",
    "SCENE",
    "RENDERER" };

#define OPERATOR 0
#define VALUE 1
#define UNKOWN_TOKEN 2

#define UNARY_SUBTRACT 255

typedef struct {
    uint16_t line;
    ScriptData* script;
} InstructionPointer;

typedef struct
{
    uint8_t dataType;
    uint8_t atomType;
    uint8_t precedence;
    uint8_t parameters;

    VariableUnion data;
    GeneralList* listData;
} Atom;

typedef struct
{
    char* operator;
    uint8_t precedence;
    uint8_t parameters;
} OperatorPrecedence;

// when an unkown token is found, set to this precedence
#define UNKOWN_PRECEDENCE 13

#define OPERATOR_COUNT 43

OperatorPrecedence OPERATOR_PRECEDENT_LIST[] = {

    {"push", 13, 1},
    {"pop", 13, 0},
    {"seek", 13, 1},
    {"contains", 13, 1},

    {".", 13, 1},

    {"ObjectByName", 13, 1},
    {"ScriptByName", 13, 1},

    {"Vector", 13, 2},

    {"input", 13, 1},
    {"deltaTime", 13, 0},

    {"getPosition", 13, 0},
    {"getScale", 13, 0},
    {"getSprite", 13, 0},
    {"getVelocity",13,0},
    {"getAngle",13,0},

    {"rightLED", 13, 1},
    {"leftLED", 13, 1},

    {"setPosition", 13, 1},
    {"setScale", 13, 1},
    {"setSprite", 13, 1},
    {"setVelocity", 13, 1},
    {"setAngle", 13, 1},

    {"addPosition", 13, 1},
    {"addScale", 13, 1},
    {"addVelocity", 13, 1},
    {"addAngle", 13, 1},

    {"PI", 12, 0},

    {"pow", 10, 2},
    {"sin", 10, 1},
    {"cos", 10, 1},

    {"%", 9, 2},
    {"^", 9, 2},
    {"*", 8, 2},
    {"/", 8, 2},
    {"-", 7, 2},
    {"+", 7, 2},

    {">=", 6, 2},
    {"<=", 6, 2},
    {">", 6, 2},
    {"<", 6, 2},

    {"==", 5, 2},
    {"!=", 5, 2},

    {"&", 4, 2},

    {"^", 3, 2},

    {"|", 2, 2},

};

//returns an instruction struct as a pointer
InstructionPointer* NewInstruction(uint16_t line, ScriptData* script) {
    InstructionPointer* output = malloc(sizeof(InstructionPointer));
    output->line = line;
    output->script = script;
    return output;
}

GeneralList instructionStack;

void InitializeInterpreter() {
    InitializeList(&instructionStack);
}

void InitializeLights()
{
    gpio_init(LEFT_LIGHT);
    gpio_set_dir(LEFT_LIGHT, true);
    gpio_put(LEFT_LIGHT, false);

    gpio_init(RIGHT_LIGHT);
    gpio_set_dir(RIGHT_LIGHT, true);
    gpio_put(RIGHT_LIGHT, false);
}

char ToLower(char in)
{
    if (in >= 65 && in <= 90)
    {
        return in + 32;
    }
    return in;
}

/*void FreeAtom(Atom *atom)
{
    if (atom == NULL)
    {
        return;
    }
    if (atom->atom != NULL_POOL)
    {
        FreeString(&atom->atom);
    }
    free(atom);
}*/

int indexOf(char* search, char* string)
{
    char* pos = strstr(string, search);
    if (pos == NULL)
        return -1;
    return pos - string;
}

bool EqualType(EngineVar* var1, EngineVar* var2, uint8_t type)
{
    if (var1->currentType == type && var2->currentType == type)
    {
        return true;
    }
    return false;
}

uint32_t CreateError(uint8_t category, uint8_t type, uint16_t line)
{
    uint32_t error = (uint32_t)(category & 0xF) << 24;
    error |= (uint32_t)(type & 0xF) << 16;
    error |= (uint32_t)line;

    return error;
}

uint16_t UnpackErrorMessage(uint32_t error)
{
    const char* ERROR_PREFIX = "ERROR: ";
    const char* CATEGORY_SUFFIX = ", ";
    const char* ERROR_TYPE_SUFFIX = ";";
    const char* LINE_PREFIX = " Line - ";

    if (error == 0)
    {
        uint16_t noError = PoolString();
        strcpy(stringPool[noError], "No Error");
        return noError;
    }
    if (error == 0x8000)
    {
        uint16_t noError = PoolString();
        strcpy(stringPool[noError], "Opperation Success");
        return noError;
    }

    uint8_t category = (uint8_t)((error >> 24) & 0xF) - 1;
    uint8_t type = (uint8_t)((error >> 16) & 0xF) - 1;
    uint16_t line = (uint16_t)(error & 0xFFFF);

    /*uint16_t errorLength = 0;
    errorLength += strlen(CATEGORY_ERRORS[category]);
    errorLength += strlen(ERROR_NAMES[type]);
    uint8_t numberLength = 0;
    if (line != 65535)
    {
        if (line == 0)
        {
            errorLength += 1;
        }
        else
        {

            numberLength = IntLength(line);

            debugPrintf("line digits: %d\n", numberLength);
            errorLength += numberLength;
            errorLength += strlen(LINE_PREFIX);
        }
    }
    errorLength += strlen(ERROR_PREFIX);
    errorLength += strlen(CATEGORY_SUFFIX);
    errorLength += strlen(ERROR_TYPE_SUFFIX);
    debugPrintf("error len %d\n", errorLength);*/

    uint16_t errorMessage = PoolString();

    if (line != 65535)
    {
        sprintf(stringPool[errorMessage], "%s%s%s%s%s%s%d", ERROR_PREFIX, CATEGORY_ERRORS[category], CATEGORY_SUFFIX, ERROR_NAMES[type], ERROR_TYPE_SUFFIX, LINE_PREFIX, line);
    }
    else
    {
        sprintf(stringPool[errorMessage], "%s%s%s%s%s", ERROR_PREFIX, CATEGORY_ERRORS[category], CATEGORY_SUFFIX, ERROR_NAMES[type], ERROR_TYPE_SUFFIX);
    }
    // errorMessage[errorLength - 1] = '\0';

    return errorMessage;
}

void SplitScript(EngineScript* script, ScriptData* scriptData)
{
    int lineCount = 0;
    for (int i = 0; i < strlen(script->content); i++)
    {
        if (script->content[i] == '\0')
        {
            break;
        }
        if (script->content[i] == ';' || script->content[i] == '{' || script->content[i] == '}')
        {
            lineCount++;
        }
    }
    // debugPrintf("lines %d\n", lineCount);
    debugPrintf("line count: %d\n", lineCount);
    char** lines = (char**)malloc(sizeof(char*) * lineCount);
    int* lineIndexes = (int*)malloc(sizeof(int) * lineCount);
    // debugPrint("total lines allocated");
    int currentLine = 0;
    uint16_t lineLength = 0;

    if (lineCount >= 1)
    {
        lineIndexes[0] = 0;
    }

    for (int i = 0; i < strlen(script->content); i++)
    {
        if (script->content[i] > 32)
        {
            debugPrintf("%c\n", script->content[i]);
        }
        else
        {
            debugPrintf("%d\n", (int)script->content[i]);
        }
        lineLength++;
        if (script->content[i] == ';')
        {
            lines[currentLine] = (char*)malloc(sizeof(char) * (lineLength));
            debugPrintf("allocated line len %d\n", lineLength + 1);
            currentLine++;
            lineLength = 0;
        }
        else if (script->content[i] == '{' || script->content[i] == '}')
        {
            lines[currentLine] = (char*)malloc(sizeof(char) * (lineLength + 1));
            debugPrintf("allocated line len %d\n", lineLength + 1);
            currentLine++;
            lineLength = 0;
        }
    }
    // lines[currentLine] = (char *)malloc(sizeof(char) * (lineLength + 1));
    //  debugPrintf("allocated line len %d\n", lineLength + 1);

    currentLine = 0;
    uint16_t caret = 0;
    for (int i = 0; i < strlen(script->content); i++)
    {
        debugPrintf("%d\n", i);
        debugPrintf("char: %c\n", script->content[i]);
        if (script->content[i] != ';')
            lines[currentLine][caret++] = script->content[i];
        // debugPrintf("line %d, %c\n", currentLine, script->content[i]);
        if (script->content[i] == ';' || script->content[i] == '{' || script->content[i] == '}')
        {
            lines[currentLine][caret] = '\0';
            debugPrintf("line %s\n", lines[currentLine]);
            currentLine++;
            caret = 0;
            if (currentLine < lineCount)
            {
                debugPrintf("Line: %d, index: %d\n", currentLine, i + 1);
                lineIndexes[currentLine] = i + 1;
            }
            else
            {
                break;
            }
        }
    }
    // lines[lineCount - 1][caret] = '\0';

    debugPrint("assigning split");

    scriptData->lines = lines;
    debugPrint("assigned to lines");

    scriptData->lineIndexes = lineIndexes;
    debugPrint("assigned to lineIndexes");

    scriptData->lineCount = lineCount;
    debugPrint("assigned to lineCount");

    scriptData->script = script;
    debugPrint("assigned to script");
}

uint32_t CalculateBrackets(EngineScript* script, ScriptData* output)
{
    uint8_t openBrackets = 0;
    uint8_t closedBrackets = 0;
    for (int i = 0; i < strlen(script->content); i++)
    {
        if (script->content[i] == '(')
        {
            openBrackets++;
        }
        if (script->content[i] == '{')
        {
            openBrackets++;
        }
        if (script->content[i] == ')')
        {
            closedBrackets++;
        }
        if (script->content[i] == '}')
        {
            closedBrackets++;
        }
    }
    if (openBrackets != closedBrackets)
    {
        return CreateError(PARENTHESIS_ERROR, INCORRECT_PAIRS, -1);
    }

    output->script = script;

    output->bracketPairs = openBrackets;
    output->brackets = (BracketPair*)malloc(sizeof(BracketPair) * openBrackets);
    for (int i = 0; i < openBrackets; i++)
    {
        output->brackets[i].start = -1;
        output->brackets[i].end = -1;
        output->brackets[i].bracketType = 0;
    }
    uint16_t currentBracket = 0;
    uint16_t lastOpen = 65535;
    uint16_t currentLine = 0;
    for (int i = 0; i < strlen(script->content); i++)
    {
        if (script->content[i] == ';')
        {
            currentLine++;
            continue;
        }
        if (script->content[i] == '(' || script->content[i] == '{')
        {
            lastOpen = currentBracket;
            output->brackets[currentBracket].start = i;
            output->brackets[currentBracket].startLine = currentLine;
            output->brackets[currentBracket++].bracketType = (script->content[i] == '{') ? CURLY_BRACKET : PARENTHESIS;
            if (script->content[i] == '{')
                currentLine++;
        }
        if (script->content[i] == ')' || script->content[i] == '}')
        {
            if (lastOpen == 65535)
                return CreateError(PARENTHESIS_ERROR, INCORRECT_PAIRS, -1);
            if (output->brackets[lastOpen].start == -1)
                return CreateError(PARENTHESIS_ERROR, INCORRECT_PAIRS, -1);
            if (output->brackets[lastOpen].bracketType == ((script->content[i] == '}') ? CURLY_BRACKET : PARENTHESIS))
            {
                output->brackets[lastOpen].end = i;
                output->brackets[lastOpen].endLine = currentLine;
                while (output->brackets[lastOpen].end != -1)
                {
                    if (lastOpen == 0)
                    {
                        lastOpen = 65535;
                        break;
                    }
                    lastOpen--;
                }
                if (script->content[i] == '}')
                    currentLine++;
            }
            else
                return CreateError(PARENTHESIS_ERROR, BRACKET_MISMATCH, -1);
        }
    }

    return 0;
}

bool IsNumber(char value)
{
    if (value <= 57 && value >= 48)
    {
        return true;
    }
    if (value == '.')
        return true;

    return false;
}

bool IsAlphaNumeric(char c)
{
    if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c >= 48 && c <= 57))
    {
        return true;
    }
    return false;
}

int FloatLength(float value)
{
    bool isDecimal = false;
    while (floor(value) != value)
    {
        value *= 10;
        isDecimal = true;
    }
    debugPrintf("get float len %d", IntLength((int)value));
    return IntLength((int)value) + (isDecimal ? 1 : 0);
}

uint16_t SerializeUnion(VariableUnion* variable, uint8_t type, bool limitDecimals)
{
    switch (type)
    {
    case TYPE_STRING:
    {
        uint16_t strCpy = PoolString();
        strcpy(stringPool[strCpy], stringPool[variable->s]);
        return strCpy;
    }
    break;
    case TYPE_INT:
    {

        uint16_t intChar = PoolString();
        sprintf(stringPool[intChar], "%d", variable->i);

        return intChar;
    }
    break;
    case TYPE_FLOAT:
    {
        debugPrint("serialize float");

        uint16_t floatChar = PoolString();
        if (limitDecimals)
            snprintf(stringPool[floatChar], sizeof(stringPool[floatChar]), "%.3f", variable->f);
        else
            snprintf(stringPool[floatChar], sizeof(stringPool[floatChar]), "%f", variable->f);
        return floatChar;
    }
    break;
    case TYPE_BOOL:
    {
        uint16_t boolChar = PoolString();
        if (variable->b)
        {
            stringPool[boolChar][0] = '1';
        }
        else
        {
            stringPool[boolChar][0] = '0';
        }
        stringPool[boolChar][1] = '\0';
        return boolChar;
    }
    break;
    case TYPE_VECTOR:
    {
        uint16_t vectorChar = PoolString();

        sprintf(stringPool[vectorChar], "(%f,%f)", variable->XY.x, variable->XY.y);
        return vectorChar;
    }
    break;
    default:
    {
        uint16_t empty = PoolString();
        stringPool[empty][0] = '\0';
        return empty;
    }
    break;
    }
}

// Returns a malloced char*
uint16_t SerializeVar(EngineVar* variable, bool limitDecimals)
{
    if (variable->listData.count > 0)
    {
        uint16_t output = PoolString();
        uint8_t index = 0;
        stringPool[output][index++] = '{';
        while (variable->listData.count > 0)
        {
            VariableUnion* pop = (VariableUnion*)PopListFirst(&variable->listData);
            uint16_t unionSerialized = SerializeUnion(pop, variable->currentType, limitDecimals);
            strcpy(stringPool[output] + index, stringPool[unionSerialized]);
            index += strlen(stringPool[unionSerialized]);
            stringPool[output][index++] = ',';
            FreeString(&unionSerialized);
        }
        stringPool[output][index - 1] = '}';
        stringPool[output][index++] = '\0';
        return output;
    }
    else
    {
        uint16_t unionSerialized = SerializeUnion(&variable->data, variable->currentType, limitDecimals);
        return unionSerialized;
    }
}

// If one is a float and one is an int, upgrade to float
void GreatestCommonType(Atom* atom1, Atom* atom2)
{
    if (atom1->dataType == TYPE_INT && atom2->dataType == TYPE_FLOAT)
    {
        atom1->dataType = TYPE_FLOAT;
    }
    if (atom2->dataType == TYPE_INT && atom1->dataType == TYPE_FLOAT)
    {
        atom2->dataType = TYPE_FLOAT;
    }
}

void PushLine(ScriptData* data, uint16_t line)
{
    InstructionPointer* instruction = NewInstruction(line, data);
    PushList(&instructionStack, instruction);
}

void JumpToFunction(ScriptData* scriptData, char* functionName)
{
    debugPrintf("jumping... %s\n", functionName);
    for (int i = 0; i < scriptData->functionCount; i++)
    {
        debugPrintf("function: %s\n", scriptData->functions[i]->name);
        if (strcmp(scriptData->functions[i]->name, functionName) == 0)
        {
            debugPrintf("function found");
            scriptData->currentScope++;
            PushLine(scriptData, scriptData->functions[i]->line);
        }
    }
    debugPrint("not found");
}

bool DataMatch(VariableUnion* a, uint8_t aType, VariableUnion* b, uint8_t bType)
{
    if (aType == TYPE_INT)
    {
        a->f = a->i;
        aType = TYPE_FLOAT;
    }
    if (bType == TYPE_INT)
    {
        b->f = b->i;
        bType = TYPE_FLOAT;
    }

    if (aType != bType)
        return false;

    if (aType == TYPE_FLOAT && a->f == b->f)
        return true;
    if (aType == TYPE_BOOL && a->b == b->b)
        return true;
    if (aType == TYPE_OBJ && a->objIndex == b->objIndex)
        return true;
    if (aType == TYPE_SCRIPT && a->objIndex == b->objIndex && a->scriptIndex == b->scriptIndex)
        return true;
    if (aType == TYPE_STRING && strcmp(stringPool[a->s], stringPool[b->s]) == 0)
        return true;
    if (aType == TYPE_VECTOR && a->XY.x == b->XY.x && a->XY.y == b->XY.y)
        return true;

    return false;
}

Atom* MallocAtom(uint8_t atomType, uint8_t datatype, uint8_t precedence, uint8_t parameters)
{
    Atom* output = malloc(sizeof(Atom));
    output->atomType = atomType;
    output->dataType = datatype;
    output->precedence = precedence;
    output->parameters = parameters;
    output->listData = NULL;
    return output;
}

Atom* MallocTokenAtom(uint8_t atomType, uint8_t precedence, uint8_t parameters, char* token)
{
    Atom* output = malloc(sizeof(Atom));
    output->atomType = atomType;
    output->dataType = TYPE_STRING;
    output->precedence = precedence;
    output->parameters = parameters;
    output->data.s = PoolString();
    strcpy(stringPool[output->data.s], token);
    output->listData = NULL;
    return output;
}

float ShuntYard(char* equation, uint16_t equationLength, EngineVar* output, ScriptData* scriptData)
{
    GeneralList operators;
    GeneralList operands;
    GeneralList tokens;
    InitializeList(&operators);
    InitializeList(&operands);
    InitializeList(&tokens);

    debugPrintf("%s\n", equation);

    ///////////////////////////////////
    // tokenize the equation
    // variables are set to atom type unkown, and data type string
    // operators are set to atom type operator, and data type string
    // Else is set to atom type value, and basic data types
    ///////////////////////////////////

    for (int i = 0; i < equationLength; i++)
    {
        debugPrintf("tokenize %c\n", equation[i]);

        // If the index is parenthesis, push to stack
        // debugPrintf("Finding atom: %c\n", equation[i]);
        if (equation[i] == ' ')
        {
            continue;
        }
        if (equation[i] == ',')
        {
            PushList(&tokens, MallocTokenAtom(OPERATOR, 0, 0, ","));
            continue;
        }
        if (equation[i] == '(')
        {

            PushList(&tokens, MallocTokenAtom(OPERATOR, 0, 0, "("));
            continue;
        }
        if (equation[i] == ')')
        {
            PushList(&tokens, MallocTokenAtom(OPERATOR, 0, 0, ")"));
            continue;
        }

        if (equation[i] == '\"')
        {

            int index = i + 1;
            while (equation[index] != '\"' && equation[index] != '\0')
            {
                index++;
            }

            if (equation[i] == '\0')
            {
                while (tokens.count > 0)
                {
                    Atom* current = (Atom*)PopList(&tokens);
                    if (current->dataType == TYPE_STRING)
                    {
                        FreeString(&current->data.s);
                    }
                    while (current->listData != NULL && current->listData->count > 0)
                    {
                        VariableUnion* listElement = (VariableUnion*)PopList(current->listData);
                        if (current->dataType == TYPE_STRING)
                        {
                            FreeString(&listElement->s);
                        }
                        free(listElement);
                    }
                    free(current);
                }
                return CreateError(EQUATION_ERROR, SYNTAX_UNKNOWN, 0);
            }

            uint16_t string = PoolString();

            strncpy(stringPool[string], equation + i + 1, index - i - 1);

            stringPool[string][index - i - 1] = '\0';

            Atom* newAtom = MallocAtom(VALUE, TYPE_STRING, 0, 0);
            newAtom->data.s = string;
            PushList(&tokens, newAtom);

            i += index - i;
            continue;
        }

        debugPrint("Finding boolean");

        if (strcmp(equation + i, "false") == 0)
        {
            Atom* newAtom = MallocAtom(VALUE, TYPE_FLOAT, 0, 0);
            newAtom->data.f = 0;
            PushList(&tokens, newAtom);
            i += 4;
            continue;
        }
        if (strcmp(equation + i, "true") == 0)
        {
            Atom* newAtom = MallocAtom(VALUE, TYPE_FLOAT, 0, 0);
            newAtom->data.f = 1;
            PushList(&tokens, newAtom);
            i += 3;
            continue;
        }

        debugPrint("Finding operator");

        //  Search through each operator for matches
        bool atomDone = false;
        for (int operator= 0; operator<OPERATOR_COUNT; operator++)
        {
            // debugPrintf("Operator: %s\n",OPERATOR_PRECEDENT_LIST[operator].operator);
            uint8_t operatorLength = strlen(OPERATOR_PRECEDENT_LIST[operator].operator);
            if (i + operatorLength > equationLength)
            {
                continue;
            }
            bool match = true;
            for (int opIndex = 0; opIndex < operatorLength; opIndex++)
            {
                if (equation[i + opIndex] != OPERATOR_PRECEDENT_LIST[operator].operator[opIndex])
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                debugPrint("found operator");

                atomDone = true;

                bool isUnarySubtract = false;

                if (equation[i] == '-')
                {
                    debugPrint("is subtraction");

                    if (tokens.count == 0)
                    {
                        Atom* previousToken = (Atom*)ListGetIndex(&tokens, tokens.count - 1);

                        if (previousToken->atomType == OPERATOR)
                        {
                            debugPrint("is unary");
                            isUnarySubtract = true;

                            char unarySubtractString[2] = { UNARY_SUBTRACT, '\0' };

                            PushList(&tokens, MallocTokenAtom(OPERATOR, OPERATOR_PRECEDENT_LIST[operator].precedence, 1, unarySubtractString));
                        }
                    }
                }

                if (!isUnarySubtract)
                {
                    debugPrint("malloc token");

                    PushList(&tokens,
                        MallocTokenAtom(
                            OPERATOR,
                            OPERATOR_PRECEDENT_LIST[operator].precedence,
                            OPERATOR_PRECEDENT_LIST[operator].parameters,
                            OPERATOR_PRECEDENT_LIST[operator].operator
                        )
                    );
                    debugPrint("done malloc");
                }

                i += operatorLength - 1;
                break;
            }
        }

        if (atomDone)
            continue;

        debugPrint("Finding number");
        //  If the current char is a number, continue searching until the end
        uint8_t numberLength = 0;
        while (i + numberLength < equationLength && IsNumber(equation[i + numberLength]))
        {
            // debugPrintf("%c\n",equation[i+numberLength]);
            numberLength++;
        }
        debugPrintf("Length: %d\n", numberLength);
        if (numberLength != 0)
        {
            char buffer[32];

            debugPrint("before loop");

            for (int numberIndex = 0; numberIndex < numberLength; numberIndex++)
            {
                debugPrint("loop");

                buffer[numberIndex] = equation[i + numberIndex];
            }
            buffer[numberLength] = '\0';

            debugPrint("buffered");

            Atom* newAtom = MallocAtom(VALUE, TYPE_FLOAT, 0, 0);
            debugPrint("malloced");

            newAtom->data.f = atof(buffer);
            PushList(&tokens, newAtom);
            debugPrint("pushed");

            debugPrint("number done");

            i += numberLength - 1;
            continue;
        }

        debugPrint("Finding other");

        if (IsAlphaNumeric(equation[i]))
        {
            debugPrint("alphanumeric");
            Atom* newAtom = MallocAtom(UNKOWN_TOKEN, TYPE_STRING, UNKOWN_PRECEDENCE, 0);

            newAtom->data.s = PoolString();

            debugPrint("allocated resources");

            uint8_t stringIndex = 0;
            while (i + stringIndex < equationLength && IsAlphaNumeric(equation[i + stringIndex]))
            {
                debugPrintf("char: %c to %d\n", equation[i + stringIndex], (int)newAtom->data.s);
                stringPool[newAtom->data.s][stringIndex] = equation[i + stringIndex];
                stringIndex++;
            }

            stringPool[newAtom->data.s][stringIndex] = '\0';

            debugPrint("copied to pool");

            PushList(&tokens, newAtom);

            debugPrint("pushed");

            i += stringIndex - 1;
            continue;
        }
    }

    // DEBUG
#ifdef DEBUG
    for (int i = 0; i < tokens.count; i++)
    {
        Atom* atom = (Atom*)ListGetIndex(&tokens, i);
        uint16_t serialized = SerializeUnion(&atom->data, atom->dataType, false);
        debugPrintf("--------------------- token: %s\n", stringPool[serialized]);
        FreeString(&serialized);
    }
#endif

    ////////////////////////////
    // Shunt yard the tokens
    // When pushing operators, if the previous one is >= precedence, then push it to operand
    // Push rest when done
    ////////////////////////////

    debugPrint("<--------- SHUNT START ------------->");

    while (tokens.count > 0)
    {

        Atom* token = (Atom*)PopListFirst(&tokens);

        debugPrint("Atom shunt yard");

        if (token->atomType == OPERATOR || token->atomType == UNKOWN_TOKEN)
        {
            if (stringPool[token->data.s][0] == ',')
            {
                debugPrint(",");
                // If count > 0, and last is not operator OR previous is not (
                while (
                    operators.count > 0 && (((Atom*)ListGetIndex(&operators, operators.count - 1))->atomType != OPERATOR ||
                        stringPool[((Atom*)ListGetIndex(&operators, operators.count - 1))->data.s][0] != '('))
                {
                    debugPrint("move from parenthesis");

                    PushList(&operands, PopList(&operators));

                    debugPrint("moved to operand stack\n");
                }
                FreeString(&token->data.s);
                free(token);
                // free?
                continue;
            }

            if (stringPool[token->data.s][0] == ')')
            {
                // If the operator is parenthesis, pop off all of the previous operations until you get to the matching one
                debugPrint("parenthesis");
                while (
                    operators.count > 0 && (((Atom*)ListGetIndex(&operators, operators.count - 1))->atomType != OPERATOR ||
                        stringPool[((Atom*)ListGetIndex(&operators, operators.count - 1))->data.s][0] != '('))
                {
                    debugPrint("move from parenthesis");

                    PushList(&operands, PopList(&operators));

                    debugPrint("moved to operand stack\n");
                }
                Atom* popLast = (Atom*)PopList(&operators);
                FreeString(&popLast->data.s);
                free(popLast);
                FreeString(&token->data.s);
                free(token);
            }
            else
            {
                // If operator is not parenthesis, then move operators to the operands IF the precedence is greater or equal, then push operator
                if (token->precedence != 0 && operators.count > 0)
                {
                    debugPrintf("Current: %d, previous %d\n",
                        token->precedence,
                        ((Atom*)ListGetIndex(&operators, operators.count - 1))->precedence);
                    while (operators.count > 0 && ((Atom*)ListGetIndex(&operators, operators.count - 1))->precedence >= token->precedence)
                    {
                        PushList(&operands, PopList(&operators));

                        debugPrint("moved to operand stack");
                    }
                }
                PushList(&operators, token);
                debugPrint("added to operation stack");
            }
        }
        else
        {
            PushList(&operands, token);
            debugPrint("added to operand stack");
        }
    }
    debugPrint("shunt done, pushing remain");
    // push remaining operators to output
    while (operators.count > 0)
    {
        PushList(&operands, PopList(&operators));
        debugPrint("moved to operand stack");
    }

    debugPrintf("operators left: %d\n", operators.count);

    // DEBUG
#ifdef DEBUG
    for (int i = 0; i < operands.count; i++)
    {
        Atom* atom = (Atom*)ListGetIndex(&operands, i);
        uint16_t serialized = SerializeUnion(&atom->data, atom->dataType, false);
        debugPrintf("-------------RPN: %s\n", stringPool[serialized]);
        FreeString(&serialized);
    }
#endif

    Atom* context = MallocAtom(VALUE, NO_TYPE, 0, 0);
    if (scriptData->objectIndex != -1 && scriptData->scriptIndex != -1)
    {
        context->dataType = TYPE_SCRIPT;
        context->data.objIndex = scriptData->objectIndex;
        context->data.scriptIndex = scriptData->scriptIndex;
        debugPrintf("Context, %d:%d\n", context->data.objIndex, context->data.scriptIndex);
    }

    //////////////////////////
    // Solve using the operators
    // Unkown types are retreived using the context var
    // the '.' operator sets the context to the previous atom
    // TODO: Check edge cases
    //////////////////////////

    for (int index = 0; index < operands.count; index++)
    {
        debugPrintf("solving index %d\n", index);
        Atom* currentAtom = (Atom*)ListGetIndex(&operands, index);
        if (currentAtom->atomType == OPERATOR || currentAtom->atomType == UNKOWN_TOKEN)
        {
            debugPrintf("Context, %d:%d\n", context->data.objIndex, context->data.scriptIndex);
            bool isValid = true;

            GeneralList parameters;
            InitializeList(&parameters);

            debugPrintf("Solving: %s, %d\n", stringPool[currentAtom->data.s], currentAtom->parameters);

            for (int checkParam = 0; checkParam < currentAtom->parameters; checkParam++)
            {
                if (index - checkParam - 1 < 0)
                {
                    isValid = false;
                    break;
                }

                Atom* previousAtom = (Atom*)ListGetIndex(&operands, index - checkParam - 1);
                if (previousAtom->atomType == OPERATOR)
                {
                    isValid = false;
                    break;
                }
            }

            if (!isValid)
            {
                continue;
            }

            for (int checkParam = 0; checkParam < currentAtom->parameters; checkParam++)
            {
                debugPrint("push param");
                PushList(&parameters, (Atom*)DeleteListElement(&operands, index - currentAtom->parameters));
            }
            index -= currentAtom->parameters;

            // DEBUG
#ifdef DEBUG
            for (int print = 0; print < parameters.count; print++)
            {
                Atom* atom = (Atom*)ListGetIndex(&parameters, print);
                uint16_t serialized = SerializeUnion(&atom->data, atom->dataType, false);
                debugPrintf("-------------parameters: %s\n", stringPool[serialized]);
                FreeString(&serialized);
            }
#endif

            char operatorString[50];
            strcpy(operatorString, stringPool[currentAtom->data.s]);
            FreeString(&currentAtom->data.s);

            // Branch from parameter count -> context -> type -> operator

            switch (parameters.count)
            {
            case 0:
            {
                if (strcmp(operatorString, "PI") == 0)
                {
                    currentAtom->dataType = TYPE_FLOAT;
                    currentAtom->data.f = 3.14159265459;
                    currentAtom->atomType = VALUE;
                }
                if (strcmp(operatorString, "getPosition") == 0)
                {
                    currentAtom->dataType = TYPE_VECTOR;
                    currentAtom->atomType = VALUE;
                    Vector2 pos = GetObjectDataByName(scriptData->linkedObject, "position")->data.XY;
                    currentAtom->data.XY.x = pos.x;
                    currentAtom->data.XY.y = pos.y;
                }
                if (strcmp(operatorString, "getScale") == 0)
                {
                    currentAtom->dataType = TYPE_VECTOR;
                    currentAtom->atomType = VALUE;
                    Vector2 scale = GetObjectDataByName(scriptData->linkedObject, "scale")->data.XY;
                    currentAtom->data.XY.x = scale.x;
                    currentAtom->data.XY.y = scale.y;
                }

                if (strcmp(operatorString, "getSprite") == 0)
                {
                    currentAtom->dataType = TYPE_INT;
                    currentAtom->atomType = VALUE;
                    currentAtom->data.i = GetObjectDataByName(scriptData->linkedObject, "sprite")->data.i;
                }
                if (strcmp(operatorString, "getVelocity") == 0)
                {
                    currentAtom->dataType = TYPE_VECTOR;
                    currentAtom->atomType = VALUE;
                    Vector2 vel = GetObjectDataByName(scriptData->linkedObject, "velocity")->data.XY;
                    currentAtom->data.XY.x = vel.x;
                    currentAtom->data.XY.y = vel.y;
                }
                if (strcmp(operatorString, "getAngle") == 0)
                {
                    currentAtom->dataType = TYPE_FLOAT;
                    currentAtom->atomType = VALUE;
                    currentAtom->data.f = GetObjectDataByName(scriptData->linkedObject, "angle")->data.f;
                }

                debugPrint("no params");
                if (context->dataType == TYPE_OBJ)
                {
                    debugPrint("is obj context");
                    if (currentAtom->atomType == UNKOWN_TOKEN)
                    {

                        currentAtom->atomType = VALUE;
                        currentAtom->dataType = NO_TYPE;

                        debugPrint("unkown type");
                        debugPrintf("obj name: %s\n", scenes[sceneIndex].objects[context->data.objIndex]->name);

                        EngineVar* variable = GetObjectDataByName(scenes[sceneIndex].objects[context->data.objIndex], operatorString);

                        if (variable != NULL) {
                            debugPrintf("found var, %d\n", variable->currentType);
                            currentAtom->dataType = variable->currentType;
                            currentAtom->data = variable->data;
                            currentAtom->listData = &variable->listData;
                            debugPrintf("variable elemets: %d\n", variable->listData.count);
                        }
                    }
                }
                if (context->dataType == TYPE_SCRIPT)
                {
                    debugPrint("is script context");
                    if (currentAtom->atomType == UNKOWN_TOKEN)
                    {

                        currentAtom->atomType = VALUE;
                        currentAtom->dataType = NO_TYPE;
                        EngineObject* obj = scenes[sceneIndex].objects[context->data.objIndex];
                        debugPrintf("script index: %d\n", context->data.scriptIndex);
                        ScriptData* scrData = (ScriptData*)ListGetIndex(&obj->scriptData, context->data.scriptIndex);

                        debugPrint("unkown type");
                        debugPrintf("obj name: %s\n", obj->name);

                        debugPrintf("scr name %s\n", scrData->script->name);

                        for (int x = 0; x < scrData->variables.count; x++)
                        {
                            EngineVar* variable = ((EngineVar*)ListGetIndex(&scrData->variables, x));
                            debugPrintf("check var: %s\n", variable->name);
                            if (strcmp(operatorString, variable->name) == 0)
                            {
                                debugPrintf("found var, %d\n", variable->currentType);
                                currentAtom->dataType = variable->currentType;
                                currentAtom->data = variable->data;
                                currentAtom->listData = &variable->listData;
                                debugPrintf("variable elemets: %d\n", variable->listData.count);

                                break;
                            }
                        }

                        for (int x = 0; x < scrData->functionCount; x++)
                        {
                            EngineFunction* function = scrData->functions[x];
                            debugPrintf("check function: %s\n", function->name);
                            if (strcmp(operatorString, function->name) == 0)
                            {
                                JumpToFunction(scrData, function->name);
                                break;
                            }
                        }
                    }
                }
                if (context->listData != NULL)
                {
                    if (strcmp(operatorString, "pop") == 0)
                    {
                        debugPrintf("context elemets before pop: %d\n", context->listData->count);
                        VariableUnion* popValue = (VariableUnion*)PopList(context->listData);

                        currentAtom->atomType = VALUE;
                        currentAtom->dataType = context->dataType;
                        currentAtom->data = *popValue;

                        free(popValue);

                        debugPrintf("context elemets after pop: %d\n", context->listData->count);
                    }
                }
            }
            break;
            case 1:
            {
                Atom* parameter0 = (Atom*)ListGetIndex(&parameters, 0);
                if (strcmp(operatorString, ".") == 0)
                {
                    debugPrint("set context");
                    context->dataType = parameter0->dataType;
                    context->data = parameter0->data;
                    context->listData = parameter0->listData;
                    if (context->listData != NULL)
                        debugPrintf("parameter elemets: %d\n", parameter0->listData->count);
                    else
                        debugPrint("context is not list");
                    FreeString(&currentAtom->data.s);
                    free(DeleteListElement(&operands, index));
                    break;
                }

                if (parameter0->dataType == TYPE_STRING)
                {
                    if (strcmp(operatorString, "input") == 0)
                    {
                        debugPrint("input operation");
                        currentAtom->dataType = TYPE_FLOAT;
                        currentAtom->atomType = VALUE;
                        switch (ToLower(stringPool[parameter0->data.s][0]))
                        {
                        case 'w':
                            debugPrintf("W state: %d\n", (gpio_get(BUTTON_W) == 0 ? 1 : 0));
                            currentAtom->data.f = (gpio_get(BUTTON_W) == 0);
                            break;
                        case 'd':
                            currentAtom->data.f = (gpio_get(BUTTON_D) == 0);
                            break;
                        case 'a':
                            currentAtom->data.f = (gpio_get(BUTTON_A) == 0);
                            break;
                        case 's':
                            currentAtom->data.f = (gpio_get(BUTTON_S) == 0);
                            break;

                        case 'i':
                            currentAtom->data.f = (gpio_get(BUTTON_I) == 0);
                            break;
                        case 'j':
                            currentAtom->data.f = (gpio_get(BUTTON_J) == 0);
                            break;
                        case 'k':
                            currentAtom->data.f = (gpio_get(BUTTON_K) == 0);
                            break;
                        case 'l':
                            currentAtom->data.f = (gpio_get(BUTTON_L) == 0);
                            break;
                        default:
                            currentAtom->data.f = -1;
                            break;
                        }
                    }
                    if (strcmp(operatorString, "ObjectByName") == 0)
                    {
                        currentAtom->dataType = NO_TYPE;
                        currentAtom->atomType = VALUE;

                        for (int x = 0; x < scenes[sceneIndex].objectCount; x++)
                        {
                            if (strcmp(stringPool[parameter0->data.s], scenes[sceneIndex].objects[x]->name) == 0)
                            {
                                debugPrint("found obj");
                                currentAtom->dataType = TYPE_OBJ;
                                currentAtom->data.objIndex = x;
                                break;
                            }
                        }
                    }
                }
                if (parameter0->dataType == TYPE_VECTOR)
                {
                    currentAtom->dataType = NO_TYPE;
                    currentAtom->atomType = VALUE;
                    if (strcmp(operatorString, "setPosition") == 0 && scriptData->linkedObject != NULL)
                    {
                        debugPrintf("set pos (%f,%f)\n", parameter0->data.XY.x, parameter0->data.XY.y);

                        GetObjectDataByName(scriptData->linkedObject, "position")->data.XY.x = parameter0->data.XY.x;
                        GetObjectDataByName(scriptData->linkedObject, "position")->data.XY.y = parameter0->data.XY.y;

                        debugPrint("set");
                    }
                    if (strcmp(operatorString, "setScale") == 0 && scriptData->linkedObject != NULL && GetObjectDataByName(scriptData->linkedObject, "scale")->currentType == TYPE_VECTOR)
                    {
                        GetObjectDataByName(scriptData->linkedObject, "scale")->data.XY.x = parameter0->data.XY.x;
                        GetObjectDataByName(scriptData->linkedObject, "scale")->data.XY.y = parameter0->data.XY.y;
                    }

                    if (strcmp(operatorString, "setVelocity") == 0 && scriptData->linkedObject != NULL)
                    {
                        GetObjectDataByName(scriptData->linkedObject, "velocity")->data.XY.x = parameter0->data.XY.x;
                        GetObjectDataByName(scriptData->linkedObject, "velocity")->data.XY.y = parameter0->data.XY.y;
                    }

                    if (strcmp(operatorString, "addPosition") == 0 && scriptData->linkedObject != NULL)
                    {
                        GetObjectDataByName(scriptData->linkedObject, "position")->data.XY.x += parameter0->data.XY.x;
                        GetObjectDataByName(scriptData->linkedObject, "position")->data.XY.y += parameter0->data.XY.y;
                        printf("new pos (%f,%f)\n",
                            GetObjectDataByName(scriptData->linkedObject, "position")->data.XY.x,
                            GetObjectDataByName(scriptData->linkedObject, "position")->data.XY.y);
                    }
                    if (strcmp(operatorString, "addScale") == 0 && scriptData->linkedObject != NULL && GetObjectDataByName(scriptData->linkedObject, "scale")->currentType == TYPE_VECTOR)
                    {
                        GetObjectDataByName(scriptData->linkedObject, "scale")->data.XY.x += parameter0->data.XY.x;
                        GetObjectDataByName(scriptData->linkedObject, "scale")->data.XY.y += parameter0->data.XY.y;
                    }

                    if (strcmp(operatorString, "addVelocity") == 0 && scriptData->linkedObject != NULL)
                    {
                        GetObjectDataByName(scriptData->linkedObject, "velocity")->data.XY.x += parameter0->data.XY.x;
                        GetObjectDataByName(scriptData->linkedObject, "velocity")->data.XY.y += parameter0->data.XY.y;
                    }
                }
                if (parameter0->dataType == TYPE_FLOAT)
                {
                    currentAtom->atomType = VALUE;
                    currentAtom->dataType = TYPE_FLOAT;
                    if (strcmp(operatorString, "cos") == 0)
                    {
                        debugPrint("COS OPERATION");
                        currentAtom->data.f = cos(parameter0->data.f);
                    }
                    if (strcmp(operatorString, "sin") == 0)
                    {
                        debugPrint("SIN OPERATION");
                        currentAtom->data.f = sin(parameter0->data.f);
                    }

                    if (strcmp(operatorString, "setScale") == 0 && scriptData->linkedObject != NULL && GetObjectDataByName(scriptData->linkedObject, "scale")->currentType == TYPE_INT)
                    {
                        currentAtom->dataType = NO_TYPE;

                        GetObjectDataByName(scriptData->linkedObject, "scale")->data.i = parameter0->data.f;
                    }

                    if (strcmp(operatorString, "addScale") == 0 && scriptData->linkedObject != NULL && GetObjectDataByName(scriptData->linkedObject, "scale")->currentType == TYPE_INT)
                    {
                        currentAtom->dataType = NO_TYPE;

                        GetObjectDataByName(scriptData->linkedObject, "scale")->data.i = parameter0->data.f;
                    }

                    if (strcmp(operatorString, "setSprite") == 0 && scriptData->linkedObject != NULL)
                    {
                        currentAtom->dataType = NO_TYPE;
                        GetObjectDataByName(scriptData->linkedObject, "sprite")->data.i = (int)parameter0->data.f;
                    }

                    if (strcmp(operatorString, "setAngle") == 0 && scriptData->linkedObject != NULL)
                    {
                        currentAtom->dataType = NO_TYPE;
                        GetObjectDataByName(scriptData->linkedObject, "angle")->data.f = (float)parameter0->data.f;
                    }
                    if (strcmp(operatorString, "addAngle") == 0 && scriptData->linkedObject != NULL)
                    {
                        currentAtom->dataType = NO_TYPE;
                        GetObjectDataByName(scriptData->linkedObject, "angle")->data.f += (float)parameter0->data.f;
                    }

                    if (strcmp(operatorString, "leftLED") == 0 && scriptData->linkedObject != NULL)
                    {
                        currentAtom->dataType = NO_TYPE;
                        gpio_put(LEFT_LIGHT, parameter0->data.f != 0);
                    }
                    if (strcmp(operatorString, "rightLED") == 0 && scriptData->linkedObject != NULL)
                    {
                        currentAtom->dataType = NO_TYPE;
                        gpio_put(RIGHT_LIGHT, parameter0->data.f != 0);
                    }

                    if (operatorString[0] == UNARY_SUBTRACT)
                    {
                        debugPrint("UNARY NEGATE OP");
                        currentAtom->data.f = -parameter0->data.f;
                    }
                }
                if (context->dataType == TYPE_OBJ)
                {
                    if (parameter0->dataType == TYPE_STRING)
                    {
                        if (strcmp(operatorString, "ScriptByName") == 0)
                        {
                            currentAtom->atomType = VALUE;
                            currentAtom->dataType = NO_TYPE;

                            EngineObject* obj = scenes[sceneIndex].objects[context->data.objIndex];


                            for (int x = 0; x < obj->scriptData.count; x++)
                            {
                                ScriptData* scrData = ((ScriptData*)ListGetIndex(&obj->scriptData, x));

                                debugPrintf("script: %s\n", scrData->script->name);
                                if (strcmp(stringPool[parameter0->data.s], scrData->script->name) == 0)
                                {
                                    currentAtom->dataType = TYPE_SCRIPT;

                                    debugPrintf("got script: %d:%d", context->data.objIndex, x);
                                    currentAtom->data.objIndex = context->data.objIndex;
                                    currentAtom->data.scriptIndex = x;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (context->listData != NULL)
                {
                    debugPrint("has list context");
                    if (strcmp(operatorString, "push") == 0)
                    {
                        currentAtom->atomType = VALUE;
                        currentAtom->dataType = context->dataType;
                        currentAtom->listData = context->listData;

                        debugPrintf("context elemets: %d\n", context->listData->count);

                        VariableUnion* copy = malloc(sizeof(VariableUnion));
                        if (currentAtom->dataType == TYPE_INT && parameter0->dataType == TYPE_FLOAT)
                        {
                            (*copy).i = (int)parameter0->data.f;
                        }
                        else if (currentAtom->dataType == TYPE_FLOAT && parameter0->dataType == TYPE_INT)
                        {
                            (*copy).f = (float)parameter0->data.i;
                        }
                        else
                        {
                            (*copy) = parameter0->data;
                        }

                        PushList(currentAtom->listData, copy);
                        for (int list = 0; list < currentAtom->listData->count; list++)
                        {
                            VariableUnion* getUnion = (VariableUnion*)ListGetIndex(currentAtom->listData, list);
                            uint16_t serialized = SerializeUnion(getUnion, currentAtom->dataType, false);
                            debugPrintf("---list elements: %s\n", stringPool[serialized]);
                            FreeString(&serialized);
                        }
                    }
                    if (strcmp(operatorString, "seek") == 0 && (parameter0->dataType == TYPE_INT || parameter0->dataType == TYPE_FLOAT))
                    {
                        VariableUnion* seekValue = NULL;
                        if (parameter0->dataType == TYPE_INT)
                        {
                            seekValue = (VariableUnion*)ListGetIndex(context->listData, parameter0->data.i);
                        }
                        else
                        {
                            seekValue = (VariableUnion*)ListGetIndex(context->listData, (int)parameter0->data.f);
                        }

                        currentAtom->atomType = VALUE;
                        currentAtom->dataType = context->dataType;
                        currentAtom->data = *seekValue;
                    }

                    if (strcmp(operatorString, "contains") == 0)
                    {
                        debugPrint("contain operator");
                        bool isFound = false;
                        VariableUnion* seekValue = NULL;

#ifdef DEBUG
                        uint16_t serialize = SerializeUnion(&parameter0->data, parameter0->dataType, false);
                        debugPrintf("find value: %s\n", stringPool[serialize]);
                        FreeString(&serialize);
                        debugPrintf("context count: %d\n", context->listData->count);
#endif

                        for (int seek = 0; seek < context->listData->count; seek++)
                        {
                            seekValue = (VariableUnion*)ListGetIndex(context->listData, seek);

#ifdef DEBUG
                            uint16_t serialize = SerializeUnion(seekValue, context->dataType, false);
                            debugPrintf("list value: %s\n", stringPool[serialize]);
                            FreeString(&serialize);
#endif

                            if (DataMatch(seekValue, context->dataType, &parameter0->data, parameter0->dataType))
                            {
                                debugPrintf("Matched parameter with index %d\n", seek);
                                isFound = true;
                                break;
                            }
                        }

                        currentAtom->atomType = VALUE;
                        currentAtom->dataType = TYPE_FLOAT;
                        currentAtom->data.f = isFound ? 1 : 0;
                    }
                }
            }
            break;
            case 2:
            {
                Atom* parameter0 = (Atom*)ListGetIndex(&parameters, 0);
                Atom* parameter1 = (Atom*)ListGetIndex(&parameters, 1);
                currentAtom->atomType = VALUE;

                if (parameter0->dataType == TYPE_INT) {
                    parameter0->data.f = parameter0->data.i;
                    parameter0->dataType = TYPE_FLOAT;
                }
                if (parameter1->dataType == TYPE_INT) {
                    parameter1->data.f = parameter1->data.i;
                    parameter0->dataType = TYPE_FLOAT;
                }

                if (strcmp(operatorString, "+") == 0)
                {
                    debugPrint("add");
                    if (parameter0->dataType == TYPE_FLOAT && parameter1->dataType == TYPE_FLOAT)
                    {
                        debugPrint("type float");
                        currentAtom->data.f = parameter0->data.f + parameter1->data.f;
                        currentAtom->dataType = TYPE_FLOAT;
                    }
                    if (parameter0->dataType == TYPE_STRING && parameter1->dataType == TYPE_STRING)
                    {
                        debugPrint("type string");
                        sprintf(operatorString, "%s%s", stringPool[parameter0->data.s], stringPool[parameter1->data.s]);

                        debugPrintf("concat result: %s\n", operatorString);
                        // free(parameter0->data.s);
                        // free(parameter1->data.s);
                        currentAtom->dataType = TYPE_STRING;
                    }
                    if (parameter0->dataType == TYPE_VECTOR && parameter1->dataType == TYPE_VECTOR)
                    {
                        currentAtom->data.XY.x = parameter0->data.XY.x + parameter1->data.XY.x;
                        currentAtom->data.XY.y = parameter0->data.XY.y + parameter1->data.XY.y;
                        currentAtom->dataType = TYPE_VECTOR;
                    }
                }

                if (parameter0->dataType == TYPE_FLOAT && parameter1->dataType == TYPE_FLOAT)
                {
                    debugPrintf("FLOAT OP: %s\n", operatorString);
                    currentAtom->dataType = TYPE_FLOAT;
                    currentAtom->atomType = VALUE;
                    if (strcmp(operatorString, "-") == 0)
                    {
                        currentAtom->data.f = parameter0->data.f - parameter1->data.f;
                    }
                    if (strcmp(operatorString, "*") == 0)
                    {
                        currentAtom->data.f = parameter0->data.f * parameter1->data.f;
                    }
                    if (strcmp(operatorString, "/") == 0)
                    {
                        currentAtom->data.f = parameter0->data.f / parameter1->data.f;
                    }
                    if (strcmp(operatorString, "%") == 0)
                    {
                        currentAtom->data.f = (int)parameter0->data.f % (int)parameter1->data.f;
                    }

                    if (strcmp(operatorString, "pow") == 0)
                    {
                        currentAtom->data.f = pow(parameter0->data.f, parameter1->data.f);
                    }

                    if (strcmp(operatorString, "|") == 0)
                    {
                        currentAtom->data.f = (float)((int)parameter0->data.f | (int)parameter1->data.f);
                    }
                    if (strcmp(operatorString, "&") == 0)
                    {
                        currentAtom->data.f = (float)((int)parameter0->data.f & (int)parameter1->data.f);
                    }
                    if (strcmp(operatorString, "^") == 0)
                    {
                        currentAtom->data.f = (float)((int)parameter0->data.f ^ (int)parameter1->data.f);
                    }

                    if (strcmp(operatorString, "==") == 0)
                    {
                        currentAtom->data.f = (parameter0->data.f == parameter1->data.f) ? 1 : 0;
                    }
                    if (strcmp(operatorString, "<") == 0)
                    {
                        currentAtom->data.f = (parameter0->data.f < parameter1->data.f) ? 1 : 0;
                    }
                    if (strcmp(operatorString, ">") == 0)
                    {
                        currentAtom->data.f = (parameter0->data.f > parameter1->data.f) ? 1 : 0;
                    }
                    if (strcmp(operatorString, "<=") == 0)
                    {
                        currentAtom->data.f = (parameter0->data.f <= parameter1->data.f) ? 1 : 0;
                    }
                    if (strcmp(operatorString, ">=") == 0)
                    {
                        currentAtom->data.f = (parameter0->data.f >= parameter1->data.f) ? 1 : 0;
                    }
                    if (strcmp(operatorString, "!=") == 0)
                    {
                        currentAtom->data.f = (parameter0->data.f != parameter1->data.f) ? 1 : 0;
                    }

                    if (strcmp(operatorString, "Vector") == 0)
                    {
                        currentAtom->dataType = TYPE_VECTOR;
                        currentAtom->data.XY.x = parameter0->data.f;
                        currentAtom->data.XY.y = parameter1->data.f;
                    }
                }
            }
            break;
            }

            while (parameters.count > 0)
            {
                debugPrintf("parameters: %d\n", parameters.count);
                Atom* param = PopList(&parameters);
                debugPrint("popped param");
                if (param->dataType == TYPE_STRING)
                {
                    debugPrint("is string");
                    FreeString(&param->data.s);
                    debugPrint("cleared string");
                }
                free(param);
                debugPrint("cleared atom");
            }
            index = -1;

            // DEBUG
#ifdef DEBUG
            for (int x = 0; x < operands.count; x++)
            {

                Atom* atom = (Atom*)ListGetIndex(&operands, x);
                if (atom->listData != NULL)
                {
                    debugPrintf("is List! elemetn count: %d\n", atom->listData->count);
                }
                else
                {
                    debugPrint("not list");
                }
                debugPrintf("operand type: %d\n", atom->dataType);
                uint16_t serialized = SerializeUnion(&atom->data, atom->dataType, false);
                debugPrintf("-------------OUT: %s\n", stringPool[serialized]);
                FreeString(&serialized);
            }
#endif
        }
    }

    debugPrint("solved");

    ///////////////////////////
    // Solved
    // Output the first operator
    // Clear any leftover values
    ///////////////////////////

    if (operands.count == 0)
    {
        output->currentType = NO_TYPE;
        if (context->dataType == TYPE_STRING)
        {
            FreeString(&context->data.s);
        }
        return 0;
    }

    Atom* outputAtom = (Atom*)PopListFirst(&operands);
    debugPrint("popped");

    output->currentType = outputAtom->dataType;
    output->data = outputAtom->data;
    debugPrint("basic copied");
    if (outputAtom->listData != NULL)
    {
        // CpyList(&output->listData, outputAtom->listData, sizeof(VariableUnion));
        for (int i = 0; i < outputAtom->listData->count; i++)
        {

            VariableUnion* valueToCopy = (VariableUnion*)ListGetIndex(outputAtom->listData, i);

            VariableUnion* cpy = malloc(sizeof(VariableUnion));
            *cpy = *valueToCopy;

            PushList(&output->listData, cpy);
        }
        output->listData.count = outputAtom->listData->count;
    }

    debugPrint("copied list");

    free(outputAtom);

    debugPrint("set output");

    debugPrint("pop operands");

    while (operands.count > 0)
    {
        debugPrint("pop");
        Atom* current = (Atom*)PopList(&operands);
        if (current->dataType == TYPE_STRING)
        {
            FreeString(&current->data.s);
        }
        while (current->listData != NULL && current->listData->count > 0)
        {
            debugPrint("pop content");
            VariableUnion* listElement = (VariableUnion*)PopList(current->listData);
            if (current->dataType == TYPE_STRING)
            {
                FreeString(&listElement->s);
            }
            free(listElement);
        }
        free(current);
    }

    debugPrint("done");
    debugPrint("free context");

    if (context->dataType == TYPE_STRING)
    {
        FreeString(&context->data.s);
    }
    free(context);

    debugPrint("done");

    return 0;
}

/*char *RemoveSpaces(char *in)
{
    char *out = (char *)malloc(1 + strlen(in));
    int i = 0;
    int writeIndex = 0;
    while (i < strlen(in))
    {
        if (in[i] > 32)
        {
            out[writeIndex++] = in[i];
        }
        i++;
    }
    out[writeIndex] = '\0';
    return out;
}*/

// DOES NOT FREE INPUT AND OUTPUT IS MALLOCED
uint16_t LeftTrim(char* str)
{
    int index = 0;
    while (str[index] <= 32 && str[index] != '\0')
    {
        index++;
        debugPrint("trim");
    }
    if (str[index] == '\0')
    {
        return NULL_POOL;
    }

    uint16_t output = PoolString();

    strcpy(stringPool[output], str + index);
    debugPrintf("trim %s\n", str + index);
    return output;
}

uint8_t GetTypeFromString(char* line)
{

    if (indexOf("bool", line) == 0)
    {
        return TYPE_BOOL;
    }
    if (indexOf("int", line) == 0)
    {
        return TYPE_INT;
    }
    if (indexOf("float", line) == 0)
    {
        return TYPE_FLOAT;
    }
    if (indexOf("string", line) == 0)
    {
        return TYPE_STRING;
    }
    if (indexOf("vector", line) == 0)
    {
        return TYPE_VECTOR;
    }
    if (indexOf("object", line) == 0)
    {
        return TYPE_OBJ;
    }
    return NO_TYPE;
}

uint32_t DeclareEntity(ScriptData* output, uint16_t l, bool declareFunctions)
{
    uint16_t trimmedLine = LeftTrim(output->lines[l]);
    if (trimmedLine == NULL_POOL)
    {
        FreeString(&trimmedLine);
        return CreateError(DATATYPE_ERROR, SYNTAX_UNKNOWN, 0);
    }
    debugPrintf("trimmed line: %s\n", stringPool[trimmedLine]);

    int varType = GetTypeFromString(stringPool[trimmedLine]);
    debugPrintf("got type %d\n", varType);
    if (varType != NO_TYPE)
    {
        int index = 0;
        // Move index to space after type
        while (stringPool[trimmedLine][index] > 32) // if it is character
        {
            index++;
        }
        if (stringPool[trimmedLine][index] == '\0')
        {
            FreeString(&trimmedLine);
            return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
        }
        index++;

        uint8_t nameLength = 0;
        while (
            IsAlphaNumeric(stringPool[trimmedLine][index + nameLength])) // if it is character
        {
            nameLength++;
        }
        if (stringPool[trimmedLine][index + nameLength] == '\0')
        {
            FreeString(&trimmedLine);
            return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
        }
        uint16_t entityName = PoolString();

        strncpy(stringPool[entityName], stringPool[trimmedLine] + index, nameLength);
        stringPool[entityName][nameLength] = '\0';

        debugPrintf("Type: %d, Name: %s\n", varType, stringPool[entityName]);
        index += nameLength;

        debugPrintf("next char: %c\n", stringPool[trimmedLine][index]);

        while (stringPool[trimmedLine][index] <= 32) // if it is not a character
        {
            index++;
        }
        if (stringPool[trimmedLine][index] == '\0')
        {
            FreeString(&entityName);
            FreeString(&trimmedLine);
            return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
        }

        if (stringPool[trimmedLine][index] == '(')
        { // function declare
            if (!declareFunctions)
            {
                FreeString(&entityName);
                FreeString(&trimmedLine);
                return 0;
            }

            debugPrint("declare function");
            index++;
            EngineFunction* newFunction = (EngineFunction*)malloc(sizeof(EngineFunction));
            newFunction->parameterIndex = 0;
            while (stringPool[trimmedLine][index] != ')' && stringPool[trimmedLine][index] != '\0')
            {
                while (stringPool[trimmedLine][index] <= 32)
                {
                    index++;
                }

                uint8_t parameterType = GetTypeFromString(stringPool[trimmedLine] + index);

                debugPrintf("parameter type: %d\n", parameterType);

                // loop until space after type
                while (stringPool[trimmedLine][index] > 32)
                {
                    index++;
                }
                if (stringPool[trimmedLine][index] == '\0')
                {
                    FreeString(&trimmedLine);
                    FreeString(&entityName);
                    free(newFunction);
                    return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
                }
                index++;

                debugPrint("data space");

                // get length
                uint8_t parameterNameLength = 0;
                while (IsAlphaNumeric(stringPool[trimmedLine][index + parameterNameLength])) // if it is character
                {
                    parameterNameLength++;
                }
                if (stringPool[trimmedLine][index + parameterNameLength] == '\0')
                {
                    FreeString(&trimmedLine);
                    FreeString(&entityName);
                    free(newFunction);
                    return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
                }
                debugPrint("got param len");
                // copy name and type to parameter
                newFunction->parameters[newFunction->parameterIndex].parameterName = (char*)malloc(parameterNameLength + 1);
                strncpy(
                    newFunction->parameters[newFunction->parameterIndex].parameterName,
                    stringPool[trimmedLine] + index,
                    parameterNameLength);
                newFunction->parameters[newFunction->parameterIndex].parameterName[parameterNameLength] = '\0';
                newFunction->parameters[newFunction->parameterIndex].parameterType = parameterType;
                debugPrintf("Param Index: %d, Type %d, Name %s\n",
                    newFunction->parameterIndex,
                    newFunction->parameters[newFunction->parameterIndex].parameterType,
                    newFunction->parameters[newFunction->parameterIndex].parameterName);
                newFunction->parameterIndex++;
                index += parameterNameLength;
                while (stringPool[trimmedLine][index] < 32 && stringPool[trimmedLine][index] != '\0')
                {
                    index++;
                }
                if (stringPool[trimmedLine][index] == ',')
                {
                    index++;
                }
                debugPrintf("rest of line: %s\n", stringPool[trimmedLine] + index);
            }
            while (stringPool[trimmedLine][index] != '{' && stringPool[trimmedLine][index] != '\0')
            {
                index++;
            }
            debugPrintf("line index: %d", output->lineIndexes[l]);
            if (stringPool[trimmedLine][index] != '{')
            {
                FreeString(&entityName);
                free(newFunction);
                FreeString(&trimmedLine);
                return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
            }

            debugPrintf("bracket pos: %d\n", l);
            newFunction->line = l;

            output->functions[output->functionCount] = newFunction;
            output->functions[output->functionCount]->name = (char*)malloc(sizeof(char) * (strlen(stringPool[entityName]) + 1));
            strcpy(output->functions[output->functionCount]->name, stringPool[entityName]);
            FreeString(&entityName);
            output->functionCount++;
            debugPrint("declare fin");

            FreeString(&trimmedLine);
            return OPPERATION_SUCCESS;
        }
        else
        { // variable declare
            EngineVar* newVariable = malloc(sizeof(EngineVar));

            newVariable->currentType = varType;
            strncpy(newVariable->name, stringPool[entityName], MAX_NAME_LENGTH - 1);
            newVariable->name[MAX_NAME_LENGTH - 1] = '\0';

            newVariable->scope = output->currentScope;

            printf("new var: %s\n", newVariable->name);

            while (stringPool[trimmedLine][index] <= 32 && stringPool[trimmedLine][index] != '\0')
            {
                index++;
            }
            if (stringPool[trimmedLine][index] != '=')
            {
                FreeString(&trimmedLine);
                FreeString(&entityName);
                return OPPERATION_SUCCESS;
            }
            index++;
            while (stringPool[trimmedLine][index] <= 32 && stringPool[trimmedLine][index] != '\0')
            {
                index++;
            }
            if (stringPool[trimmedLine][index] == '\0')
            {
                FreeString(&trimmedLine);
                FreeString(&entityName);
                return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
            }

            debugPrintf("right side: %s\n", stringPool[trimmedLine] + index);

            printf("var name before pool: %s\n", newVariable->name);

            uint16_t assignValue = PoolVar("");
            printf("var name before shunt: %s\n", newVariable->name);

            uint32_t error = ShuntYard(stringPool[trimmedLine] + index, strlen(stringPool[trimmedLine] + index), &varPool[assignValue], output);
            printf("var name after shunt: %s\n", newVariable->name);
            if (error != 0)
            {
                FreeString(&trimmedLine);
                FreeString(&entityName);
                FreeVar(&assignValue);
                return error | ((uint16_t)l & 0xFFFF);
            }
            debugPrintf("TYPE to %d, from %d\n", newVariable->currentType, varPool[assignValue].currentType);
            if (newVariable->currentType == TYPE_INT)
            {
                if (varPool[assignValue].currentType == TYPE_INT)
                {
                    debugPrintf("indexes: %d, %d\n", output->variables.count, assignValue);
                    debugPrintf("set to %s, from %d\n", newVariable->name, varPool[assignValue].data.i);
                    newVariable->data.i = varPool[assignValue].data.i;
                }
                else if (varPool[assignValue].currentType == TYPE_FLOAT)
                {

                    newVariable->data.i = (int)varPool[assignValue].data.f;
                }
                else
                {
                    FreeString(&trimmedLine);
                    FreeString(&entityName);
                    FreeVar(&assignValue);
                    return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
                }
            }
            else if (newVariable->currentType == TYPE_FLOAT)
            {
                if (varPool[assignValue].currentType == TYPE_INT)
                {
                    debugPrintf("indexes: %d, %d\n", output->variables.count, assignValue);
                    debugPrintf("set to %s, from %d\n", newVariable->name, varPool[assignValue].data.i);
                    float floatVal = (float)varPool[assignValue].data.i;
                    newVariable->data.f = floatVal;
                }
                else if (varPool[assignValue].currentType == TYPE_FLOAT)
                {

                    newVariable->data.f = varPool[assignValue].data.f;
                }
                else
                {
                    FreeString(&trimmedLine);
                    FreeString(&entityName);
                    FreeVar(&assignValue);
                    return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
                }
            }
            else if (EqualType(&varPool[assignValue], newVariable, TYPE_STRING))
            {
                FreeString(&newVariable->data.s);
                newVariable->data.s = PoolString();
                strcpy(stringPool[newVariable->data.s], stringPool[varPool[assignValue].data.s]);
                FreeString(&varPool[assignValue].data.s);
            }
            else if (EqualType(&varPool[assignValue], newVariable, TYPE_VECTOR))
            {
                newVariable->data.XY.x = varPool[assignValue].data.XY.x;
                newVariable->data.XY.y = varPool[assignValue].data.XY.y;
            }

            FreeVar(&assignValue);

            InitializeList(&newVariable->listData);

            debugPrintf("variable declared: %s=type %d\n", newVariable->name, newVariable->currentType);
            PushList(&output->variables, newVariable);

            FreeString(&trimmedLine);
            FreeString(&entityName);
            return OPPERATION_SUCCESS;
        }
        FreeString(&entityName);
    }

    FreeString(&trimmedLine);
    return 0;
}

uint8_t GetScope(ScriptData* scriptData, uint16_t line)
{
    uint8_t level = 0;

    for (int i = 0; i < scriptData->bracketPairs; i++)
    {
        if (
            scriptData->brackets[i].bracketType == CURLY_BRACKET &&
            (scriptData->lineIndexes[line] > scriptData->brackets[i].start) &&
            (scriptData->lineIndexes[line] < scriptData->brackets[i].end))
        {
            level++;
        }
    }
    debugPrintf("line: %d, scope: %d\n", line, level);
    return level;
}

uint32_t SetScriptData(EngineScript* script, ScriptData* output, uint8_t scopeLevel)
{

    uint32_t error = 0;

    output->currentScope = 0;

    SplitScript(script, output);
    /*for (int i = 0; i < output->lineCount; i++)
    {
        debugPrintf("split done line num %d\n", i);
        debugPrintf("split done line: %s\n", output->lines[i]);
    }*/

    error = CalculateBrackets(script, output);
    if (error != 0)
    {
        return error;
    }

    for (int l = 0; l < output->lineCount; l++)
    {
        debugPrintf("setscr %d\n", l);
        // Find and add all variables/functions to the script data
        /*for (int i = 0; i < output->lineCount; i++)
        {
            debugPrintf("split done line num %d\n", i);
            debugPrintf("split done line: %s\n", output->lines[i]);
        }*/

        if (GetScope(output, l) != scopeLevel)
        {
            continue;
        }

        error = DeclareEntity(output, l, true);

        if (error != 0 && error != OPPERATION_SUCCESS)
        {
            debugPrint("set scr error");
            return error;
        }
    }
    debugPrint("set script done");

    output->currentScope = 0;

    return 0;
}

// Frees all script lines, line indexes, brackets, and functions
void FreeScriptData(ScriptData* scriptData, bool onlyFreeContent)
{

    if (scriptData->lineIndexes != NULL) {
        free(scriptData->lineIndexes);
        debugPrint("cleared line indexes");
    }

    if (scriptData->lines != NULL) {
        for (int i = 0; i < scriptData->lineCount; i++)
        {
            free(scriptData->lines[i]);
        }
        debugPrint("cleared scrdatalines");

        free(scriptData->lines);
        debugPrint("cleared line array");
    }


    if (scriptData->brackets != NULL)
        free(scriptData->brackets);
    debugPrint("cleared brackets");

    // debugPrint("free scr data 1");

    for (int i = 0; i < scriptData->functionCount; i++)
    {
        free(scriptData->functions[i]->name);
        debugPrint("cleared function name");
        // debugPrint("free scr data 1a");
        free(scriptData->functions[i]);
        debugPrint("cleared function");
        // debugPrint("free scr data 1b");
    }
    while (scriptData->variables.count > 0)
    {
        EngineVar* var = (EngineVar*)PopList(&scriptData->variables);
        if (var->currentType == TYPE_STRING)
            FreeString(&var->data.s);
        free(var);
        debugPrint("cleared var");
    }
    // debugPrint("free scr data 3");
    scriptData->lineCount = 0;
    scriptData->bracketPairs = 0;
    scriptData->functionCount = 0;

    if (!onlyFreeContent)
        free(scriptData);
    debugPrint("free done");
}

/*#define PATH_VARIABLE 0
#define PATH_VECTOR 1
#define PATH_SCRIPT 2
#define PATH_OBJECT 3
#define PATH_NONE 4

void AssignToPath(char *path, ScriptData *scriptData, EngineVar *assignValue)
{
    GeneralList propertySizes;
    InitializeList(&propertySizes);

    uint8_t length = 0;
    for (int i = 0; i < strlen(path); i++)
    {
        length++;
        if (path[i] == '.')
        {
            uint8_t *temp = malloc(sizeof(uint8_t));
            *temp = length;
            PushList(&propertySizes, temp);
            debugPrintf("%d\n", length);
            length = 0;
        }
    }

    length++;
    uint8_t *temp = malloc(sizeof(uint8_t));
    *temp = length;
    PushList(&propertySizes, temp);
    debugPrintf("%d\n", length);
    length = 0;

    char **properties = (char **)malloc(sizeof(char *) * propertySizes.count);

    int pathIndex = 0;
    for (int i = 0; i < propertySizes.count; i++)
    {
        uint8_t *size = (uint8_t *)ListGetIndex(&propertySizes, i);
        char *property = malloc(*size);

        for (int x = 0; x < *size - 1; x++)
        {
            property[x] = path[pathIndex++];
        }
        property[*size - 1] = '\0';

        debugPrintf("%s\n", property);

        properties[i] = property;
        pathIndex++;

        free(size);
    }

}*/

uint32_t ExecuteLine(EngineScript* script, ScriptData* scriptData, uint16_t currentLine)
{

    if (currentLine >= scriptData->lineCount)
    {
        return 0;
    }


    bool justShuntYard = true;

    uint16_t trimmedLine = LeftTrim(scriptData->lines[currentLine]);
    debugPrintf("Exexute line: %s\n", stringPool[trimmedLine]);

    // If line is a variable or function declaration, declare entities
    if (GetTypeFromString(stringPool[trimmedLine]) != NO_TYPE)
    {
        DeclareEntity(scriptData, currentLine, false);
        FreeString(&trimmedLine);
        PushLine(scriptData, currentLine + 1);
        return 0;
    }

    // If line only includes end of a curly bracket, just pop, do not push next line and free variables
    if (stringPool[trimmedLine][0] == '}')
    {
        for (int i = 0; i < scriptData->variables.count; i++)
        {
            EngineVar* variable = (EngineVar*)ListGetIndex(&scriptData->variables, i);

            if (variable->scope == scriptData->currentScope)
            {
                if (variable->currentType == TYPE_STRING)
                {
                    FreeString(&variable->data.s);
                }
                while (variable->listData.count > 0)
                {
                    debugPrint("pop variable list");
                    VariableUnion* pop = (VariableUnion*)PopList(&variable->listData);
                    if (variable->currentType == TYPE_STRING)
                    {
                        FreeString(&pop->s);
                    }
                    free(pop);
                }

                free(DeleteListElement(&scriptData->variables, i));
            }
        }
        scriptData->currentScope--;
        FreeString(&trimmedLine);
        return 0;
    }
    debugPrint("bracket execute 2");

    // Set variable

    int varIndex = -1;
    for (int i = 0; i < scriptData->variables.count; i++)
    {
        EngineVar* var = (EngineVar*)ListGetIndex(&scriptData->variables, i);
        if (indexOf(var->name, stringPool[trimmedLine]) == 0)
        {
            varIndex = i;
            break;
        }
    }

    if (varIndex != -1 && indexOf("=", stringPool[trimmedLine]) != -1)
    {
        int index = indexOf("=", stringPool[trimmedLine]) - 1;
        uint8_t assignType = stringPool[trimmedLine][index];

        // create a variable to store the assignment result
        uint16_t value = PoolVar("");

        index += 2;

        while (stringPool[trimmedLine][index] <= 32 && stringPool[trimmedLine][index] != '\0')
        {
            index++;
        }
        debugPrintf("assign value :%s\n", stringPool[trimmedLine] + index);

        char buffer[50];

        const char mutators[] = { '+', '-', '*', '%', '/', '|', '^', '&' };
        bool doMutate = false;

        for (int i = 0; i < sizeof(mutators); i++)
        {
            if (mutators[i] == assignType)
            {
                doMutate = true;
                break;
            }
        }

        EngineVar* setVar = (EngineVar*)ListGetIndex(&scriptData->variables, varIndex);

        if (doMutate)
        {
            sprintf(buffer, "%s%c%s", setVar->name, assignType, stringPool[trimmedLine] + index);
        }
        else
        {
            strcpy(buffer, stringPool[trimmedLine] + index);
        }
        uint32_t error = ShuntYard(buffer, strlen(buffer), &varPool[value], scriptData);

        if (error != 0)
        {
            FreeString(&trimmedLine);
            FreeVar(&value);
            return error | currentLine;
        }
        debugPrintf("to %d, from %d", setVar->currentType, varPool[value].currentType);

        if (varPool[value].currentType == TYPE_INT)
        {
            varPool[value].currentType = TYPE_FLOAT;
            varPool[value].data.f = varPool[value].data.i;
        }

        if (setVar->currentType == TYPE_BOOL && varPool[value].currentType == TYPE_FLOAT)
        {
            setVar->data.b = (varPool[value].data.f != 0);
        }
        if (setVar->currentType == TYPE_VECTOR && varPool[value].currentType == TYPE_VECTOR)
        {
            setVar->data.XY.x = varPool[value].data.XY.x;
            setVar->data.XY.y = varPool[value].data.XY.y;
        }
        if (EqualType(setVar, &varPool[value], TYPE_FLOAT))
        {
            setVar->data.f = varPool[value].data.f;
        }
        if (setVar->currentType == TYPE_INT && varPool[value].currentType == TYPE_FLOAT)
        {
            debugPrint("float assign to int");
            debugPrintf("%d<-%d\n", setVar->data.i, (int)varPool[value].data.f);
            setVar->data.i = (int)varPool[value].data.f;
        }
        if (EqualType(setVar, &varPool[value], TYPE_STRING))
        {
            uint16_t temp = PoolString();
            sprintf(stringPool[temp], "%s", stringPool[varPool[value].data.s]);

            FreeString(&setVar->data.s);
            FreeString(&varPool[value].data.s);

            setVar->data.s = temp;
        }
        if (EqualType(setVar, &varPool[value], TYPE_OBJ))
        {
            setVar->data.objIndex = varPool[value].data.objIndex;
        }

        if (varPool[value].currentType == TYPE_STRING)
            FreeString(&varPool[value].data.s);
        FreeVar(&value);
        justShuntYard = false;
    }

    // If statement
    if (indexOf("if", stringPool[trimmedLine]) == 0)
    {
        int index = 2;
        while (stringPool[trimmedLine][index] != '(' && stringPool[trimmedLine][index] != '\0')
        {
            index++;
        }
        if (stringPool[trimmedLine][index] == '\0')
        {
            FreeString(&trimmedLine);
            return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, currentLine);
        }
        index++;

        int endIndex = index;
        uint8_t parenthesisCount = 1;

        while (stringPool[trimmedLine][endIndex] != '\0' && parenthesisCount > 0)
        {
            if (stringPool[trimmedLine][endIndex] == ')')
            {
                parenthesisCount--;
            }
            if (stringPool[trimmedLine][endIndex] == '(')
            {
                parenthesisCount++;
            }
            endIndex++;
        }
        endIndex--;

        uint16_t value = PoolVar("");
        uint32_t error = ShuntYard(stringPool[trimmedLine] + index, endIndex - index, &varPool[value], scriptData);
        if (error != 0)
        {
            FreeString(&trimmedLine);
            if (varPool[value].currentType == TYPE_STRING)
                FreeString(&varPool[value].data.s);
            FreeVar(&value);
            return error | currentLine;
        }

        bool isZero = false;
        if (varPool[value].currentType == TYPE_FLOAT)
        {
            isZero = (floor(varPool[value].data.f) == 0) ? 1 : 0;
        }
        else if (varPool[value].currentType == TYPE_INT)
        {
            isZero = (floor(varPool[value].data.i) == 0) ? 1 : 0;
        }
        else
        {
            FreeString(&trimmedLine);
            if (varPool[value].currentType == TYPE_STRING)
                FreeString(&varPool[value].data.s);
            FreeVar(&value);
            return CreateError(GENERAL_ERROR, TYPE_MISMATCH, currentLine);
        }

        // get global start
        debugPrintf("start bracket: %d\n", currentLine);
        uint16_t endLine = 0;

        bool foundEnd = false;
        // Iterate pairs to find the end
        for (int i = 0; i < scriptData->bracketPairs; i++)
        {
            debugPrintf("check bracket: %d, %d\n", scriptData->brackets[i].startLine, scriptData->brackets[i].endLine);
            if (scriptData->brackets[i].startLine == currentLine)
            {
                foundEnd = true;
                debugPrintf("end bracket: %d\n", scriptData->brackets[i].endLine);
                endLine = scriptData->brackets[i].endLine;
            }
        }

        if (!foundEnd)
        {
            FreeString(&trimmedLine);
            if (varPool[value].currentType == TYPE_STRING)
                FreeString(&varPool[value].data.s);
            FreeVar(&value);
            return CreateError(GENERAL_ERROR, BRACKET_MISMATCH, scriptData->lineCount);
        }

        if (isZero)
        {
            FreeString(&trimmedLine);
            if (varPool[value].currentType == TYPE_STRING)
                FreeString(&varPool[value].data.s);
            FreeVar(&value);

            debugPrintf("pushed %d\n", endLine + 1);
            PushLine(scriptData, endLine + 1);
            return 0;
        }
        else
        {
            FreeString(&trimmedLine);
            if (varPool[value].currentType == TYPE_STRING)
                FreeString(&varPool[value].data.s);
            FreeVar(&value);

            PushLine(scriptData, endLine + 1);
            PushLine(scriptData, currentLine + 1);

            scriptData->currentScope++;

            debugPrintf("pushed %d\n", endLine + 1);
            debugPrintf("pushed %d\n", currentLine + 1);

            return 0;
        }
    }

    // while statement
    if (indexOf("while", stringPool[trimmedLine]) == 0)
    {
        int index = 2;
        while (stringPool[trimmedLine][index] != '(' && stringPool[trimmedLine][index] != '\0')
        {
            index++;
        }
        if (stringPool[trimmedLine][index] == '\0')
        {
            FreeString(&trimmedLine);
            return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, currentLine);
        }
        index++;

        int endIndex = index;
        uint8_t parenthesisCount = 1;

        while (stringPool[trimmedLine][endIndex] != '\0' && parenthesisCount > 0)
        {
            if (stringPool[trimmedLine][endIndex] == ')')
            {
                parenthesisCount--;
            }
            if (stringPool[trimmedLine][endIndex] == '(')
            {
                parenthesisCount++;
            }
            endIndex++;
        }
        endIndex--;

        uint16_t value = PoolVar("");
        uint32_t error = ShuntYard(stringPool[trimmedLine] + index, endIndex - index, &varPool[value], scriptData);
        if (error != 0)
        {
            FreeString(&trimmedLine);
            if (varPool[value].currentType == TYPE_STRING)
                FreeString(&varPool[value].data.s);
            FreeVar(&value);
            return error | currentLine;
        }

        bool isZero = false;
        if (varPool[value].currentType == TYPE_FLOAT)
        {
            isZero = (floor(varPool[value].data.f) == 0) ? 1 : 0;
        }
        else if (varPool[value].currentType == TYPE_INT)
        {
            isZero = (floor(varPool[value].data.i) == 0) ? 1 : 0;
        }
        else
        {
            FreeString(&trimmedLine);
            if (varPool[value].currentType == TYPE_STRING)
                FreeString(&varPool[value].data.s);
            FreeVar(&value);
            return CreateError(GENERAL_ERROR, TYPE_MISMATCH, currentLine);
        }

        // get global start
        debugPrintf("start bracket: %d\n", currentLine);
        uint16_t endLine = 0;

        bool foundEnd = false;
        // Iterate pairs to find the end
        for (int i = 0; i < scriptData->bracketPairs; i++)
        {
            debugPrintf("check bracket: %d, %d\n", scriptData->brackets[i].startLine, scriptData->brackets[i].endLine);
            if (scriptData->brackets[i].startLine == currentLine)
            {
                foundEnd = true;
                debugPrintf("end bracket: %d\n", scriptData->brackets[i].endLine);
                endLine = scriptData->brackets[i].endLine;
            }
        }

        if (!foundEnd)
        {
            FreeString(&trimmedLine);
            if (varPool[value].currentType == TYPE_STRING)
                FreeString(&varPool[value].data.s);
            FreeVar(&value);
            return CreateError(GENERAL_ERROR, BRACKET_MISMATCH, scriptData->lineCount);
        }

        if (isZero)
        {
            FreeString(&trimmedLine);
            if (varPool[value].currentType == TYPE_STRING)
                FreeString(&varPool[value].data.s);
            FreeVar(&value);

            debugPrintf("pushed %d\n", endLine + 1);
            PushLine(scriptData, endLine + 1);
            return 0;
        }
        else
        {
            FreeString(&trimmedLine);
            if (varPool[value].currentType == TYPE_STRING)
                FreeString(&varPool[value].data.s);
            FreeVar(&value);

            PushLine(scriptData, currentLine);
            PushLine(scriptData, currentLine + 1);

            scriptData->currentScope++;

            debugPrintf("pushed %d\n", endLine + 1);
            debugPrintf("pushed %d\n", currentLine + 1);
            return 0;
        }
    }

    if (indexOf("print", stringPool[trimmedLine]) == 0)
    {
        int start = indexOf("(", stringPool[trimmedLine]) + 1;
        int endIndex = start;
        uint8_t parenthesisCount = 1;

        while (stringPool[trimmedLine][endIndex] != '\0' && parenthesisCount > 0)
        {
            if (stringPool[trimmedLine][endIndex] == ')')
            {
                parenthesisCount--;
            }
            if (stringPool[trimmedLine][endIndex] == '(')
            {
                parenthesisCount++;
            }
            endIndex++;
        }
        endIndex--;
        uint16_t out = PoolVar("");
        printf("length: %d\n", endIndex - start);
        uint32_t error = ShuntYard(stringPool[trimmedLine] + start, endIndex - start, &varPool[out], scriptData);
        if (error != 0)
        {
            FreeString(&trimmedLine);
            FreeVar(&out);
            return error | currentLine;
        }


        uint16_t printMessage = SerializeVar(&varPool[out], false);
        debugPrintf("message print: %s\n", stringPool[printMessage]);
        UI_PrintMessage(scriptData, printMessage);

        if (varPool[out].currentType == TYPE_STRING) {
            FreeString(&varPool[out].data.s);

            while (varPool[out].listData.count > 0)
            {
                debugPrint("pop content");
                VariableUnion* listElement = (VariableUnion*)PopList(&varPool[out].listData);
                if (varPool[out].currentType == TYPE_STRING)
                {
                    FreeString(&listElement->s);
                }
                free(listElement);
            }
        }
        FreeVar(&out);
        justShuntYard = false;
    }

    if (justShuntYard)
    {
        uint16_t out = PoolVar("");
        uint32_t error = ShuntYard(stringPool[trimmedLine], strlen(stringPool[trimmedLine]), &varPool[out], scriptData);
        if (varPool[out].currentType == TYPE_STRING)
        {
            FreeString(&varPool[out].data.s);
        }
        while (varPool[out].listData.count > 0)
        {
            debugPrint("pop output");
            sleep_ms(100);
            VariableUnion* pop = (VariableUnion*)PopList(&varPool[out].listData);

#ifdef DEBUG
            uint16_t serialize = SerializeUnion(pop, varPool[out].currentType, false);
            debugPrintf("popped data: %s\n", stringPool[serialize]);
            FreeString(&serialize);
#endif

            if (pop == NULL)
            {
                debugPrint("null pop!");
                continue;
            }
            if (varPool[out].currentType == TYPE_STRING)
            {
                debugPrint("freed string");
                FreeString(&pop->s);
            }
            debugPrint("freeing pop...");
            free(pop);
            debugPrint("freed");
        }
        debugPrint("freed content");

        FreeVar(&out);

        debugPrint("shunt all done");
    }

    PushLine(scriptData, currentLine + 1);

    FreeString(&trimmedLine);

    return 0;
}

void ResetScriptData(ScriptData* scriptData)
{
    debugPrint("RESET SCRIPT DATA");
    for (int i = 0; i < scriptData->variables.count; i++)
    {
        debugPrint("get variable");
        EngineVar* variable = (EngineVar*)ListGetIndex(&scriptData->variables, i);

        if (variable->scope != 0)
        {
            debugPrintf("free variable: %s\n", variable->name);
            if (variable->currentType == TYPE_STRING)
            {
                FreeString(&variable->data.s);
            }
            while (variable->listData.count > 0)
            {
                debugPrint("pop variable list");
                VariableUnion* pop = (VariableUnion*)PopList(&variable->listData);
                if (variable->currentType == TYPE_STRING)
                {
                    FreeString(&pop->s);
                }
                free(pop);
            }

            DeleteListElement(&scriptData->variables, i);

            free(variable);

            i = -1;
        }
    }
    scriptData->currentScope = 0;
    debugPrint("reset data done");
}

uint32_t ExecuteInstructionStack() {

    void UI_PrintError(ScriptData * scrData, uint32_t error);

    debugPrintf("instructions to do: %d\n", instructionStack.count);
    while (instructionStack.count > 0) {
        InstructionPointer* poppedLine = (InstructionPointer*)PopList(&instructionStack);
        uint32_t error = ExecuteLine(poppedLine->script->script, poppedLine->script, poppedLine->line);
        if (error != 0)
            UI_PrintError(poppedLine->script, error);
        free(poppedLine);

        debugPrintf("instructions to do: %d\n", instructionStack.count);
    }
    return 0;
}

#endif