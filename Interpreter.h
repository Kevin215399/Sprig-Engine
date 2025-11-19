#ifndef INTERPRETER
#define INTERPRETER

#include <stdio.h>
#include "pico/stdlib.h"

#include "SaveSystem.h"

#include "Editor.h"

#include "EngineStructs.h"
#include <string.h>

#include <math.h>

#define EQUATION_ERROR 1
#define FUNCTION_ERROR 2
#define VARIABLE_ERROR 3
#define PARENTHESIS_ERROR 4
#define DATATYPE_ERROR 5
#define GENERAL_ERROR 6

#define INCORRECT_PAIRS 1
#define BRACKET_MISMATCH 2
#define SYNTAX_UNKNOWN 3
#define MALLOC_FAIL 4
#define EQUATION_UNSOLVED 5
#define TYPE_MISMATCH 6
#define TOKEN_NOT_RECOGNIZED 7
#define UNKNOWN_ASSIGNMENT_OPERATOR 8
#define OPERATOR_CANNOT_BE_USED_WITH_TYPE 9

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
    "Type cannot be transformed using this operator"};

const char *CATEGORY_ERRORS[] = {
    "EQUATION",
    "FUNCTION",
    "VARIABLE",
    "PARENTHESIS",
    "DATATYPE",
    "GENERAL"};

#define OPERATOR_ATOM 255

typedef struct
{
    char *atom;
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

#define OPERATOR_COUNT 20

OperatorPrecedence OPERATOR_PRECEDENT_LIST[] = {
    {"PI", 11, 0},
    {"vector", 10, 2},

    {"pow", 9, 2},
    {"sin", 9, 1},
    {"cos", 9, 1},

    {"%", 8, 2},
    {"^", 8, 2},
    {"*", 7, 2},
    {"/", 7, 2},
    {"-", 6, 2},
    {"+", 6, 2},

    {">=", 5, 2},
    {"<=", 5, 2},
    {">", 5, 2},
    {"<", 5, 2},

    {"==", 4, 2},
    {"!=", 4, 2},

    {"&", 3, 2},

    {"^", 2, 2},

    {"|", 1, 2},
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

char *UnpackErrorMessage(uint32_t error)
{
    const char *ERROR_PREFIX = "ERROR: ";
    const char *CATEGORY_SUFFIX = ", ";
    const char *ERROR_TYPE_SUFFIX = ";";
    const char *LINE_PREFIX = " Line - ";

    if (error == 0)
    {
        char *noError = malloc(strlen("No Error") + 1);
        strcpy(noError, "No Error");
        return noError;
    }
    if (error == 0x8000)
    {
        char *noError = malloc(strlen("Opperation Success") + 1);
        strcpy(noError, "Opperation Success");
        return noError;
    }

    uint8_t category = (uint8_t)((error >> 24) & 0xF) - 1;
    uint8_t type = (uint8_t)((error >> 16) & 0xF) - 1;
    uint16_t line = (uint16_t)(error & 0xFFFF);

    uint16_t errorLength = 0;
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

            printf("line digits: %d\n", numberLength);
            errorLength += numberLength;
            errorLength += strlen(LINE_PREFIX);
        }
    }
    errorLength += strlen(ERROR_PREFIX);
    errorLength += strlen(CATEGORY_SUFFIX);
    errorLength += strlen(ERROR_TYPE_SUFFIX);
    printf("error len %d\n", errorLength);

    char *errorMessage = (char *)malloc(errorLength + 1);

    if (numberLength > 0)
    {
        sprintf(errorMessage, "%s%s%s%s%s%s%d", ERROR_PREFIX, CATEGORY_ERRORS[category], CATEGORY_SUFFIX, ERROR_NAMES[type], ERROR_TYPE_SUFFIX, LINE_PREFIX, line);
    }
    else
    {
        sprintf(errorMessage, "%s%s%s%s%s", ERROR_PREFIX, CATEGORY_ERRORS[category], CATEGORY_SUFFIX, ERROR_NAMES[type], ERROR_TYPE_SUFFIX);
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
    // printf("lines %d\n", lineCount);
    // printf("line count: %d\n",lineCount);
    char **lines = (char **)malloc(sizeof(char *) * lineCount);
    int *lineIndexes = (int *)malloc(sizeof(int) * lineCount);
    // print("total lines allocated");
    int currentLine = 0;
    uint16_t lineLength = 0;

    if (lineCount >= 1)
    {
        lineIndexes[0] = 0;
    }

    for (int i = 0; i < strlen(script->content); i++)
    {
        lineLength++;
        if (script->content[i] == ';')
        {
            lines[currentLine] = (char *)malloc(sizeof(char) * (lineLength));
            // printf("allocated line len %d\n", lineLength + 1);
            currentLine++;
            lineLength = 0;
        }
        else if (script->content[i] == '{' || script->content[i] == '}')
        {
            lines[currentLine] = (char *)malloc(sizeof(char) * (lineLength + 1));
            // printf("allocated line len %d\n", lineLength + 1);
            currentLine++;
            lineLength = 0;
        }
    }
    // lines[currentLine] = (char *)malloc(sizeof(char) * (lineLength + 1));
    //  printf("allocated line len %d\n", lineLength + 1);

    currentLine = 0;
    uint16_t caret = 0;
    for (int i = 0; i < strlen(script->content); i++)
    {
        if (script->content[i] != ';')
            lines[currentLine][caret++] = script->content[i];
        // printf("line %d, %c\n", currentLine, script->content[i]);
        if (script->content[i] == ';' || script->content[i] == '{' || script->content[i] == '}')
        {
            lines[currentLine][caret] = '\0';
            // printf("line %s\n", lines[currentLine]);
            currentLine++;
            caret = 0;
            if (currentLine < lineCount)
            {
                // printf("Line: %d, index: %d\n", currentLine, i + 1);
                lineIndexes[currentLine] = i + 1;
            }
        }
    }
    // lines[lineCount - 1][caret] = '\0';

    scriptData->lines = lines;
    scriptData->lineIndexes = lineIndexes;
    scriptData->lineCount = lineCount;

    scriptData->script = script;
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

/*bool IsOperator(char *value)
{
    if (Strcmp(value, "("))
    {
        return true;
    }
    if (Strcmp(value, ")"))
    {
        return true;
    }
    for (int i = 0; i < sizeof(OPERATOR_PRECEDENT_LIST) / sizeof(OperatorPrecedence); i++)
    {
        if (Strcmp(value, OPERATOR_PRECEDENT_LIST[i].operator))
        {
            return true;
        }
    }
    return false;
}*/

/*uint8_t GetPrecedence(char *operator)
{
    if (Strcmp(operator, "("))
    {
        return 0;
    }
    if (Strcmp(operator, ")"))
    {
        return 0;
    }
    for (int i = 0; i < OPERATOR_COUNT; i++)
    {
        if (Strcmp(operator, OPERATOR_PRECEDENT_LIST[i].operator))
        {
            return OPERATOR_PRECEDENT_LIST[i].precedence;
        }
    }
    return 0;
}*/

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
    to->atom = (char *)malloc(sizeof(char) * (strlen(from->atom) + 1));
    strcpy(to->atom, from->atom);
    to->atom[strlen(from->atom)] = '\0';
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
    to->atom = (char *)malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(to->atom, name);
    to->atom[strlen(name)] = '\0';
}

int FloatLength(float value)
{
    bool isDecimal = false;
    while (floor(value) != value)
    {
        value *= 10;
        isDecimal = true;
    }
    printf("get float len %d", IntLength((int)value));
    return IntLength((int)value) + (isDecimal ? 1 : 0);
}
// Returns a malloced char*
char *SerializeVar(EngineVar *variable)
{
    if (variable->currentType == TYPE_STRING)
    {
        char *strCpy = malloc(strlen(variable->data.s) + 1);
        strcpy(strCpy, variable->data.s);
        return strCpy;
    }
    if (variable->currentType == TYPE_INT)
    {
        printf("int type serialize, %d\n", variable->data.i);
        int numberLength = IntLength(variable->data.i);

        printf("length: %d", numberLength);
        char *intChar = malloc(numberLength + 1);
        sprintf(intChar, "%d", variable->data.i);
        printf("out: %s\n", intChar);
        return intChar;
    }
    if (variable->currentType == TYPE_FLOAT)
    {
        int numberLength = FloatLength(variable->data.f);

        char *floatChar = malloc(numberLength + 2);

        snprintf(floatChar, sizeof(floatChar), "%f", variable->data.f);
        return floatChar;
    }
    if (variable->currentType == TYPE_BOOL)
    {
        char *boolChar = malloc(2);
        if (variable->data.b)
        {
            boolChar[0] = '1';
        }
        else
        {
            boolChar[0] = '0';
        }
        boolChar[1] = '\0';
        return boolChar;
    }
    if (variable->currentType == TYPE_VECTOR)
    {
        int xLength = IntLength(variable->data.XY.x);
        int yLength = IntLength(variable->data.XY.y);

        char *vectorChar = malloc(5 + xLength + yLength); // (xlen, ylen)\0
        sprintf(vectorChar, "(%d, %d)", variable->data.XY.x, variable->data.XY.y);
        return vectorChar;
    }

    char *empty = malloc(1);
    empty[0] = '\0';
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
    print("shunt yard");
    Atom operationStack[50] = {0};
    uint8_t operatorIndex = 0;
    Atom operandStack[50] = {0};
    uint8_t operandIndex = 0;

    Atom allAtoms[50] = {0};
    uint16_t atomIndex = 0;

    // Tokenize the input into atoms
    for (int i = 0; i < equationLength; i++)
    {
        printf("tokenize %c\n", equation[i]);

        // If the index is parenthesis, push to stack
        // printf("Finding atom: %c\n", equation[i]);
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

            printf("Atom: %s\n", allAtoms[atomIndex - 1].atom);
            continue;
        }
        if (equation[i] == ')')
        {

            setatom(&allAtoms[atomIndex++], OPERATOR_ATOM, 0, 0, ")");

            printf("Atom: %s\n", allAtoms[atomIndex - 1].atom);
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
                    free(operationStack[i].atom);
                }
                for (int i = 0; i < operandIndex; i++)
                {
                    free(operandStack[i].atom);
                }
                return CreateError(EQUATION_ERROR, SYNTAX_UNKNOWN, 0);
            }

            char *string = malloc(index - i);

            strncpy(string, equation + i + 1, index - i - 1);

            string[index - i - 1] = '\0';

            setatom(&allAtoms[atomIndex++], TYPE_STRING, 0, 0, string);

            free(string);

            printf("Atom: %s\n", allAtoms[atomIndex - 1].atom);

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
            printf("Var name: %s\n", scriptData->data[variable].name);
            if (indexOf(scriptData->data[variable].name, equation + i) == 0)
            {
                print("var match");
                char *value = SerializeVar(&(scriptData->data[variable]));

                if (scriptData->data[variable].currentType == TYPE_FLOAT || scriptData->data[variable].currentType == TYPE_INT)
                {
                    setatom(&allAtoms[atomIndex++], TYPE_FLOAT, 0, 0, value);
                }
                else if (scriptData->data[variable].currentType == TYPE_STRING)
                {
                    setatom(&allAtoms[atomIndex++], TYPE_STRING, 0, 0, value);
                }

                printf("Atom: %s\n", allAtoms[atomIndex - 1].atom);
                isVariable = true;
                i += strlen(scriptData->data[variable].name) - 1;
                break;
            }
        }
        if (isVariable)
        {
            continue;
        }

        // print("Finding operator");
        //  Search through each operator for matches
        bool atomDone = false;
        for (int operator= 0; operator<OPERATOR_COUNT; operator++)
        {
             printf("Operator: %s\n",OPERATOR_PRECEDENT_LIST[operator].operator);
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

                setatom(
                    &allAtoms[atomIndex++],
                    OPERATOR_ATOM,
                    OPERATOR_PRECEDENT_LIST[operator].precedence,
                    OPERATOR_PRECEDENT_LIST[operator].parameters,
                    OPERATOR_PRECEDENT_LIST[operator].operator
                );

                i += operatorLength - 1;
                printf("Atom: %s\n", allAtoms[atomIndex - 1].atom);
                break;
            }
        }

        if (atomDone)
            continue;

        // print("Finding number");
        //  If the current char is a number, continue searching until the end
        uint8_t numberLength = 0;
        while (IsNumber(equation[i + numberLength]))
        {
            // printf("%c\n",equation[i+numberLength]);
            numberLength++;
        }
        if (numberLength == 0)
        {
            for (int i = 0; i < operatorIndex; i++)
            {
                free(operationStack[i].atom);
            }
            for (int i = 0; i < operandIndex; i++)
            {
                free(operandStack[i].atom);
            }
            return CreateError(EQUATION_ERROR, SYNTAX_UNKNOWN, 0);
        }

        allAtoms[atomIndex].atom = (char *)malloc(sizeof(char) * (numberLength + 1));
        if (allAtoms[atomIndex].atom == NULL)
        {
            print("malloc fail");
            for (int i = 0; i < operatorIndex; i++)
            {
                free(operationStack[i].atom);
            }
            for (int i = 0; i < operandIndex; i++)
            {
                free(operandStack[i].atom);
            }
            return CreateError(EQUATION_ERROR, MALLOC_FAIL, 0);
        }
        for (int numberIndex = 0; numberIndex < numberLength; numberIndex++)
        {
            // printf("%c\n", equation[i + numberIndex]);
            allAtoms[atomIndex].atom[numberIndex] = equation[i + numberIndex];
        }
        allAtoms[atomIndex].atom[numberLength] = '\0';
        allAtoms[atomIndex++].type = TYPE_FLOAT;
        i += numberLength - 1;
        printf("Atom: %s\n", allAtoms[atomIndex - 1].atom);
    }

    // At this point, the string should be tokenized. Shunt yard can commence

    for (int i = 0; i < atomIndex; i++)
    {
        printf("Atom shunt yard %s\n", allAtoms[i].atom);

        if (allAtoms[i].atom[0] == ',')
        {
            while (operatorIndex > 0 && operationStack[operatorIndex - 1].atom[0] != '(')
            {
                printf("move from parenthesis %s\n", operationStack[operatorIndex - 1].atom);
                cpyatom(&operandStack[operandIndex++], &operationStack[operatorIndex - 1]);

                printf("moved %s to operand stack\n", operationStack[operatorIndex - 1].atom);
                free(operationStack[--operatorIndex].atom);
            }
            continue;
        }

        if (allAtoms[i].type == OPERATOR_ATOM)
        {
            if (allAtoms[i].atom[0] == ')')
            {
                // If the operator is parenthesis, pop off all of the previous operations until you get to the matching one
                print("parenthesis");
                while (operatorIndex > 0 && operationStack[operatorIndex - 1].atom[0] != '(')
                {
                    printf("move from parenthesis %s\n", operationStack[operatorIndex - 1].atom);
                    cpyatom(&operandStack[operandIndex++], &operationStack[operatorIndex - 1]);

                    printf("moved %s to operand stack\n", operationStack[operatorIndex - 1].atom);
                    free(operationStack[--operatorIndex].atom);
                }
                free(operationStack[--operatorIndex].atom);
            }
            else
            {
                // If operator is not parenthesis, then move operators to the operands IF the precedence is greater or equal, then push operator
                if (allAtoms[i].precedence != 0)
                {
                    while (operatorIndex > 0 && operationStack[operatorIndex - 1].precedence >= allAtoms[i].precedence)
                    {
                        cpyatom(&operandStack[operandIndex++], &operationStack[operatorIndex - 1]);

                        printf("moved %s to operand stack\n", operationStack[operatorIndex - 1].atom);
                        free(operationStack[--operatorIndex].atom);
                    }
                }
                cpyatom(&operationStack[operatorIndex++], &allAtoms[i]);
                printf("added %s to operation stack\n", allAtoms[i].atom);
            }
        }
        else
        {
            cpyatom(&operandStack[operandIndex++], &allAtoms[i]);
            printf("added %s to operand stack\n", allAtoms[i].atom);
        }
        free(allAtoms[i].atom);
    }
    print("shunt done, pushing remain");
    // push remaining operators to output
    while (operatorIndex > 0)
    {
        cpyatom(&operandStack[operandIndex++], &operationStack[operatorIndex - 1]);
        printf("moved %s to operand stack\n", operationStack[operatorIndex - 1].atom);
        free(operationStack[--operatorIndex].atom);
    }

    for (int i = 0; i < operatorIndex; i++)
    {
        free(operationStack[i].atom);
    }

    for (int i = 0; i < operandIndex; i++)
    {
        printf("RPN: %s\n", operandStack[i].atom);
        // free(operandStack[i].atom);
    }

    // The input is now in Reverse Polish Notation

    for (int i = 0; i < operandIndex; i++)
    {
        if (operandStack[i].type == OPERATOR_ATOM)
        {
            bool isValid = true;
            printf("operation: %s\n", operandStack[i].atom);
            EngineVar *parameters = (EngineVar *)malloc(sizeof(EngineVar) * operandStack[i].parameters);
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
                        print("normal param");
                        parameters[operandStack[i].parameters - x - 1].data.f = atof(operandStack[i - x - 1].atom);
                        parameters[operandStack[i].parameters - x - 1].currentType = TYPE_FLOAT;
                    }
                    else if (operandStack[i - x - 1].type == TYPE_VECTOR)
                    {
                        print("vector param");
                        float vectX = 0;
                        float vectY = 0;
                        sscanf((operandStack[i - x - 1].atom), "%f,%f", x, vectY);
                        parameters[operandStack[i].parameters - x - 1].data.XY.x = vectX;
                        parameters[operandStack[i].parameters - x - 1].data.XY.y = vectY;
                        parameters[operandStack[i].parameters - x - 1].currentType = TYPE_VECTOR;
                    }
                    else if (operandStack[i - x - 1].type == TYPE_STRING)
                    {
                        print("string param");
                        parameters[operandStack[i].parameters - x - 1].data.s = malloc(strlen(operandStack[i - x - 1].atom) + 1);
                        strcpy(parameters[operandStack[i].parameters - x - 1].data.s, operandStack[i - x - 1].atom);
                        parameters[operandStack[i].parameters - x - 1].currentType = TYPE_STRING;
                        print("copied");
                    }
                }
            }
            EngineVar *result = VarConstructor("", 0, NO_TYPE);
            if (!isValid)
            {
                print("invalid parameters");
                free(parameters);
                continue;
            }

            if (strcmp(operandStack[i].atom, "+") == 0)
            {
                print("add");
                if (EqualType(&parameters[0], &parameters[1], TYPE_FLOAT))
                {
                    print("type float");
                    result->data.f = parameters[0].data.f + parameters[1].data.f;
                    result->currentType = TYPE_FLOAT;
                }
                if (EqualType(&parameters[0], &parameters[1], TYPE_STRING))
                {
                    print("type string");
                    result->data.s = malloc(strlen(parameters[0].data.s) + strlen(parameters[1].data.s) + 1);
                    sprintf(result->data.s, "%s%s", parameters[0].data.s, parameters[1].data.s);

                    printf("concat result: %s\n", result->data.s);
                    // free(parameters[0].data.s);
                    // free(parameters[1].data.s);
                    result->currentType = TYPE_STRING;
                }
                if (EqualType(&parameters[0], &parameters[1], TYPE_VECTOR))
                {
                    result->data.XY.x = parameters[0].data.XY.x + parameters[1].data.XY.x;
                    result->data.XY.y = parameters[0].data.XY.y + parameters[1].data.XY.y;
                    result->currentType = TYPE_FLOAT;
                }
            }

            if (EqualType(&parameters[0], &parameters[1], TYPE_FLOAT))
            {
                result->currentType = TYPE_FLOAT;
                if (strcmp(operandStack[i].atom, "-") == 0)
                {
                    result->data.f = parameters[0].data.f - parameters[1].data.f;
                }
                if (strcmp(operandStack[i].atom, "*") == 0)
                {
                    result->data.f = parameters[0].data.f * parameters[1].data.f;
                }
                if (strcmp(operandStack[i].atom, "/") == 0)
                {
                    result->data.f = parameters[0].data.f / parameters[1].data.f;
                }
                if (strcmp(operandStack[i].atom, "%") == 0)
                {
                    result->data.f = (int)parameters[0].data.f % (int)parameters[1].data.f;
                }
                if (strcmp(operandStack[i].atom, "cos") == 0)
                {
                    result->data.f = cos(parameters[0].data.f);
                }
                if (strcmp(operandStack[i].atom, "sin") == 0)
                {
                    result->data.f = sin(parameters[0].data.f);
                }
                if (strcmp(operandStack[i].atom, "pow") == 0)
                {
                    result->data.f = pow(parameters[0].data.f, parameters[1].data.f);
                }
                if (strcmp(operandStack[i].atom, "PI") == 0)
                {
                    result->data.f = 3.14159265459;
                }

                if (strcmp(operandStack[i].atom, "|") == 0)
                {
                    result->data.f = (float)((int)parameters[0].data.f | (int)parameters[1].data.f);
                }
                if (strcmp(operandStack[i].atom, "&") == 0)
                {
                    result->data.f = (float)((int)parameters[0].data.f & (int)parameters[1].data.f);
                }
                if (strcmp(operandStack[i].atom, "^") == 0)
                {
                    result->data.f = (float)((int)parameters[0].data.f ^ (int)parameters[1].data.f);
                }

                if (strcmp(operandStack[i].atom, "==") == 0)
                {
                    result->data.f = (parameters[0].data.f == parameters[1].data.f) ? 1 : 0;
                }
                if (strcmp(operandStack[i].atom, "<") == 0)
                {
                    result->data.f = (parameters[0].data.f < parameters[1].data.f) ? 1 : 0;
                }
                if (strcmp(operandStack[i].atom, ">") == 0)
                {
                    result->data.f = (parameters[0].data.f > parameters[1].data.f) ? 1 : 0;
                }
                if (strcmp(operandStack[i].atom, "<=") == 0)
                {
                    result->data.f = (parameters[0].data.f <= parameters[1].data.f) ? 1 : 0;
                }
                if (strcmp(operandStack[i].atom, ">=") == 0)
                {
                    result->data.f = (parameters[0].data.f >= parameters[1].data.f) ? 1 : 0;
                }
                if (strcmp(operandStack[i].atom, "!=") == 0)
                {
                    result->data.f = (parameters[0].data.f != parameters[1].data.f) ? 1 : 0;
                }
            }

            if (EqualType(&parameters[0], &parameters[1], TYPE_VECTOR))
            {
                if (strcmp(operandStack[i].atom, "-") == 0)
                {
                    result->data.XY.x = parameters[0].data.XY.x - parameters[1].data.XY.x;
                    result->data.XY.y = parameters[0].data.XY.y - parameters[1].data.XY.y;
                    result->currentType = TYPE_FLOAT;
                }
                if (strcmp(operandStack[i].atom, "*") == 0)
                {
                    result->data.XY.x = parameters[0].data.XY.x * parameters[1].data.XY.x;
                    result->data.XY.y = parameters[0].data.XY.y * parameters[1].data.XY.y;
                    result->currentType = TYPE_FLOAT;
                }
                if (strcmp(operandStack[i].atom, "/") == 0)
                {
                    result->data.XY.x = parameters[0].data.XY.x / parameters[1].data.XY.x;
                    result->data.XY.y = parameters[0].data.XY.y / parameters[1].data.XY.y;
                    result->currentType = TYPE_FLOAT;
                }
            }

            if (operandStack[i].parameters > 0)
            {
                for (int x = i + 1; x < operandIndex; x++)
                {
                    free(operandStack[x - operandStack[i].parameters].atom);
                    cpyatom(&operandStack[x - operandStack[i].parameters], &operandStack[x]);
                }
                for (int x = 0; x < operandStack[i].parameters; x++)
                {
                    free(operandStack[operandIndex - x - 1].atom);
                }
            }
            operandIndex -= operandStack[i].parameters;

            free(operandStack[i - operandStack[i].parameters].atom);

            if (result->currentType == TYPE_FLOAT)
            {
                printf("Completed an float operation result: %f\n", result->data.f);

                char strResult[20];

                snprintf(strResult, sizeof(strResult), "%f", result->data.f);

                setatom(&operandStack[i - operandStack[i].parameters], TYPE_FLOAT, 0, 0, strResult);
            }
            else
            {
                printf("Completed an str operation result: %s\n", result->data.s);

                setatom(&operandStack[i - operandStack[i].parameters], TYPE_STRING, 0, 0, result->data.s);
            }

            free(parameters);

            i = 0;

            for (int x = 0; x < operandIndex; x++)
            {
                printf("OUT: %s\n", operandStack[x].atom);
            }
            print("");
        }
    }

    if (operandIndex > 1)
    {
        for (int i = 0; i < operandIndex; i++)
        {
            // printf("atom: %s\n", operandStack[i].atom);
            free(operandStack[i].atom);
        }
        return CreateError(EQUATION_ERROR, EQUATION_UNSOLVED, 0);
    }
    else
    {
        if (operandStack[0].type == TYPE_FLOAT)
        {

            output->data.f = atof(operandStack[0].atom);

            if (floor(output->data.f) == output->data.f)
            {
                output->data.i = floor(output->data.f);
                output->currentType = TYPE_INT;
                print("type int");
            }
            else
            {
                output->currentType = TYPE_FLOAT;
                print("type float");
            }
        }
        if (operandStack[0].type == TYPE_STRING)
        {
            output->currentType = TYPE_STRING;
            output->data.s = malloc(strlen(operandStack[0].atom) + 1);
            strcpy(output->data.s, operandStack[0].atom);
        }
    }

    for (int i = 0; i < operandIndex; i++)
    {
        free(operandStack[i].atom);
    }

    return 0;
}

char *RemoveSpaces(char *in)
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
}

// DOES NOT FREE INPUT AND OUTPUT IS MALLOCED
char *LeftTrim(char *str)
{
    int index = 0;
    while (str[index] <= 32 && str[index] != '\0')
    {
        index++;
    }
    if (str[index] == '\0')
    {
        return NULL;
    }

    char *output = (char *)malloc(strlen(str) - index + 1);

    strcpy(output, str + index);
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

//  !!!DEPRECEATED USE SHUNT YARD!!! The data must not have a left space and must be null terminated
uint32_t GetData(char *data, EngineVar *output, ScriptData *scriptData)
{
    if (data[0] == '\"') // string
    {                    // string
        uint16_t index = 1;
        while (data[index] != '"' && data[index] != '\0')
        {
            index++;
        }
        if (data[index] == '\0')
        {
            return CreateError(DATATYPE_ERROR, SYNTAX_UNKNOWN, 0);
        }
        if (output->currentType != NO_TYPE && output->currentType != TYPE_STRING)
        {
            return CreateError(DATATYPE_ERROR, TYPE_MISMATCH, 0);
        }
        // printf("getData: %s\n", data + 1);
        output->data.s = (char *)malloc(index);
        strncpy(output->data.s, data + 1, index - 1);
        output->data.s[index - 1] = '\0';
        // printf("getData result: %s\n", output->data.s);
        output->currentType = TYPE_STRING;
        return 0;
    }
    if (indexOf("vector(", data) == 0) // Vector
    {
        uint8_t bracketCount = 0;
        uint16_t index = 7;
        while ((data[index] != ',' || bracketCount > 0) && data[index] != '\0')
        {
            if (data[index] == '(')
            {
                bracketCount++;
            }
            else if (data[index] == ')')
            {
                bracketCount--;
            }
            index++;
        }
        if (data[index] == '\0')
        {
            return CreateError(DATATYPE_ERROR, SYNTAX_UNKNOWN, 0);
        }
        EngineVar *xValue = VarConstructor("", 0, NO_TYPE);
        uint32_t error = ShuntYard(data + 7, index - 7, xValue, scriptData);
        if (error != 0)
        {
            return error;
        }

        if (xValue->currentType == TYPE_FLOAT)
        {
            xValue->data.i = floor(xValue->data.f);
            xValue->currentType = TYPE_INT;
        }
        if (xValue->currentType != TYPE_INT)
        {
            return CreateError(DATATYPE_ERROR, TYPE_MISMATCH, 0);
        }
        printf("xValue: %f\n", xValue);

        uint8_t commaLocation = index;

        index++;
        while ((data[index] != ')' || bracketCount > 0) && data[index] != '\0')
        {
            if (data[index] == '(')
            {
                bracketCount++;
            }
            else if (data[index] == ')')
            {
                bracketCount--;
            }
            index++;
        }
        if (data[index] == '\0')
        {
            return CreateError(DATATYPE_ERROR, SYNTAX_UNKNOWN, 0);
        }
        EngineVar *yValue = VarConstructor("", 0, NO_TYPE);
        error = ShuntYard(data + commaLocation + 1, index - commaLocation - 1, yValue, scriptData);
        if (error != 0)
        {
            return error;
        }

        if (yValue->currentType == TYPE_FLOAT)
        {
            yValue->data.i = floor(yValue->data.f);
            yValue->currentType = TYPE_INT;
        }
        if (yValue->currentType != TYPE_INT)
        {
            return CreateError(DATATYPE_ERROR, TYPE_MISMATCH, 0);
        }
        printf("yValue: %f\n", xValue);

        if (output->currentType != NO_TYPE && output->currentType != TYPE_VECTOR)
        {
            return CreateError(DATATYPE_ERROR, TYPE_MISMATCH, 0);
        }

        output->data.XY.x = xValue->data.XY.x;
        output->data.XY.y = yValue->data.XY.y;
        output->currentType = TYPE_VECTOR;
        return 0;
    }
    if (indexOf("false", data) == 0)
    {
        if (output->currentType != NO_TYPE && output->currentType != TYPE_BOOL)
        {
            return CreateError(DATATYPE_ERROR, TYPE_MISMATCH, 0);
        }
        output->data.b = false;
        output->currentType = TYPE_BOOL;
        return 0;
    }
    if (indexOf("true", data) == 0)
    {
        if (output->currentType != NO_TYPE && output->currentType != TYPE_BOOL)
        {
            return CreateError(DATATYPE_ERROR, TYPE_MISMATCH, 0);
        }
        output->data.b = true;
        output->currentType = TYPE_BOOL;
        return 0;
    }

    EngineVar *valueOut = VarConstructor("", 0, NO_TYPE);
    uint32_t error = ShuntYard(data, strlen(data), valueOut, scriptData);
    if (error != 0)
    {
        return error;
    }

    if (valueOut->currentType == TYPE_INT && (output->currentType == TYPE_INT || output->currentType == NO_TYPE))
    {
        output->data.i = valueOut->data.i;
        output->currentType = TYPE_INT;
        return 0;
    }
    else if (valueOut->currentType == TYPE_FLOAT && output->currentType == TYPE_INT)
    {
        output->data.i = floor(valueOut->data.f);
        output->currentType = TYPE_INT;
        return 0;
    }
    else if (valueOut->currentType == TYPE_FLOAT && (output->currentType == TYPE_FLOAT || output->currentType == NO_TYPE))
    {
        output->data.f = valueOut->data.f;
        output->currentType = TYPE_FLOAT;
        return 0;
    }
    else if (valueOut->currentType == TYPE_STRING && (output->currentType == TYPE_STRING || output->currentType == NO_TYPE))
    {
        output->data.s = valueOut->data.s;
        output->currentType = TYPE_STRING;
        return 0;
    }

    return CreateError(DATATYPE_ERROR, TYPE_MISMATCH, 0);
}

uint32_t DeclareEntity(ScriptData *output, uint16_t l)
{
    char *trimmedLine = LeftTrim(output->lines[l]);
    if (trimmedLine == NULL)
    {
        free(trimmedLine);
        return CreateError(DATATYPE_ERROR, SYNTAX_UNKNOWN, 0);
    }
    printf("trimmed line: %s\n", trimmedLine);
    int varType = GetTypeFromString(trimmedLine);
    printf("got type %d\n", varType);
    if (varType != NO_TYPE)
    {
        int index = 0;
        // Move index to space after type
        while (trimmedLine[index] > 32) // if it is character
        {
            index++;
        }
        if (trimmedLine[index] == '\0')
        {
            free(trimmedLine);
            return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
        }
        index++;

        uint8_t nameLength = 0;
        while (
            IsAlphaNumeric(trimmedLine[index + nameLength])) // if it is character
        {
            nameLength++;
        }
        if (trimmedLine[index + nameLength] == '\0')
        {
            free(trimmedLine);
            return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
        }
        char *entityName = (char *)malloc(nameLength + 1);

        strncpy(entityName, trimmedLine + index, nameLength);
        entityName[nameLength] = '\0';

        printf("Type: %d, Name: %s\n", varType, entityName);
        index += nameLength;

        printf("next char: %c\n", trimmedLine[index]);

        while (trimmedLine[index] <= 32) // if it is not a character
        {
            index++;
        }
        if (trimmedLine[index] == '\0')
        {
            free(entityName);
            free(trimmedLine);
            return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
        }

        if (trimmedLine[index] == '(')
        { // function declare
            print("declare function");
            index++;
            EngineFunction *newFunction = (EngineFunction *)malloc(sizeof(EngineFunction));
            newFunction->parameterIndex = 0;
            while (trimmedLine[index] != ')' && trimmedLine[index] != '\0')
            {
                while (trimmedLine[index] <= 32)
                {
                    index++;
                }

                uint8_t parameterType = GetTypeFromString(trimmedLine + index);

                printf("parameter type: %d\n", parameterType);

                // loop until space after type
                while (trimmedLine[index] > 32)
                {
                    index++;
                }
                if (trimmedLine[index] == '\0')
                {
                    free(trimmedLine);
                    free(entityName);
                    free(newFunction);
                    return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
                }
                index++;

                print("data space");

                // get length
                uint8_t parameterNameLength = 0;
                while (IsAlphaNumeric(trimmedLine[index + parameterNameLength])) // if it is character
                {
                    parameterNameLength++;
                }
                if (trimmedLine[index + parameterNameLength] == '\0')
                {
                    free(trimmedLine);
                    free(entityName);
                    free(newFunction);
                    return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
                }
                print("got param len");
                // copy name and type to parameter
                newFunction->parameters[newFunction->parameterIndex].parameterName = (char *)malloc(parameterNameLength + 1);
                strncpy(
                    newFunction->parameters[newFunction->parameterIndex].parameterName,
                    trimmedLine + index,
                    parameterNameLength);
                newFunction->parameters[newFunction->parameterIndex].parameterName[parameterNameLength] = '\0';
                newFunction->parameters[newFunction->parameterIndex].parameterType = parameterType;
                printf("Param Index: %d, Type %d, Name %s\n",
                       newFunction->parameterIndex,
                       newFunction->parameters[newFunction->parameterIndex].parameterType,
                       newFunction->parameters[newFunction->parameterIndex].parameterName);
                newFunction->parameterIndex++;
                index += parameterNameLength;
                while (trimmedLine[index] < 32 && trimmedLine[index] != '\0')
                {
                    index++;
                }
                if (trimmedLine[index] == ',')
                {
                    index++;
                }
                printf("rest of line: %s\n", trimmedLine + index);
            }
            while (trimmedLine[index] != '{' && trimmedLine[index] != '\0')
            {
                index++;
            }
            printf("line index: %d", output->lineIndexes[l]);
            if (trimmedLine[index] != '{')
            {
                free(entityName);
                free(newFunction);
                free(trimmedLine);
                return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
            }

            int bracketStart = indexOf("{", output->lines[l]) + output->lineIndexes[l];
            printf("bracket pos: %d\n", bracketStart);
            newFunction->bracketStart = bracketStart;

            bool foundEnd = false;
            for (int i = 0; i < output->bracketPairs; i++)
            {
                if (output->brackets[i].startPos == bracketStart)
                {
                    foundEnd = true;
                    printf("end bracket: %d\n", output->brackets[i].endPos);
                    newFunction->bracketEnd = output->brackets[i].endPos;
                    break;
                }
            }
            if (!foundEnd)
            {
                free(entityName);
                free(newFunction);
                free(trimmedLine);
                return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
            }

            output->functions[output->functionCount] = newFunction;
            output->functions[output->functionCount]->name = (char *)malloc(sizeof(char) * (strlen(entityName) + 1));
            strcpy(output->functions[output->functionCount]->name, entityName);
            free(entityName);
            output->functionCount++;
            print("declare fin");

            free(trimmedLine);
            return OPPERATION_SUCCESS;
        }
        else
        { // variable declare
            output->data[output->variableCount].currentType = varType;
            strncpy(output->data[output->variableCount].name, entityName, MAX_NAME_LENGTH - 1);
            output->data[output->variableCount].name[MAX_NAME_LENGTH - 1] = '\0';

            while (trimmedLine[index] <= 32 && trimmedLine[index] != '\0')
            {
                index++;
            }
            if (trimmedLine[index] != '=')
            {
                free(trimmedLine);
                return OPPERATION_SUCCESS;
            }
            index++;
            while (trimmedLine[index] <= 32 && trimmedLine[index] != '\0')
            {
                index++;
            }
            if (trimmedLine[index] == '\0')
            {
                free(trimmedLine);
                return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, l);
            }

            printf("right side: %s\n", trimmedLine + index);

            uint32_t error = ShuntYard(trimmedLine + index, strlen(trimmedLine + index), &(output->data[output->variableCount]), output);

            if (error != 0)
            {
                free(trimmedLine);
                return error | ((uint16_t)l & 0xFFFF);
            }
            printf("variable declared: %d\n", output->data[output->variableCount].currentType);
            output->variableCount++;
            free(trimmedLine);
            return OPPERATION_SUCCESS;
        }
    }

    free(trimmedLine);
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
    printf("line: %d, scope: %d\n", line, level);
    return level;
}

uint32_t SetScriptData(EngineScript *script, ScriptData *output, uint8_t scopeLevel)
{

    uint32_t error = 0;

    SplitScript(script, output);

    error = CalculateBrackets(script, output);
    if (error != 0)
    {
        return error;
    }

    for (int l = 0; l < output->lineCount; l++)
    {
        // Find and add all variables/functions to the script data

        printf("Set scr data: line: %s\n", output->lines[l]);

        if (GetScope(output, l) != scopeLevel)
        {
            continue;
        }

        error = DeclareEntity(output, l);

        if (error != 0 && error != OPPERATION_SUCCESS)
        {
            print("set scr error");
            return error;
        }
    }
    print("set script done");
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

    // print("free scr data 1");

    for (int i = 0; i < scriptData->functionCount; i++)
    {
        free(scriptData->functions[i]->name);
        // print("free scr data 1a");
        free(scriptData->functions[i]);
        // print("free scr data 1b");
    }
    // print("free scr data 2");
    scriptData->lineCount = 0;
    scriptData->bracketPairs = 0;
    scriptData->functionCount = 0;

    if (!onlyFreeContent)
        free(scriptData);
}

uint32_t ExecuteLine(EngineScript *script, ScriptData *scriptData)
{
    char *trimmedLine = LeftTrim(scriptData->lines[scriptData->currentLine]);
    printf("Exexute line: %s\n", trimmedLine);
    // If line is a variable or function declaration, ignore
    if (GetTypeFromString(trimmedLine) != NO_TYPE)
    {
        free(trimmedLine);
        scriptData->currentLine++;
        return 0;
    }
    // If line only includes end of a curly bracket
    if (trimmedLine[0] == '}')
    {
        print("jumpback");
        int bracketEnd = scriptData->lineIndexes[scriptData->currentLine] + indexOf("}", scriptData->lines[scriptData->currentLine]);

        bool foundEnd = false;
        for (int i = 0; i < scriptData->bracketPairs; i++)
        {
            if (scriptData->brackets[i].endPos == bracketEnd)
            {
                foundEnd = true;
                // printf("end bracket: %d\n", scriptData->brackets[i].endPos);

                for (int x = 0; x < scriptData->lineCount - 1; x++)
                {
                    if (
                        scriptData->lineIndexes[x] <= scriptData->brackets[i].startPos &&
                        scriptData->lineIndexes[x + 1] > scriptData->brackets[i].startPos)
                    {
                        if (indexOf("while", scriptData->lines[x]) != -1)
                        {
                            scriptData->currentLine = x;
                            printf("jumpback %d\n", scriptData->currentLine);
                        }
                        else
                        {
                            print("end brack was not a loop");
                            free(trimmedLine);
                            scriptData->currentLine++;
                            print("bracket execute a");
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
                        printf("jumpback %d\n", scriptData->currentLine);
                    }
                    else
                    {
                        print("end brack was not a loop");
                        free(trimmedLine);
                        scriptData->currentLine++;
                        print("bracket execute b");
                        return 0;
                    }
                }

                break;
            }
        }
        print("bracket execute 1");
        if (!foundEnd)
        {
            print("did not find end");
            free(trimmedLine);
            return CreateError(GENERAL_ERROR, BRACKET_MISMATCH, scriptData->lineCount);
        }
        return 0;
    }
    print("bracket execute 2");

    // Set variable

    int varIndex = -1;
    for (int i = 0; i < scriptData->variableCount; i++)
    {
        if (indexOf(scriptData->data[i].name, trimmedLine) == 0)
        {
            varIndex = i;
            break;
        }
    }

    if (varIndex != -1)
    {
        int index = indexOf("=", trimmedLine) - 1;
        uint8_t assignType = trimmedLine[index];

        // create a variable to store the assignment result
        EngineVar *value = VarConstructor("", 0, 0);

        index += 2;

        while (trimmedLine[index] <= 32 && trimmedLine[index] != '\0')
        {
            index++;
        }
        printf("assign value :%s\n", trimmedLine + index);

        uint32_t error = ShuntYard(trimmedLine + index, strlen(trimmedLine + index), value, scriptData);

        if (error != 0)
        {
            free(trimmedLine);
            return error | scriptData->currentLine;
        }

        if (value->currentType != scriptData->data[varIndex].currentType)
        {
            free(trimmedLine);
            return CreateError(VARIABLE_ERROR, TYPE_MISMATCH, scriptData->currentLine);
        }

        switch (assignType)
        {
        default:
            if (EqualType(&scriptData->data[varIndex], value, TYPE_BOOL))
            {
                scriptData->data[varIndex].data.b = value->data.b;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_VECTOR))
            {
                scriptData->data[varIndex].data.XY.x = value->data.XY.x;
                scriptData->data[varIndex].data.XY.y = value->data.XY.y;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_FLOAT))
            {
                scriptData->data[varIndex].data.f = value->data.f;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_INT))
            {
                scriptData->data[varIndex].data.i = value->data.i;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_STRING))
            {
                char *temp = malloc(strlen(value->data.s) + 1);
                sprintf(temp, "%s", value->data.s);

                free(scriptData->data[varIndex].data.s);
                free(value->data.s);

                scriptData->data[varIndex].data.s = temp;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_OBJ))
            {
                scriptData->data[varIndex].data.objID = value->data.objID;
            }
            free(trimmedLine);
            return CreateError(VARIABLE_ERROR, TYPE_MISMATCH, scriptData->currentLine);
            break;
        case '+':
            if (EqualType(&scriptData->data[varIndex], value, TYPE_BOOL))
            {
                scriptData->data[varIndex].data.b = value->data.b | scriptData->data[varIndex].data.b;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_VECTOR))
            {
                scriptData->data[varIndex].data.XY.x = value->data.XY.x + scriptData->data[varIndex].data.XY.x;
                scriptData->data[varIndex].data.XY.y = value->data.XY.y + scriptData->data[varIndex].data.XY.y;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_FLOAT))
            {
                scriptData->data[varIndex].data.f = value->data.f + scriptData->data[varIndex].data.f;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_INT))
            {
                scriptData->data[varIndex].data.i = value->data.i + scriptData->data[varIndex].data.i;
                printf("added to int: %d\n", scriptData->data[varIndex].data.i);
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_STRING))
            {
                char *temp = malloc(strlen(scriptData->data[varIndex].data.s) + strlen(value->data.s) + 1);
                sprintf(temp, "%s%s", scriptData->data[varIndex].data.s, value->data.s);

                free(scriptData->data[varIndex].data.s);
                free(value->data.s);

                scriptData->data[varIndex].data.s = temp;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_OBJ))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            free(trimmedLine);
            return CreateError(VARIABLE_ERROR, TYPE_MISMATCH, scriptData->currentLine);
            break;
        case '-':
            if (EqualType(&scriptData->data[varIndex], value, TYPE_BOOL))
            {
                scriptData->data[varIndex].data.b = (value->data.b & !scriptData->data[varIndex].data.b);
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_VECTOR))
            {
                scriptData->data[varIndex].data.XY.x = value->data.XY.x - scriptData->data[varIndex].data.XY.x;
                scriptData->data[varIndex].data.XY.y = value->data.XY.y - scriptData->data[varIndex].data.XY.y;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_FLOAT))
            {
                scriptData->data[varIndex].data.f = value->data.f - scriptData->data[varIndex].data.f;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_INT))
            {
                scriptData->data[varIndex].data.i = value->data.i - scriptData->data[varIndex].data.i;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_STRING))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_OBJ))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            free(trimmedLine);
            return CreateError(VARIABLE_ERROR, TYPE_MISMATCH, scriptData->currentLine);
            break;
        case '*':
            if (EqualType(&scriptData->data[varIndex], value, TYPE_BOOL))
            {
                scriptData->data[varIndex].data.b = (value->data.b & scriptData->data[varIndex].data.b);
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_VECTOR))
            {
                scriptData->data[varIndex].data.XY.x = value->data.XY.x * scriptData->data[varIndex].data.XY.x;
                scriptData->data[varIndex].data.XY.y = value->data.XY.y * scriptData->data[varIndex].data.XY.y;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_FLOAT))
            {
                scriptData->data[varIndex].data.f = value->data.f * scriptData->data[varIndex].data.f;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_INT))
            {
                scriptData->data[varIndex].data.i = value->data.i * scriptData->data[varIndex].data.i;
                printf("multiplied ints: %d\n", scriptData->data[varIndex].data.i);
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_STRING))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_OBJ))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            free(trimmedLine);
            return CreateError(VARIABLE_ERROR, TYPE_MISMATCH, scriptData->currentLine);
            break;
        case '/':
            if (EqualType(&scriptData->data[varIndex], value, TYPE_BOOL))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_VECTOR))
            {
                scriptData->data[varIndex].data.XY.x = value->data.XY.x / scriptData->data[varIndex].data.XY.x;
                scriptData->data[varIndex].data.XY.y = value->data.XY.y / scriptData->data[varIndex].data.XY.y;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_FLOAT))
            {
                scriptData->data[varIndex].data.f = value->data.f / scriptData->data[varIndex].data.f;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_INT))
            {
                scriptData->data[varIndex].data.i = value->data.i / scriptData->data[varIndex].data.i;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_STRING))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_OBJ))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            free(trimmedLine);
            return CreateError(VARIABLE_ERROR, TYPE_MISMATCH, scriptData->currentLine);
            break;
        case '|':
            if (EqualType(&scriptData->data[varIndex], value, TYPE_BOOL))
            {
                scriptData->data[varIndex].data.b = (value->data.b || scriptData->data[varIndex].data.b);
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_VECTOR))
            {
                scriptData->data[varIndex].data.XY.x = value->data.XY.x | scriptData->data[varIndex].data.XY.x;
                scriptData->data[varIndex].data.XY.y = value->data.XY.y | scriptData->data[varIndex].data.XY.y;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_FLOAT))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_INT))
            {
                scriptData->data[varIndex].data.i = value->data.i | scriptData->data[varIndex].data.i;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_STRING))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_OBJ))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            free(trimmedLine);
            return CreateError(VARIABLE_ERROR, TYPE_MISMATCH, scriptData->currentLine);
            break;
        case '&':
            if (EqualType(&scriptData->data[varIndex], value, TYPE_BOOL))
            {
                scriptData->data[varIndex].data.b = (value->data.b && scriptData->data[varIndex].data.b);
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_VECTOR))
            {
                scriptData->data[varIndex].data.XY.x = value->data.XY.x & scriptData->data[varIndex].data.XY.x;
                scriptData->data[varIndex].data.XY.y = value->data.XY.y & scriptData->data[varIndex].data.XY.y;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_FLOAT))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_INT))
            {
                scriptData->data[varIndex].data.i = value->data.i & scriptData->data[varIndex].data.i;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_STRING))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_OBJ))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            free(trimmedLine);
            return CreateError(VARIABLE_ERROR, TYPE_MISMATCH, scriptData->currentLine);
            break;
        case '^':
            if (EqualType(&scriptData->data[varIndex], value, TYPE_BOOL))
            {
                scriptData->data[varIndex].data.b = (value->data.b ^ scriptData->data[varIndex].data.b);
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_VECTOR))
            {
                scriptData->data[varIndex].data.XY.x = value->data.XY.x ^ scriptData->data[varIndex].data.XY.x;
                scriptData->data[varIndex].data.XY.y = value->data.XY.y ^ scriptData->data[varIndex].data.XY.y;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_FLOAT))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_INT))
            {
                scriptData->data[varIndex].data.i = value->data.i ^ scriptData->data[varIndex].data.i;
                break;
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_STRING))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            if (EqualType(&scriptData->data[varIndex], value, TYPE_OBJ))
            {
                free(trimmedLine);
                return CreateError(VARIABLE_ERROR, OPERATOR_CANNOT_BE_USED_WITH_TYPE, scriptData->currentLine);
            }
            free(trimmedLine);
            return CreateError(VARIABLE_ERROR, TYPE_MISMATCH, scriptData->currentLine);
            break;
        }
    }

    // If statement
    if (indexOf("if", trimmedLine) == 0)
    {
        int index = 2;
        while (trimmedLine[index] != '(' && trimmedLine[index] != '\0')
        {
            index++;
        }
        if (trimmedLine[index] == '\0')
        {
            free(trimmedLine);
            return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, scriptData->currentLine);
        }
        index++;

        int endIndex = index;
        uint8_t parenthesisCount = 1;

        while (trimmedLine[endIndex] != '\0' && parenthesisCount > 0)
        {
            if (trimmedLine[endIndex] == ')')
            {
                parenthesisCount--;
            }
            if (trimmedLine[endIndex] == '(')
            {
                parenthesisCount++;
            }
            endIndex++;
        }
        endIndex--;

        EngineVar *value = VarConstructor("", 0, NO_TYPE);
        uint32_t error = ShuntYard(trimmedLine + index, endIndex - index, value, scriptData);
        if (error != 0)
        {
            free(trimmedLine);
            return error | scriptData->currentLine;
        }

        bool isZero = false;
        if (value->currentType == TYPE_FLOAT)
        {
            isZero = (floor(value->data.f) == 0) ? 1 : 0;
        }
        else if (value->currentType == TYPE_INT)
        {
            isZero = (floor(value->data.i) == 0) ? 1 : 0;
        }
        else
        {
            free(trimmedLine);
            return CreateError(GENERAL_ERROR, TYPE_MISMATCH, scriptData->currentLine);
        }

        if (isZero)
        {
            int bracketStart = indexOf("{", scriptData->lines[scriptData->currentLine]) + scriptData->lineIndexes[scriptData->currentLine];
            printf("bracket pos: %d\n", bracketStart);

            bool foundEnd = false;
            for (int i = 0; i < scriptData->bracketPairs; i++)
            {
                if (scriptData->brackets[i].startPos == bracketStart)
                {
                    foundEnd = true;
                    printf("end bracket: %d\n", scriptData->brackets[i].endPos);

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

                    free(trimmedLine);
                    printf("Script line: %d\n", scriptData->currentLine);
                    return 0;
                }
            }

            if (!foundEnd)
            {
                free(trimmedLine);
                return CreateError(GENERAL_ERROR, BRACKET_MISMATCH, scriptData->lineCount);
            }
        }
    }

    // While statement
    if (indexOf("while", trimmedLine) == 0)
    {
        int index = 2;
        while (trimmedLine[index] != '(' && trimmedLine[index] != '\0')
        {
            index++;
        }
        if (trimmedLine[index] == '\0')
        {
            free(trimmedLine);
            return CreateError(GENERAL_ERROR, SYNTAX_UNKNOWN, scriptData->currentLine);
        }
        index++;

        print("while found (");

        int endIndex = index;
        uint8_t parenthesisCount = 1;

        while (trimmedLine[endIndex] != '\0' && parenthesisCount > 0)
        {
            if (trimmedLine[endIndex] == ')')
            {
                parenthesisCount--;
            }
            if (trimmedLine[endIndex] == '(')
            {
                parenthesisCount++;
            }
            endIndex++;
        }
        endIndex--;

        print("while found )");

        EngineVar *value = VarConstructor("", 0, NO_TYPE);
        uint32_t error = ShuntYard(trimmedLine + index, endIndex - index, value, scriptData);
        if (error != 0)
        {
            free(trimmedLine);
            return error | scriptData->currentLine;
        }

        bool isZero = false;
        if (value->currentType == TYPE_FLOAT)
        {
            isZero = (floor(value->data.f) == 0) ? 1 : 0;
        }
        else if (value->currentType == TYPE_INT)
        {
            isZero = (floor(value->data.i) == 0) ? 1 : 0;
        }
        else
        {
            free(trimmedLine);
            return CreateError(GENERAL_ERROR, TYPE_MISMATCH, scriptData->currentLine);
        }

        if (isZero)
        {
            int bracketStart = indexOf("{", scriptData->lines[scriptData->currentLine]) + scriptData->lineIndexes[scriptData->currentLine];
            printf("bracket pos: %d\n", bracketStart);

            bool foundEnd = false;
            for (int i = 0; i < scriptData->bracketPairs; i++)
            {
                if (scriptData->brackets[i].startPos == bracketStart)
                {
                    foundEnd = true;
                    printf("end bracket: %d\n", scriptData->brackets[i].endPos);

                    for (int x = 0; x < scriptData->lineCount - 1; x++)
                    {

                        printf("(%d,%d): %d\n", scriptData->lineIndexes[x], scriptData->lineIndexes[x + 1], scriptData->brackets[i].endPos);
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

                    free(trimmedLine);
                    printf("Script line: %d\n", scriptData->currentLine);
                    return 0;
                }
            }
            if (!foundEnd)
            {
                free(trimmedLine);
                return CreateError(GENERAL_ERROR, BRACKET_MISMATCH, scriptData->lineCount);
            }
        }
    }

    if (indexOf("print", trimmedLine) == 0)
    {
        int start = indexOf("(", trimmedLine) + 1;
        int endIndex = start;
        uint8_t parenthesisCount = 1;

        while (trimmedLine[endIndex] != '\0' && parenthesisCount > 0)
        {
            if (trimmedLine[endIndex] == ')')
            {
                parenthesisCount--;
            }
            if (trimmedLine[endIndex] == '(')
            {
                parenthesisCount++;
            }
            endIndex++;
        }
        endIndex--;
        EngineVar *out = VarConstructor("", 0, NO_TYPE);
        uint32_t error = ShuntYard(trimmedLine + start, endIndex - start, out, scriptData);
        if (error != 0)
        {
            free(trimmedLine);
            return error | scriptData->currentLine;
        }

        if (out->currentType == TYPE_FLOAT || out->currentType == TYPE_INT)
        {
            int outLength = 0;

            char *printMessage;

            if (out->currentType == TYPE_FLOAT)
            {
                outLength = strlen(scriptData->script->name) + FloatLength(out->data.f) + 4;
                printMessage = malloc(outLength);
                snprintf(printMessage, outLength, "(%s) %f", scriptData->script->name, out->data.f);
            }
            else
            {
                outLength = strlen(scriptData->script->name) + FloatLength(out->data.i) + 4;
                printMessage = malloc(outLength);
                snprintf(printMessage, outLength, "(%s) %d", scriptData->script->name, out->data.i);
            }

            printf("float message print: %s\n", printMessage);

            UI_PrintToScreen(printMessage, false);
        }
        else if (out->currentType == TYPE_STRING)
        {
            printf("str message print: %s\n", out->data.s);

            int outLength = strlen(scriptData->script->name) + strlen(out->data.s) + 4;
            char *printMessage = malloc(outLength);
            snprintf(printMessage, outLength, "(%s) %s", scriptData->script->name, out->data.s);

            UI_PrintToScreen(printMessage, false);
        }
        free(out);
    }
    scriptData->currentLine++;
    printf("Script line: %d\n", scriptData->currentLine);

    free(trimmedLine);

    return 0;
}

/*uint32_t RunLine(char *line, ScriptData *scriptData)
{

    int varIndex = indexOf("var", line);
    if (varIndex != -1)
    {
        print("found var");
        int varStart = varIndex + 3;
        while (line[varStart] <= 32)
        {
            varStart++;
        }
        printf("start: %d\n", varStart);
        int varEnd = varStart;
        while (line[varEnd] > 32 && line[varEnd] != '=' && varEnd < strlen(line))
        {
            varEnd++;
        }
        printf("end: %d\n", varEnd);
        scriptData->data[scriptData->variableCount].currentType = TYPE_FLOAT;
        print("set type");
        for (int i = 0; i < MAX_NAME_LENGTH; i++)
        {
            if (i < varEnd - varStart)
            {
                printf("name: %c\n", line[varStart + i]);
                scriptData->data[scriptData->variableCount].name[i] = line[varStart + i];
            }
            else
            {
                scriptData->data[scriptData->variableCount].name[i] = '\0';
            }
        }
        scriptData->data[scriptData->variableCount++].data.f = 0;
        printf("Variable: %s\n", scriptData->data[scriptData->variableCount - 1].name);
        return 0;
    }

    int setIndex = indexOf("=", line);
    if (setIndex != -1 && line[setIndex + 1] != '=')
    {
        float out = 0;
        uint8_t error = ShuntYard(line + setIndex + 1, strlen(line + setIndex + 1), &out);
        if (error != 0)
        {
            return error;
        }
        for (int i = 0; i < scriptData->variableCount; i++)
        {
            bool match = true;
            for (int x = 0; x < setIndex; x++)
            {
                if (line[x] != scriptData->data[i].name[x])
                {
                    match = false;
                    break;
                }
            }

            if (match && scriptData->data[i].currentType == TYPE_FLOAT)
            {
                printf("Set value %f to %s\n", out, scriptData->data[i].name);
                scriptData->data[i].data.f = out;
                return 0;
            }
        }
        return SYNTAX_UNKNOWN;
    }
}
*/

#endif