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

#define LEFT_LIGHT 28
#define RIGHT_LIGHT 4

// #define DEBUG 1

#ifdef DEBUG
#define debugPrintf(...) printf("DEBUG: " __VA_ARGS__)
#define debugPrint(x) printf("%s\n", x)
#else
#define debugPrintf(...) \
    do                   \
    {                    \
    } while (0)
#define debugPrint(x) \
    do                \
    {                 \
    } while (0)
#endif

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

#define OPPERATION_SUCCESS 0x8000

const char *ERROR_NAMES[] = {
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
    "CORE 1 DID NOT RESPOND"};

const char *CATEGORY_ERRORS[] = {
    "EQUATION",
    "FUNCTION",
    "VARIABLE",
    "PARENTHESIS",
    "DATATYPE",
    "GENERAL",
    "SCENE",
    "RENDERER"};

#define OPERATOR_ATOM 255

#define UNARY_SUBTRACT 255

typedef struct
{
    uint16_t atom;
    uint8_t type;
    uint8_t precedence;
    uint8_t parameters;
} Atom;

typedef struct
{
    char *operator;
    uint8_t precedence;
    uint8_t parameters;
} OperatorPrecedence;





#define OPERATOR_COUNT 36

OperatorPrecedence OPERATOR_PRECEDENT_LIST[] = {

    {"Vector", 13, 2},

    {"input", 13, 1},
    {"deltaTime", 13, 0},

    {"getPosition", 13, 0},
    {"getScale", 13, 0},
    {"getSprite", 13, 0},

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

/*
int IntLength(int value)
{
    int numberLength = 1;
    if (abs(value) != 0)
    {
        numberLength = (int)(log10(abs(value))) + 1;
    }
    if (abs(value) != value)
    {
        numberLength += 1;
    }
    return numberLength;
}*/

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

void FreeAtom(Atom *atom)
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
}

int indexOf(char *search, char *string)
{
    char *pos = strstr(string, search);
    if (pos == NULL)
        return -1;
    return pos - string;
}

bool EqualType(EngineVar *var1, EngineVar *var2, uint8_t type)
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
    const char *ERROR_PREFIX = "ERROR: ";
    const char *CATEGORY_SUFFIX = ", ";
    const char *ERROR_TYPE_SUFFIX = ";";
    const char *LINE_PREFIX = " Line - ";

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

void SplitScript(EngineScript *script, ScriptData *scriptData)
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
    char **lines = (char **)malloc(sizeof(char *) * lineCount);
    int *lineIndexes = (int *)malloc(sizeof(int) * lineCount);
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
            lines[currentLine] = (char *)malloc(sizeof(char) * (lineLength));
            debugPrintf("allocated line len %d\n", lineLength + 1);
            currentLine++;
            lineLength = 0;
        }
        else if (script->content[i] == '{' || script->content[i] == '}')
        {
            lines[currentLine] = (char *)malloc(sizeof(char) * (lineLength + 1));
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

uint32_t CalculateBrackets(EngineScript *script, ScriptData *output)
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
    output->brackets = (BracketPair *)malloc(sizeof(BracketPair) * openBrackets);
    for (int i = 0; i < openBrackets; i++)
    {
        output->brackets[i].start = -1;
        output->brackets[i].end = -1;
        output->brackets[i].bracketType = 0;
    }
    uint16_t currentBracket = 0;
    uint16_t lastOpen = 65535;
    // uint16_t currentLine = 0;
    for (int i = 0; i < strlen(script->content); i++)
    {
        /*if (script->content[i] == '\n')
        {
            currentLine++;
            continue;
        }*/
        if (script->content[i] == '(' || script->content[i] == '{')
        {
            lastOpen = currentBracket;
            output->brackets[currentBracket].start = i;
            output->brackets[currentBracket].startPos = i;
            output->brackets[currentBracket++].bracketType = (script->content[i] == '{') ? CURLY_BRACKET : PARENTHESIS;
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
                output->brackets[lastOpen].endPos = i;
                while (output->brackets[lastOpen].end != -1)
                {
                    if (lastOpen == 0)
                    {
                        lastOpen = 65535;
                        break;
                    }
                    lastOpen--;
                }
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

/*void FreeAtom(Atom *atom)
{
    free(atom->atom);
    free(atom);
}*/

void cpyatom(Atom *to, Atom *from)
{
    /*if (to->atom != NULL)
    {
        free(to->atom);
    }*/
    to->type = from->type;
    to->precedence = from->precedence;
    to->parameters = from->parameters;
    to->atom = PoolString();
    strcpy(stringPool[to->atom], stringPool[from->atom]);
}

void setatom(Atom *to, uint8_t type, uint8_t precedence, uint8_t parameters, char *name)
{
    /*if (to->atom != NULL)
    {
        free(to->atom);
    }*/
    to->type = type;
    to->precedence = precedence;
    to->parameters = parameters;
    to->atom = PoolString();
    strcpy(stringPool[to->atom], name);
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
// Returns a malloced char*
uint16_t SerializeVar(EngineVar *variable)
{
    if (variable->currentType == TYPE_STRING)
    {
        uint16_t strCpy = PoolString();
        strcpy(stringPool[strCpy], stringPool[variable->data.s]);
        return strCpy;
    }
    if (variable->currentType == TYPE_INT)
    {
        debugPrintf("int type serialize, %d\n", variable->data.i);
        int numberLength = IntLength(variable->data.i);

        debugPrintf("length: %d", numberLength);
        uint16_t intChar = PoolString();
        sprintf(stringPool[intChar], "%d", variable->data.i);
        debugPrintf("out: %s\n", stringPool[intChar]);
        return intChar;
    }
    if (variable->currentType == TYPE_FLOAT)
    {
        debugPrint("serialize float");

        debugPrintf("val: %f\n", variable->data.f);

        int numberLength = FloatLength(variable->data.f);

        uint16_t floatChar = PoolString();

        snprintf(stringPool[floatChar], sizeof(stringPool[floatChar]), "%f", variable->data.f);
        return floatChar;
    }
    if (variable->currentType == TYPE_BOOL)
    {
        uint16_t boolChar = PoolString();
        if (variable->data.b)
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
    if (variable->currentType == TYPE_VECTOR)
    {
        uint16_t vectorChar = PoolString();

        sprintf(stringPool[vectorChar], "(%f,%f)", variable->data.XY.x, variable->data.XY.y);
        return vectorChar;
    }

    uint16_t empty = PoolString();
    stringPool[empty][0] = '\0';
    return empty;
}

// If one is a float and one is an int, upgrade to float
void GreatestCommonType(Atom *atom1, Atom *atom2)
{
    if (atom1->type == TYPE_INT && atom2->type == TYPE_FLOAT)
    {
        atom1->type = TYPE_FLOAT;
    }
    if (atom2->type == TYPE_INT && atom1->type == TYPE_FLOAT)
    {
        atom2->type = TYPE_FLOAT;
    }
}

float ShuntYard(char *equation, uint16_t equationLength, EngineVar *output, ScriptData *scriptData)
{
    debugPrint("shunt yard");
    Atom operationStack[50] = {0};
    uint8_t operatorIndex = 0;
    Atom operandStack[50] = {0};
    uint8_t operandIndex = 0;

    Atom allAtoms[50] = {0};
    uint16_t atomIndex = 0;

    // Tokenize the input into atoms
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
            setatom(&allAtoms[atomIndex++], OPERATOR_ATOM, 0, 0, ",");
            continue;
        }
        if (equation[i] == '(')
        {

            setatom(&allAtoms[atomIndex++], OPERATOR_ATOM, 0, 0, "(");

            debugPrintf("Atom: %s\n", stringPool[allAtoms[atomIndex - 1].atom]);
            continue;
        }
        if (equation[i] == ')')
        {

            setatom(&allAtoms[atomIndex++], OPERATOR_ATOM, 0, 0, ")");

            debugPrintf("Atom: %s\n", stringPool[allAtoms[atomIndex - 1].atom]);
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
                for (int i = 0; i < operatorIndex; i++)
                {
                    FreeString(&operationStack[i].atom);
                }
                for (int i = 0; i < operandIndex; i++)
                {
                    FreeString(&operandStack[i].atom);
                }
                return CreateError(EQUATION_ERROR, SYNTAX_UNKNOWN, 0);
            }

            uint16_t string = PoolString();

            strncpy(stringPool[string], equation + i + 1, index - i - 1);

            stringPool[string][index - i - 1] = '\0';

            setatom(&allAtoms[atomIndex++], TYPE_STRING, 0, 0, stringPool[string]);

            FreeString(&string);

            debugPrintf("Atom: %s\n", stringPool[allAtoms[atomIndex - 1].atom]);

            i += index - i;
            continue;
        }

        if (strcmp(equation + i, "false") == 0)
        {
            setatom(&allAtoms[atomIndex++], TYPE_FLOAT, 0, 0, "0");
            i += 4;
            continue;
        }
        if (strcmp(equation + i, "true") == 0)
        {
            setatom(&allAtoms[atomIndex++], TYPE_FLOAT, 0, 0, "1");
            i += 3;
            continue;
        }

        bool isVariable = false;
        for (int variable = 0; variable < scriptData->variableCount; variable++)
        {
            debugPrintf("Var name: %s\n", scriptData->data[variable].name);
            if (indexOf(scriptData->data[variable].name, equation + i) == 0)
            {
                debugPrint("var match");
                uint16_t value = SerializeVar(&(scriptData->data[variable]));

                if (scriptData->data[variable].currentType == TYPE_FLOAT || scriptData->data[variable].currentType == TYPE_INT)
                {
                    setatom(&allAtoms[atomIndex++], TYPE_FLOAT, 0, 0, stringPool[value]);
                }
                else if (scriptData->data[variable].currentType == TYPE_STRING)
                {
                    setatom(&allAtoms[atomIndex++], TYPE_STRING, 0, 0, stringPool[value]);
                }
                else if (scriptData->data[variable].currentType == TYPE_VECTOR)
                {
                    setatom(&allAtoms[atomIndex++], TYPE_VECTOR, 0, 0, stringPool[value]);
                }

                debugPrintf("Atom: %s\n", stringPool[allAtoms[atomIndex - 1].atom]);
                isVariable = true;
                i += strlen(scriptData->data[variable].name) - 1;

                FreeString(&value);
                break;
            }
        }
        if (isVariable)
        {
            continue;
        }

        // debugPrint("Finding operator");
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
                if(equation[i+opIndex] != OPERATOR_PRECEDENT_LIST[operator].operator[opIndex])
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                atomDone = true;

                bool isUnarySubtract = false;

                if (equation[i] == '-')
                {
                    debugPrint("is subtraction");

                    if (atomIndex == 0 || allAtoms[atomIndex - 1].type == OPERATOR_ATOM)
                    {
                        debugPrint("is unary");
                        isUnarySubtract = true;

                        char unarySubtractString[2] = {UNARY_SUBTRACT,'\0'};

                        setatom(
                            &allAtoms[atomIndex++],
                            OPERATOR_ATOM,
                            OPERATOR_PRECEDENT_LIST[operator].precedence,
                            1,
                            unarySubtractString
                        );
                    }
                }

                if (!isUnarySubtract)
                {
                    setatom(
                        &allAtoms[atomIndex++],
                        OPERATOR_ATOM,
                        OPERATOR_PRECEDENT_LIST[operator].precedence,
                        OPERATOR_PRECEDENT_LIST[operator].parameters,
                        OPERATOR_PRECEDENT_LIST[operator].operator
                    );
                }

                i += operatorLength - 1;
                debugPrintf("Atom: %s\n", stringPool[allAtoms[atomIndex - 1].atom]);
                break;
            }
        }

        if (atomDone)
            continue;

        // debugPrint("Finding number");
        //  If the current char is a number, continue searching until the end
        uint8_t numberLength = 0;
        while (IsNumber(equation[i + numberLength]))
        {
            // debugPrintf("%c\n",equation[i+numberLength]);
            numberLength++;
        }
        if (numberLength == 0)
        {
            for (int i = 0; i < operatorIndex; i++)
            {
                FreeString(&operationStack[i].atom);
            }
            for (int i = 0; i < operandIndex; i++)
            {
                FreeString(&operandStack[i].atom);
            }
            return CreateError(EQUATION_ERROR, SYNTAX_UNKNOWN, 0);
        }

        allAtoms[atomIndex].atom = PoolString();

        for (int numberIndex = 0; numberIndex < numberLength; numberIndex++)
        {
            // debugPrintf("%c\n", equation[i + numberIndex]);
            stringPool[allAtoms[atomIndex].atom][numberIndex] = equation[i + numberIndex];
        }
        stringPool[allAtoms[atomIndex].atom][numberLength] = '\0';
        allAtoms[atomIndex++].type = TYPE_FLOAT;
        i += numberLength - 1;
        debugPrintf("Atom: %s\n", stringPool[allAtoms[atomIndex - 1].atom]);
    }

    // At this point, the string should be tokenized. Shunt yard can commence

    for (int i = 0; i < atomIndex; i++)
    {
        debugPrintf("Atom shunt yard %s\n", stringPool[allAtoms[i].atom]);

        if (stringPool[allAtoms[i].atom][0] == ',')
        {
            while (operatorIndex > 0 && stringPool[operationStack[operatorIndex - 1].atom][0] != '(')
            {
                debugPrintf("move from parenthesis %s\n", stringPool[operationStack[operatorIndex - 1].atom]);
                cpyatom(&operandStack[operandIndex++], &operationStack[operatorIndex - 1]);

                debugPrintf("moved %s to operand stack\n", stringPool[operationStack[operatorIndex - 1].atom]);
                FreeString(&operationStack[--operatorIndex].atom);
            }
            FreeString(&allAtoms[i].atom);
            // free?
            continue;
        }

        if (allAtoms[i].type == OPERATOR_ATOM)
        {
            if (stringPool[allAtoms[i].atom][0] == ')')
            {
                // If the operator is parenthesis, pop off all of the previous operations until you get to the matching one
                debugPrint("parenthesis");
                while (operatorIndex > 0 && stringPool[operationStack[operatorIndex - 1].atom][0] != '(')
                {
                    debugPrintf("move from parenthesis %s\n", stringPool[operationStack[operatorIndex - 1].atom]);
                    cpyatom(&operandStack[operandIndex++], &operationStack[operatorIndex - 1]);

                    debugPrintf("moved %s to operand stack\n", stringPool[operationStack[operatorIndex - 1].atom]);
                    FreeString(&operationStack[--operatorIndex].atom);
                }
                FreeString(&operationStack[--operatorIndex].atom);
            }
            else
            {
                // If operator is not parenthesis, then move operators to the operands IF the precedence is greater or equal, then push operator
                if (allAtoms[i].precedence != 0 && operandIndex > 0)
                {
                    while (operatorIndex > 0 && operationStack[operatorIndex - 1].precedence >= allAtoms[i].precedence)
                    {
                        cpyatom(&operandStack[operandIndex++], &operationStack[operatorIndex - 1]);

                        debugPrintf("moved %s to operand stack\n", stringPool[operationStack[operatorIndex - 1].atom]);
                        FreeString(&operationStack[--operatorIndex].atom);
                    }
                }
                cpyatom(&operationStack[operatorIndex++], &allAtoms[i]);
                debugPrintf("added %s to operation stack\n", stringPool[allAtoms[i].atom]);
            }
        }
        else
        {
            cpyatom(&operandStack[operandIndex++], &allAtoms[i]);
            debugPrintf("added %s to operand stack\n", stringPool[allAtoms[i].atom]);
        }
        FreeString(&allAtoms[i].atom);
    }
    debugPrint("shunt done, pushing remain");
    // push remaining operators to output
    while (operatorIndex > 0)
    {
        cpyatom(&operandStack[operandIndex++], &operationStack[operatorIndex - 1]);
        debugPrintf("moved %s to operand stack\n", stringPool[operationStack[operatorIndex - 1].atom]);
        FreeString(&operationStack[--operatorIndex].atom);
    }

    for (int i = 0; i < operatorIndex; i++)
    {
        FreeString(&operationStack[i].atom);
    }

    for (int i = 0; i < operandIndex; i++)
    {
        debugPrintf("RPN: %s\n", stringPool[operandStack[i].atom]);
        // free(operandStack[i].atom);
    }

    // The input is now in Reverse Polish Notation

    for (int i = 0; i < operandIndex; i++)
    {
        if (operandStack[i].type == OPERATOR_ATOM)
        {
            bool isValid = true;
            debugPrintf("operation: %s, params: %d\n", stringPool[operandStack[i].atom], operandStack[i].parameters);

            uint8_t PARAMETER_COUNT = 2;

            uint16_t parameters[PARAMETER_COUNT];

            for (int freeParam = 0; freeParam < PARAMETER_COUNT; freeParam++)
                parameters[freeParam] = PoolVar("");

            for (int x = 0; x < operandStack[i].parameters; x++)
            {
                if (operandStack[i - x - 1].type == OPERATOR_ATOM)
                {
                    isValid = false;
                    break;
                }
                else
                {
                    if (operandStack[i - x - 1].type == TYPE_FLOAT)
                    {
                        debugPrint("normal param");
                        varPool[parameters[operandStack[i].parameters - x - 1]].data.f = atof(stringPool[operandStack[i - x - 1].atom]);
                        varPool[parameters[operandStack[i].parameters - x - 1]].currentType = TYPE_FLOAT;
                    }
                    else if (operandStack[i - x - 1].type == TYPE_VECTOR)
                    {
                        debugPrint("vector param");
                        float vectX = 0;
                        float vectY = 0;
                        sscanf((stringPool[operandStack[i - x - 1].atom]), "(%f,%f)", &vectX, &vectY);
                        debugPrintf("v:%f,%f\n", vectX, vectY);
                        varPool[parameters[operandStack[i].parameters - x - 1]].data.XY.x = vectX;
                        varPool[parameters[operandStack[i].parameters - x - 1]].data.XY.y = vectY;
                        varPool[parameters[operandStack[i].parameters - x - 1]].currentType = TYPE_VECTOR;
                    }
                    else if (operandStack[i - x - 1].type == TYPE_STRING)
                    {
                        debugPrint("string param");
                        varPool[parameters[operandStack[i].parameters - x - 1]].data.s = PoolString();
                        strcpy(stringPool[varPool[parameters[operandStack[i].parameters - x - 1]].data.s], stringPool[operandStack[i - x - 1].atom]);
                        varPool[parameters[operandStack[i].parameters - x - 1]].currentType = TYPE_STRING;
                        debugPrint("copied");
                    }
                }
            }
            uint16_t result = PoolVar("");
            if (!isValid)
            {
                debugPrint("invalid parameters");
                for (int freeParam = 0; freeParam < PARAMETER_COUNT; freeParam++)
                {
                    if (varPool[parameters[freeParam]].currentType == TYPE_STRING)
                    {
                        FreeString(&varPool[parameters[freeParam]].data.s);
                    }
                    FreeVar(&parameters[freeParam]);
                }
                continue;
            }

            uint8_t parameterCount = operandStack[i].parameters;

            if (parameterCount == 2 && strcmp(stringPool[operandStack[i].atom], "+") == 0)
            {
                debugPrint("add");
                if (EqualType(&varPool[parameters[0]], &varPool[parameters[1]], TYPE_FLOAT))
                {
                    debugPrint("type float");
                    varPool[result].data.f = varPool[parameters[0]].data.f + varPool[parameters[1]].data.f;
                    varPool[result].currentType = TYPE_FLOAT;
                }
                if (EqualType(&varPool[parameters[0]], &varPool[parameters[1]], TYPE_STRING))
                {
                    debugPrint("type string");
                    varPool[result].data.s = PoolString();
                    sprintf(stringPool[varPool[result].data.s], "%s%s", stringPool[varPool[parameters[0]].data.s], stringPool[varPool[parameters[1]].data.s]);

                    debugPrintf("concat result: %s\n", stringPool[varPool[result].data.s]);
                    // free(varPool[parameters[0]].data.s);
                    // free(varPool[parameters[1]].data.s);
                    varPool[result].currentType = TYPE_STRING;
                }
                if (EqualType(&varPool[parameters[0]], &varPool[parameters[1]], TYPE_VECTOR))
                {
                    varPool[result].data.XY.x = varPool[parameters[0]].data.XY.x + varPool[parameters[1]].data.XY.x;
                    varPool[result].data.XY.y = varPool[parameters[0]].data.XY.y + varPool[parameters[1]].data.XY.y;
                    varPool[result].currentType = TYPE_FLOAT;
                }
            }

            if (parameterCount == 1 && varPool[parameters[0]].currentType == TYPE_STRING)
            {
                if (strcmp(stringPool[operandStack[i].atom], "input") == 0)
                {
                    debugPrint("input operation");
                    varPool[result].currentType = TYPE_FLOAT;
                    switch (ToLower(stringPool[varPool[parameters[0]].data.s][0]))
                    {
                    case 'w':
                        debugPrintf("W state: %d\n", (gpio_get(BUTTON_W) == 0 ? 1 : 0));
                        varPool[result].data.f = (gpio_get(BUTTON_W) == 0);
                        break;
                    case 'd':
                        varPool[result].data.f = (gpio_get(BUTTON_D) == 0);
                        break;
                    case 'a':
                        varPool[result].data.f = (gpio_get(BUTTON_A) == 0);
                        break;
                    case 's':
                        varPool[result].data.f = (gpio_get(BUTTON_S) == 0);
                        break;

                    case 'i':
                        varPool[result].data.f = (gpio_get(BUTTON_I) == 0);
                        break;
                    case 'j':
                        varPool[result].data.f = (gpio_get(BUTTON_J) == 0);
                        break;
                    case 'k':
                        varPool[result].data.f = (gpio_get(BUTTON_K) == 0);
                        break;
                    case 'l':
                        varPool[result].data.f = (gpio_get(BUTTON_L) == 0);
                        break;
                    default:
                        varPool[result].data.f = -1;
                        break;
                    }
                }
            }

            if (parameterCount == 1 && varPool[parameters[0]].currentType == TYPE_VECTOR)
            {
                if (strcmp(stringPool[operandStack[i].atom], "setPosition") == 0 && scriptData->linkedObject != NULL)
                {
                    debugPrintf("set pos (%f,%f)\n", varPool[parameters[0]].data.XY.x, varPool[parameters[0]].data.XY.y);

                    GetObjectDataByName(scriptData->linkedObject, "position")->data.XY.x = varPool[parameters[0]].data.XY.x;
                    GetObjectDataByName(scriptData->linkedObject, "position")->data.XY.y = varPool[parameters[0]].data.XY.y;

                    debugPrint("set");
                }
                if (strcmp(stringPool[operandStack[i].atom], "setScale") == 0 && scriptData->linkedObject != NULL && GetObjectDataByName(scriptData->linkedObject, "scale")->currentType == TYPE_VECTOR)
                {
                    GetObjectDataByName(scriptData->linkedObject, "scale")->data.XY.x = varPool[parameters[0]].data.XY.x;
                    GetObjectDataByName(scriptData->linkedObject, "scale")->data.XY.y = varPool[parameters[0]].data.XY.y;
                }

                if (strcmp(stringPool[operandStack[i].atom], "setVelocity") == 0 && scriptData->linkedObject != NULL)
                {
                    GetObjectDataByName(scriptData->linkedObject, "velocity")->data.XY.x = varPool[parameters[0]].data.XY.x;
                    GetObjectDataByName(scriptData->linkedObject, "velocity")->data.XY.y = varPool[parameters[0]].data.XY.y;
                }

                if (strcmp(stringPool[operandStack[i].atom], "addPosition") == 0 && scriptData->linkedObject != NULL)
                {
                    GetObjectDataByName(scriptData->linkedObject, "position")->data.XY.x += varPool[parameters[0]].data.XY.x;
                    GetObjectDataByName(scriptData->linkedObject, "position")->data.XY.y += varPool[parameters[0]].data.XY.y;
                    printf("new pos (%f,%f)\n",
                           GetObjectDataByName(scriptData->linkedObject, "position")->data.XY.x,
                           GetObjectDataByName(scriptData->linkedObject, "position")->data.XY.y);
                }
                if (strcmp(stringPool[operandStack[i].atom], "addScale") == 0 && scriptData->linkedObject != NULL && GetObjectDataByName(scriptData->linkedObject, "scale")->currentType == TYPE_VECTOR)
                {
                    GetObjectDataByName(scriptData->linkedObject, "scale")->data.XY.x += varPool[parameters[0]].data.XY.x;
                    GetObjectDataByName(scriptData->linkedObject, "scale")->data.XY.y += varPool[parameters[0]].data.XY.y;
                }

                if (strcmp(stringPool[operandStack[i].atom], "addVelocity") == 0 && scriptData->linkedObject != NULL)
                {
                    GetObjectDataByName(scriptData->linkedObject, "velocity")->data.XY.x += varPool[parameters[0]].data.XY.x;
                    GetObjectDataByName(scriptData->linkedObject, "velocity")->data.XY.y += varPool[parameters[0]].data.XY.y;
                }
            }

            if (parameterCount == 2 && EqualType(&varPool[parameters[0]], &varPool[parameters[1]], TYPE_FLOAT))
            {
                debugPrintf("FLOAT OP: %s\n", stringPool[operandStack[i].atom]);
                varPool[result].currentType = TYPE_FLOAT;
                if (strcmp(stringPool[operandStack[i].atom], "-") == 0)
                {
                    varPool[result].data.f = varPool[parameters[0]].data.f - varPool[parameters[1]].data.f;
                }
                if (strcmp(stringPool[operandStack[i].atom], "*") == 0)
                {
                    varPool[result].data.f = varPool[parameters[0]].data.f * varPool[parameters[1]].data.f;
                }
                if (strcmp(stringPool[operandStack[i].atom], "/") == 0)
                {
                    varPool[result].data.f = varPool[parameters[0]].data.f / varPool[parameters[1]].data.f;
                }
                if (strcmp(stringPool[operandStack[i].atom], "%") == 0)
                {
                    varPool[result].data.f = (int)varPool[parameters[0]].data.f % (int)varPool[parameters[1]].data.f;
                }

                if (strcmp(stringPool[operandStack[i].atom], "pow") == 0)
                {
                    varPool[result].data.f = pow(varPool[parameters[0]].data.f, varPool[parameters[1]].data.f);
                }

                if (strcmp(stringPool[operandStack[i].atom], "|") == 0)
                {
                    varPool[result].data.f = (float)((int)varPool[parameters[0]].data.f | (int)varPool[parameters[1]].data.f);
                }
                if (strcmp(stringPool[operandStack[i].atom], "&") == 0)
                {
                    varPool[result].data.f = (float)((int)varPool[parameters[0]].data.f & (int)varPool[parameters[1]].data.f);
                }
                if (strcmp(stringPool[operandStack[i].atom], "^") == 0)
                {
                    varPool[result].data.f = (float)((int)varPool[parameters[0]].data.f ^ (int)varPool[parameters[1]].data.f);
                }

                if (strcmp(stringPool[operandStack[i].atom], "==") == 0)
                {
                    varPool[result].data.f = (varPool[parameters[0]].data.f == varPool[parameters[1]].data.f) ? 1 : 0;
                }
                if (strcmp(stringPool[operandStack[i].atom], "<") == 0)
                {
                    varPool[result].data.f = (varPool[parameters[0]].data.f < varPool[parameters[1]].data.f) ? 1 : 0;
                }
                if (strcmp(stringPool[operandStack[i].atom], ">") == 0)
                {
                    varPool[result].data.f = (varPool[parameters[0]].data.f > varPool[parameters[1]].data.f) ? 1 : 0;
                }
                if (strcmp(stringPool[operandStack[i].atom], "<=") == 0)
                {
                    varPool[result].data.f = (varPool[parameters[0]].data.f <= varPool[parameters[1]].data.f) ? 1 : 0;
                }
                if (strcmp(stringPool[operandStack[i].atom], ">=") == 0)
                {
                    varPool[result].data.f = (varPool[parameters[0]].data.f >= varPool[parameters[1]].data.f) ? 1 : 0;
                }
                if (strcmp(stringPool[operandStack[i].atom], "!=") == 0)
                {
                    varPool[result].data.f = (varPool[parameters[0]].data.f != varPool[parameters[1]].data.f) ? 1 : 0;
                }

                if (strcmp(stringPool[operandStack[i].atom], "Vector") == 0)
                {
                    varPool[result].currentType = TYPE_VECTOR;
                    varPool[result].data.XY.x = varPool[parameters[0]].data.f;
                    varPool[result].data.XY.y = varPool[parameters[1]].data.f;
                }
            }
            else if (parameterCount == 1 && varPool[parameters[0]].currentType == TYPE_FLOAT)
            {
                varPool[result].currentType = TYPE_FLOAT;
                if (strcmp(stringPool[operandStack[i].atom], "cos") == 0)
                {
                    debugPrint("COS OPERATION");
                    varPool[result].data.f = cos(varPool[parameters[0]].data.f);
                }
                if (strcmp(stringPool[operandStack[i].atom], "sin") == 0)
                {
                    debugPrint("SIN OPERATION");
                    varPool[result].data.f = sin(varPool[parameters[0]].data.f);
                }

                if (strcmp(stringPool[operandStack[i].atom], "setScale") == 0 && scriptData->linkedObject != NULL && GetObjectDataByName(scriptData->linkedObject, "scale")->currentType == TYPE_INT)
                {
                    varPool[result].currentType = NO_TYPE;

                    GetObjectDataByName(scriptData->linkedObject, "scale")->data.i = varPool[parameters[0]].data.f;
                }

                if (strcmp(stringPool[operandStack[i].atom], "addScale") == 0 && scriptData->linkedObject != NULL && GetObjectDataByName(scriptData->linkedObject, "scale")->currentType == TYPE_INT)
                {
                    varPool[result].currentType = NO_TYPE;

                    GetObjectDataByName(scriptData->linkedObject, "scale")->data.i = varPool[parameters[0]].data.f;
                }

                if (strcmp(stringPool[operandStack[i].atom], "setSprite") == 0 && scriptData->linkedObject != NULL)
                {
                    varPool[result].currentType = NO_TYPE;
                    GetObjectDataByName(scriptData->linkedObject, "sprite")->data.i = (int)varPool[parameters[0]].data.f;
                }

                if (strcmp(stringPool[operandStack[i].atom], "setAngle") == 0 && scriptData->linkedObject != NULL)
                {
                    varPool[result].currentType = NO_TYPE;
                    GetObjectDataByName(scriptData->linkedObject, "angle")->data.f = (float)varPool[parameters[0]].data.f;
                }
                if (strcmp(stringPool[operandStack[i].atom], "addAngle") == 0 && scriptData->linkedObject != NULL)
                {
                    varPool[result].currentType = NO_TYPE;
                    GetObjectDataByName(scriptData->linkedObject, "angle")->data.f += (float)varPool[parameters[0]].data.f;
                }

                if (strcmp(stringPool[operandStack[i].atom], "leftLED") == 0 && scriptData->linkedObject != NULL)
                {
                    varPool[result].currentType = NO_TYPE;
                    gpio_put(LEFT_LIGHT, varPool[parameters[0]].data.f != 0);
                }
                if (strcmp(stringPool[operandStack[i].atom], "rightLED") == 0 && scriptData->linkedObject != NULL)
                {
                    varPool[result].currentType = NO_TYPE;
                    gpio_put(RIGHT_LIGHT, varPool[parameters[0]].data.f != 0);
                }

                if (stringPool[operandStack[i].atom][0] == UNARY_SUBTRACT)
                {
                    debugPrint("UNARY NEGATE OP");
                    varPool[result].data.f = -varPool[parameters[0]].data.f;
                }
            }

            if (strcmp(stringPool[operandStack[i].atom], "PI") == 0)
            {
                varPool[result].currentType = TYPE_FLOAT;
                varPool[result].data.f = 3.14159265459;
            }

            if (parameterCount == 2 && EqualType(&varPool[parameters[0]], &varPool[parameters[1]], TYPE_VECTOR))
            {
                if (strcmp(stringPool[operandStack[i].atom], "-") == 0)
                {
                    varPool[result].data.XY.x = varPool[parameters[0]].data.XY.x - varPool[parameters[1]].data.XY.x;
                    varPool[result].data.XY.y = varPool[parameters[0]].data.XY.y - varPool[parameters[1]].data.XY.y;
                    varPool[result].currentType = TYPE_FLOAT;
                }
                if (strcmp(stringPool[operandStack[i].atom], "*") == 0)
                {
                    varPool[result].data.XY.x = varPool[parameters[0]].data.XY.x * varPool[parameters[1]].data.XY.x;
                    varPool[result].data.XY.y = varPool[parameters[0]].data.XY.y * varPool[parameters[1]].data.XY.y;
                    varPool[result].currentType = TYPE_FLOAT;
                }
                if (strcmp(stringPool[operandStack[i].atom], "/") == 0)
                {
                    varPool[result].data.XY.x = varPool[parameters[0]].data.XY.x / varPool[parameters[1]].data.XY.x;
                    varPool[result].data.XY.y = varPool[parameters[0]].data.XY.y / varPool[parameters[1]].data.XY.y;
                    varPool[result].currentType = TYPE_FLOAT;
                }
            }

            if (parameterCount > 0)
            {

                // shift the rest of the operands
                for (int x = i + 1; x < operandIndex; x++)
                {
                    FreeString(&operandStack[x - parameterCount].atom);
                    cpyatom(&operandStack[x - parameterCount], &operandStack[x]);
                    debugPrintf("shift: %s to %d\n", stringPool[operandStack[x].atom], x - parameterCount);
                    // free(operandStack[x].atom);
                }
                for (int x = 0; x < parameterCount; x++)
                {
                    FreeString(&operandStack[operandIndex - x - 1].atom);
                }
                operandIndex -= parameterCount;
            }

            FreeString(&operandStack[i - parameterCount].atom);

            if (varPool[result].currentType == TYPE_FLOAT)
            {
                debugPrintf("Completed an float operation result: %f\n", varPool[result].data.f);

                char strResult[20];

                snprintf(strResult, sizeof(strResult), "%f", varPool[result].data.f);

                setatom(&operandStack[i - parameterCount], TYPE_FLOAT, 0, 0, strResult);
            }
            else if (varPool[result].currentType == TYPE_VECTOR)
            {
                uint16_t serializedVector = SerializeVar(&varPool[result]);

                setatom(&operandStack[i - parameterCount], TYPE_VECTOR, 0, 0, stringPool[serializedVector]);
                debugPrintf("new Vector: %s\n", stringPool[operandStack[i - parameterCount].atom]);

                FreeString(&serializedVector);
            }
            else
            {
                if (varPool[result].currentType == NO_TYPE)
                {
                    varPool[result].currentType = TYPE_STRING;
                    varPool[result].data.s = PoolString();
                    stringPool[varPool[result].data.s][0] = '\0';
                }
                debugPrintf("Completed an str operation result: %s\n", stringPool[varPool[result].data.s]);

                setatom(&operandStack[i - parameterCount], TYPE_STRING, 0, 0, stringPool[varPool[result].data.s]);
            }

            for (int freeParam = 0; freeParam < PARAMETER_COUNT; freeParam++)
            {
                if (varPool[parameters[freeParam]].currentType == TYPE_STRING)
                {
                    FreeString(&varPool[parameters[freeParam]].data.s);
                }
                FreeVar(&parameters[freeParam]);
            }
            FreeVar(&result);

            i = 0;

            for (int x = 0; x < operandIndex; x++)
            {
                debugPrintf("OUT: %s\n", stringPool[operandStack[x].atom]);
                // debugPrint("debugPrint done");
            }
            debugPrint("");
        }
    }

    debugPrint("Outputting shunt yard");

    if (operandIndex > 1)
    {
        for (int i = 0; i < operandIndex; i++)
        {
            // debugPrintf("atom: %s\n", operandStack[i].atom);
            FreeString(&operandStack[i].atom);
        }

        return CreateError(EQUATION_ERROR, EQUATION_UNSOLVED, 0);
    }
    else
    {
        if (operandStack[0].type == TYPE_FLOAT)
        {

            output->data.f = atof(stringPool[operandStack[0].atom]);

            if (floor(output->data.f) == output->data.f)
            {
                output->data.i = floor(output->data.f);
                output->currentType = TYPE_INT;
                debugPrint("type int");
            }
            else
            {
                output->currentType = TYPE_FLOAT;
                debugPrint("type float");
            }
        }
        if (operandStack[0].type == TYPE_STRING)
        {
            debugPrint("output is string");
            output->currentType = TYPE_STRING;
            output->data.s = PoolString();
            debugPrint("copying");
            debugPrintf("from: %s\n", stringPool[varPool[operandStack[0].atom].data.s]);
            debugPrintf("to: %s\n", stringPool[output->data.s]);
            strcpy(stringPool[output->data.s], stringPool[varPool[operandStack[0].atom].data.s]);
            debugPrint("copy done");
            FreeString(&varPool[operandStack[0].atom].data.s);
            debugPrint("freed string");
        }
    }

    debugPrint("freeing operands");
    for (int i = 0; i < operandIndex; i++)
    {
        FreeString(&operandStack[i].atom);
    }
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
uint16_t LeftTrim(char *str)
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

uint8_t GetTypeFromString(char *line)
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

bool IsAlphaNumeric(char c)
{
    if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c >= 48 && c <= 57))
    {
        return true;
    }
    return false;
}

uint32_t DeclareEntity(ScriptData *output, uint16_t l)
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
            debugPrint("declare function");
            index++;
            EngineFunction *newFunction = (EngineFunction *)malloc(sizeof(EngineFunction));
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
                newFunction->parameters[newFunction->parameterIndex].parameterName = (char *)malloc(parameterNameLength + 1);
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

            int bracketStart = indexOf("{", output->lines[l]) + output->lineIndexes[l];
            debugPrintf("bracket pos: %d\n", bracketStart);
            newFunction->bracketStart = bracketStart;

            bool foundEnd = false;
            for (int i = 0; i < output->bracketPairs; i++)
            {
                if (output->brackets[i].startPos == bracketStart)
                {
                    foundEnd = true;
                    debugPrintf("end bracket: %d\n", output->brackets[i].endPos);
                    newFunction->bracketEnd = output->brackets[i].endPos;
                    break;
                }
            }
            if (!foundEnd)
            {
                FreeString(&entityName);
                free(newFunction);
                FreeString(&trimmedLine);
                return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
            }

            output->functions[output->functionCount] = newFunction;
            output->functions[output->functionCount]->name = (char *)malloc(sizeof(char) * (strlen(stringPool[entityName]) + 1));
            strcpy(output->functions[output->functionCount]->name, stringPool[entityName]);
            FreeString(&entityName);
            output->functionCount++;
            debugPrint("declare fin");

            FreeString(&trimmedLine);
            return OPPERATION_SUCCESS;
        }
        else
        { // variable declare
            output->data[output->variableCount].currentType = varType;
            strncpy(output->data[output->variableCount].name, stringPool[entityName], MAX_NAME_LENGTH - 1);
            output->data[output->variableCount].name[MAX_NAME_LENGTH - 1] = '\0';

            printf("new var: %s\n", output->data[output->variableCount].name);

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

            debugPrintf("right side: %s\n", trimmedLine + index);

            uint16_t assignValue = PoolVar("");

            uint32_t error = ShuntYard(stringPool[trimmedLine] + index, strlen(stringPool[trimmedLine] + index), &varPool[assignValue], output);

            if (error != 0)
            {
                FreeString(&trimmedLine);
                FreeString(&entityName);
                FreeVar(&assignValue);
                return error | ((uint16_t)l & 0xFFFF);
            }
            debugPrintf("TYPE to %d, from %d\n", output->data[output->variableCount].currentType, varPool[assignValue].currentType);
            if (output->data[output->variableCount].currentType == TYPE_INT)
            {
                if (varPool[assignValue].currentType == TYPE_INT)
                {
                    debugPrintf("indexes: %d, %d\n", output->variableCount, assignValue);
                    debugPrintf("set to %s, from %d\n", output->data[output->variableCount].name, varPool[assignValue].data.i);
                    output->data[output->variableCount].data.i = varPool[assignValue].data.i;
                }
                else if (varPool[assignValue].currentType == TYPE_FLOAT)
                {

                    output->data[output->variableCount].data.i = (int)varPool[assignValue].data.f;
                }
                else
                {
                    FreeString(&trimmedLine);
                    FreeString(&entityName);
                    FreeVar(&assignValue);
                    return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
                }
            }
            else if (output->data[output->variableCount].currentType == TYPE_FLOAT)
            {
                if (varPool[assignValue].currentType == TYPE_INT)
                {
                    debugPrintf("indexes: %d, %d\n", output->variableCount, assignValue);
                    debugPrintf("set to %s, from %d\n", output->data[output->variableCount].name, varPool[assignValue].data.i);
                    float floatVal = (float)varPool[assignValue].data.i;
                    output->data[output->variableCount].data.f = floatVal;
                }
                else if (varPool[assignValue].currentType == TYPE_FLOAT)
                {

                    output->data[output->variableCount].data.f = varPool[assignValue].data.f;
                }
                else
                {
                    FreeString(&trimmedLine);
                    FreeString(&entityName);
                    FreeVar(&assignValue);
                    return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
                }
            }
            else if (EqualType(&varPool[assignValue], &output->data[output->variableCount], TYPE_STRING))
            {
                FreeString(&output->data[output->variableCount].data.s);
                output->data[output->variableCount].data.s = PoolString();
                strcpy(stringPool[output->data[output->variableCount].data.s], stringPool[varPool[assignValue].data.s]);
                FreeString(&varPool[assignValue].data.s);
            }
            else if (EqualType(&varPool[assignValue], &output->data[output->variableCount], TYPE_VECTOR))
            {
                output->data[output->variableCount].data.XY.x = varPool[assignValue].data.XY.x;
                output->data[output->variableCount].data.XY.y = varPool[assignValue].data.XY.y;
            }

            FreeVar(&assignValue);

            debugPrintf("variable declared: %s=%d\n", output->data[output->variableCount].name, output->data[output->variableCount].currentType);
            output->variableCount++;
            FreeString(&trimmedLine);
            FreeString(&entityName);
            return OPPERATION_SUCCESS;
        }
        FreeString(&entityName);
    }

    FreeString(&trimmedLine);
    return 0;
}

uint8_t GetScope(ScriptData *scriptData, uint16_t line)
{
    uint8_t level = 0;

    for (int i = 0; i < scriptData->bracketPairs; i++)
    {
        if (
            scriptData->brackets[i].bracketType == CURLY_BRACKET &&
            (scriptData->lineIndexes[line] > scriptData->brackets[i].startPos) &&
            (scriptData->lineIndexes[line] < scriptData->brackets[i].endPos))
        {
            level++;
        }
    }
    debugPrintf("line: %d, scope: %d\n", line, level);
    return level;
}

uint32_t SetScriptData(EngineScript *script, ScriptData *output, uint8_t scopeLevel)
{

    uint32_t error = 0;

    SplitScript(script, output);
    for (int i = 0; i < output->lineCount; i++)
    {
        debugPrintf("split done line num %d\n", i);
        debugPrintf("split done line: %s\n", output->lines[i]);
    }

    error = CalculateBrackets(script, output);
    if (error != 0)
    {
        return error;
    }

    for (int l = 0; l < output->lineCount; l++)
    {
        debugPrintf("setscr %d\n", l);
        // Find and add all variables/functions to the script data
        for (int i = 0; i < output->lineCount; i++)
        {
            debugPrintf("split done line num %d\n", i);
            debugPrintf("split done line: %s\n", output->lines[i]);
        }

        if (GetScope(output, l) != scopeLevel)
        {
            continue;
        }

        error = DeclareEntity(output, l);

        if (error != 0 && error != OPPERATION_SUCCESS)
        {
            debugPrint("set scr error");
            return error;
        }
    }
    debugPrint("set script done");
    return 0;
}

// Frees all script lines, line indexes, brackets, and functions
void FreeScriptData(ScriptData *scriptData, bool onlyFreeContent)
{
    for (int i = 0; i < scriptData->lineCount; i++)
    {
        free(scriptData->lines[i]);
    }
    free(scriptData->lineIndexes);
    free(scriptData->lines);
    free(scriptData->brackets);

    // debugPrint("free scr data 1");

    for (int i = 0; i < scriptData->functionCount; i++)
    {
        free(scriptData->functions[i]->name);
        // debugPrint("free scr data 1a");
        free(scriptData->functions[i]);
        // debugPrint("free scr data 1b");
    }
    for (int i = 0; i < scriptData->variableCount; i++)
    {
        if (scriptData->data[i].currentType == TYPE_STRING)
            FreeString(&scriptData->data[i].data.s);
    }
    free(scriptData->data);
    // debugPrint("free scr data 3");
    scriptData->lineCount = 0;
    scriptData->bracketPairs = 0;
    scriptData->functionCount = 0;
    scriptData->variableCount = 0;

    if (!onlyFreeContent)
        free(scriptData);
}

uint32_t ExecuteLine(EngineScript *script, ScriptData *scriptData)
{
    // Function prototype
    bool UI_PrintToScreen(char *message, bool isError);

    bool justShuntYard = true;

    uint16_t trimmedLine = LeftTrim(scriptData->lines[scriptData->currentLine]);
    debugPrintf("Exexute line: %s\n", stringPool[trimmedLine]);
    // If line is a variable or function declaration, ignore
    if (GetTypeFromString(stringPool[trimmedLine]) != NO_TYPE)
    {
        FreeString(&trimmedLine);
        scriptData->currentLine++;
        return 0;
    }
    // If line only includes end of a curly bracket
    if (stringPool[trimmedLine][0] == '}')
    {
        debugPrint("jumpback");
        int bracketEnd = scriptData->lineIndexes[scriptData->currentLine] + indexOf("}", scriptData->lines[scriptData->currentLine]);

        bool foundEnd = false;
        for (int i = 0; i < scriptData->bracketPairs; i++)
        {
            if (scriptData->brackets[i].endPos == bracketEnd)
            {
                foundEnd = true;
                // debugPrintf("end bracket: %d\n", scriptData->brackets[i].endPos);

                for (int x = 0; x < scriptData->lineCount - 1; x++)
                {
                    if (
                        scriptData->lineIndexes[x] <= scriptData->brackets[i].startPos &&
                        scriptData->lineIndexes[x + 1] > scriptData->brackets[i].startPos)
                    {
                        if (indexOf("while", scriptData->lines[x]) != -1)
                        {
                            scriptData->currentLine = x;
                            debugPrintf("jumpback %d\n", scriptData->currentLine);
                        }
                        else
                        {
                            debugPrint("end brack was not a loop");
                            FreeString(&trimmedLine);
                            scriptData->currentLine++;
                            debugPrint("bracket execute a");
                            return 0;
                        }
                    }
                }

                if (
                    scriptData->lineIndexes[scriptData->lineCount - 1] <= scriptData->brackets[i].startPos)
                {
                    if (indexOf("while", scriptData->lines[scriptData->lineCount - 1]) != -1)
                    {
                        scriptData->currentLine = scriptData->lineCount - 1;
                        debugPrintf("jumpback %d\n", scriptData->currentLine);
                    }
                    else
                    {
                        debugPrint("end brack was not a loop");
                        FreeString(&trimmedLine);
                        scriptData->currentLine++;
                        debugPrint("bracket execute b");
                        return 0;
                    }
                }

                break;
            }
        }
        debugPrint("bracket execute 1");
        if (!foundEnd)
        {
            debugPrint("did not find end");
            FreeString(&trimmedLine);
            return CreateError(GENERAL_ERROR, BRACKET_MISMATCH, scriptData->lineCount);
        }
        FreeString(&trimmedLine);
        return 0;
    }
    debugPrint("bracket execute 2");

    // Set variable

    int varIndex = -1;
    for (int i = 0; i < scriptData->variableCount; i++)
    {
        if (indexOf(scriptData->data[i].name, stringPool[trimmedLine]) == 0)
        {
            varIndex = i;
            break;
        }
    }

    if (varIndex != -1)
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

        const char mutators[] = {'+', '-', '*', '%', '/', '|', '^', '&'};
        bool doMutate = false;

        for (int i = 0; i < sizeof(mutators); i++)
        {
            if (mutators[i] == assignType)
            {
                doMutate = true;
                break;
            }
        }

        if (doMutate)
        {
            sprintf(buffer, "%s%c%s", scriptData->data[varIndex].name, assignType, stringPool[trimmedLine] + index);
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
            return error | scriptData->currentLine;
        }
        debugPrintf("to %d, from %d", scriptData->data[varIndex].currentType, varPool[value].currentType);

        if (varPool[value].currentType == TYPE_INT)
        {
            varPool[value].currentType = TYPE_FLOAT;
            varPool[value].data.f = varPool[value].data.i;
        }

        if (scriptData->data[varIndex].currentType == TYPE_BOOL && varPool[value].currentType == TYPE_FLOAT)
        {
            scriptData->data[varIndex].data.b = (varPool[value].data.f != 0);
        }
        if (scriptData->data[varIndex].currentType == TYPE_VECTOR && varPool[value].currentType == TYPE_VECTOR)
        {
            scriptData->data[varIndex].data.XY.x = varPool[value].data.XY.x;
            scriptData->data[varIndex].data.XY.y = varPool[value].data.XY.y;
        }
        if (EqualType(&scriptData->data[varIndex], &varPool[value], TYPE_FLOAT))
        {
            scriptData->data[varIndex].data.f = varPool[value].data.f;
        }
        if (scriptData->data[varIndex].currentType == TYPE_INT && varPool[value].currentType == TYPE_FLOAT)
        {
            debugPrint("float assign to int");
            debugPrintf("%d<-%d\n", scriptData->data[varIndex].data.i, (int)varPool[value].data.f);
            scriptData->data[varIndex].data.i = (int)varPool[value].data.f;
        }
        if (EqualType(&scriptData->data[varIndex], &varPool[value], TYPE_STRING))
        {
            uint16_t temp = PoolString();
            sprintf(stringPool[temp], "%s", stringPool[varPool[value].data.s]);

            FreeString(&scriptData->data[varIndex].data.s);
            FreeString(&varPool[value].data.s);

            scriptData->data[varIndex].data.s = temp;
        }
        if (EqualType(&scriptData->data[varIndex], &varPool[value], TYPE_OBJ))
        {
            scriptData->data[varIndex].data.objID = varPool[value].data.objID;
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
            return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, scriptData->currentLine);
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
            return error | scriptData->currentLine;
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
            return CreateError(GENERAL_ERROR, TYPE_MISMATCH, scriptData->currentLine);
        }

        if (isZero)
        {
            int bracketStart = indexOf("{", scriptData->lines[scriptData->currentLine]) + scriptData->lineIndexes[scriptData->currentLine];
            debugPrintf("bracket pos: %d\n", bracketStart);

            bool foundEnd = false;
            for (int i = 0; i < scriptData->bracketPairs; i++)
            {
                if (scriptData->brackets[i].startPos == bracketStart)
                {
                    foundEnd = true;
                    debugPrintf("end bracket: %d\n", scriptData->brackets[i].endPos);

                    for (int x = 0; x < scriptData->lineCount - 1; x++)
                    {
                        if (
                            scriptData->lineIndexes[x] <= scriptData->brackets[i].endPos &&
                            scriptData->lineIndexes[x + 1] > scriptData->brackets[i].endPos)
                        {
                            scriptData->currentLine = x + 1;
                        }
                    }

                    if (
                        scriptData->lineIndexes[scriptData->lineCount - 1] <= scriptData->brackets[i].endPos)
                    {
                        scriptData->currentLine = scriptData->lineCount;
                    }

                    FreeString(&trimmedLine);
                    if (varPool[value].currentType == TYPE_STRING)
                        FreeString(&varPool[value].data.s);
                    FreeVar(&value);
                    debugPrintf("Script line: %d\n", scriptData->currentLine);

                    return 0;
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
        }
        if (varPool[value].currentType == TYPE_STRING)
            FreeString(&varPool[value].data.s);
        FreeVar(&value);
        justShuntYard = false;
    }

    // While statement
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
            return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, scriptData->currentLine);
        }
        index++;

        debugPrint("while found (");

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

        debugPrint("while found )");

        uint16_t value = PoolVar("");
        uint32_t error = ShuntYard(stringPool[trimmedLine] + index, endIndex - index, &varPool[value], scriptData);
        if (error != 0)
        {
            FreeString(&trimmedLine);
            if (varPool[value].currentType == TYPE_STRING)
                FreeString(&varPool[value].data.s);
            FreeVar(&value);
            return error | scriptData->currentLine;
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
            return CreateError(GENERAL_ERROR, TYPE_MISMATCH, scriptData->currentLine);
        }

        if (isZero)
        {
            int bracketStart = indexOf("{", scriptData->lines[scriptData->currentLine]) + scriptData->lineIndexes[scriptData->currentLine];
            debugPrintf("bracket pos: %d\n", bracketStart);

            bool foundEnd = false;
            for (int i = 0; i < scriptData->bracketPairs; i++)
            {
                if (scriptData->brackets[i].startPos == bracketStart)
                {
                    foundEnd = true;
                    debugPrintf("end bracket: %d\n", scriptData->brackets[i].endPos);

                    for (int x = 0; x < scriptData->lineCount - 1; x++)
                    {

                        debugPrintf("(%f,%f): %d\n", scriptData->lineIndexes[x], scriptData->lineIndexes[x + 1], scriptData->brackets[i].endPos);
                        if (
                            scriptData->lineIndexes[x] <= scriptData->brackets[i].endPos &&
                            scriptData->lineIndexes[x + 1] > scriptData->brackets[i].endPos)
                        {
                            scriptData->currentLine = x + 1;
                        }
                    }

                    if (
                        scriptData->lineIndexes[scriptData->lineCount - 1] <= scriptData->brackets[i].endPos)
                    {
                        scriptData->currentLine = scriptData->lineCount;
                    }

                    FreeString(&trimmedLine);
                    if (varPool[value].currentType == TYPE_STRING)
                        FreeString(&varPool[value].data.s);
                    FreeVar(&value);
                    debugPrintf("Script line: %d\n", scriptData->currentLine);
                    return 0;
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
        }
        if (varPool[value].currentType == TYPE_STRING)
            FreeString(&varPool[value].data.s);
        FreeVar(&value);
        justShuntYard = false;
    }

    if (indexOf("debugPrint", stringPool[trimmedLine]) == 0)
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
        uint32_t error = ShuntYard(stringPool[trimmedLine] + start, endIndex - start, &varPool[out], scriptData);
        if (error != 0)
        {
            FreeString(&trimmedLine);
            FreeVar(&out);
            return error | scriptData->currentLine;
        }

        if (varPool[out].currentType == TYPE_FLOAT || varPool[out].currentType == TYPE_INT)
        {
            int outLength = 0;

            uint16_t printMessage = PoolString();

            if (varPool[out].currentType == TYPE_FLOAT)
            {
                snprintf(stringPool[printMessage], STRING_POOL_WIDTH, "(%s) %f", scriptData->script->name, varPool[out].data.f);
            }
            else
            {
                snprintf(stringPool[printMessage], STRING_POOL_WIDTH, "(%s) %d", scriptData->script->name, varPool[out].data.i);
            }

            debugPrintf("float message debugPrint: %s\n", stringPool[printMessage]);

            UI_PrintToScreen(stringPool[printMessage], false);

            FreeString(&printMessage);
        }
        else if (varPool[out].currentType == TYPE_STRING)
        {
            debugPrintf("str message debugPrint: %s\n", stringPool[varPool[out].data.s]);

            uint16_t printMessage = PoolString();
            snprintf(stringPool[printMessage], STRING_POOL_WIDTH, "(%s) %s", scriptData->script->name, varPool[out].data.s);

            UI_PrintToScreen(stringPool[printMessage], false);

            FreeString(&printMessage);
        }
        if (varPool[out].currentType == TYPE_STRING)
        {
            FreeString(&varPool[out].data.s);
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
        FreeVar(&out);
    }

    scriptData->currentLine++;
    debugPrintf("Script line: %d\n", scriptData->currentLine);

    FreeString(&trimmedLine);

    return 0;
}

#endif