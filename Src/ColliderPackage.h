#ifndef COLLIDERPACKAGE
#define COLLIDERPACKAGE

#include <stdio.h>
#include <stdlib.h>
#include "EngineStructs.h"
#include "TFT.h"

#define COLLIDER_RECT 0
#define COLLIDER_MESH 1

#define STATIC 0
#define DYNAMIC 1

#include "DebugPrint.h"


typedef struct Rect
{
    Vector2 localCenter;
    Vector2 globalCenter;
    Vector2 localScale;
    Vector2 globalScale;

    float angle;
    EngineObject* link;

    bool didCollide;
    bool couldExit;

    int ID;
} Rect;

typedef struct Cell
{
    Vector2 topLeft;
    GeneralList rectsWithin;
} Cell;

GeneralList allColliders;
GeneralList cells;

int setRectID = 0;


#define CELL_SIZE_X 8
#define CELL_SIZE_Y 8

/////////////////////////////////////Helper functions
#pragma region

//////////////////////////////// HELPER FUNCTIONS

float TruncateFloat(float value, int digits)
{
    return floor(value * pow(10, digits)) / pow(10, digits);
}

float ProjectPoint(Vector2 point, float slope) {
    return slope * point.x - point.y;
}

float Min4(float a, float b, float c, float d) {
    return min(min(a, b), min(c, d));
}

float Max4(float a, float b, float c, float d) {
    return max(max(a, b), max(c, d));
}

// rotates a point around a point in degrees
Vector2 RotatePoint(Vector2 point, Vector2 pivot, float angle)
{
    Vector2 output;

    angle *= (3.1415 / 180);

    output.x = pivot.x + (point.x - pivot.x) * cos(angle) - (point.y - pivot.y) * sin(angle);
    output.y = pivot.y + (point.x - pivot.x) * sin(angle) + (point.y - pivot.y) * cos(angle);

    return output;
}

Vector2 PreciseProjectPoint(Vector2 point, float slope, Vector2 lineOffset) {
    Vector2 output;
    output.x = (-slope * lineOffset.x + lineOffset.y - slope * point.x - point.y) / (-2 * slope);
    output.y = slope * (output.x - lineOffset.x) + lineOffset.y;
    return output;
}

//////////////////////////////// CONSTRUCTORS / SETUP


Rect* NewRect(float centerX, float centerY, float scaleX, float scaleY, EngineObject* link)
{
    Rect* output = malloc(sizeof(Rect));
    output->localCenter.x = centerX;
    output->localCenter.y = centerY;
    output->localScale.x = scaleX;
    output->localScale.y = scaleY;

    output->globalCenter.x = 0;
    output->globalCenter.y = 0;
    output->globalScale.x = 0;
    output->globalScale.y = 0;

    output->didCollide = false;


    output->link = link;

    debugPrintf("created rect index: %d\n", setRectID);
    output->ID = setRectID++;
    

    return output;
}

Rect* FindRectFromAll(int ID) {
    GeneralListNode* current = allColliders.firstElement;
    while (current != NULL && ((Rect*)(current->content))->ID != ID) {
        current = current->next;
    }
    if (current == NULL)
        return NULL;
    return (Rect*)current->content;
}

void DeleteRectFromAll(int ID) {
    GeneralListNode* current = allColliders.firstElement;
    int elementIndex = 0;
    while (current != NULL && ((Rect*)(current->content))->ID != ID) {
        current = current->next;
        elementIndex++;
    }
    if (current == NULL)
        return;
    debugPrint("found rect, deleting");
    free(DeleteListElement(&allColliders,elementIndex));
}

Cell* NewCell(float topLeftx, float topLefty)
{
    Cell* output = malloc(sizeof(Cell));
    output->topLeft.x = topLeftx;
    output->topLeft.y = topLefty;
    InitializeList(&output->rectsWithin);
    return output;
}

#pragma endregion

////////////////////////////////////Collision setup
#pragma region

void SetupCollisionPackage()
{
    InitializeList(&cells);
    InitializeList(&allColliders);
}

// Converts a shape into a series of rectangles. Efficient, but CPU heavy
void AddMeshToObject(EngineObject* object)
{
    bool renderedPixels[SPRITE_WIDTH][SPRITE_HEIGHT];

    EngineSprite* sprite = &sprites[GetObjectDataByName(object, "sprite")->data.i];

    for (int x = 0; x < SPRITE_WIDTH; x++)
        for (int y = 0; y < SPRITE_HEIGHT; y++)
            renderedPixels[y][x] = false;

    for (int y = 0; y < SPRITE_HEIGHT; y++)
    {
        for (int x = 0; x < SPRITE_WIDTH; x++)
        {
            if (renderedPixels[y][x])
            {
                continue;
            }

            if (sprite->sprite[x][y] == TRANSPARENT)
            {
                continue;
            }

            uint8_t width = 1;
            while (sprite->sprite[x + width][y] != TRANSPARENT && !renderedPixels[y][x + width] && (width + x < SPRITE_WIDTH))
            {
                width++;
            }

            bool doExtend = true;
            uint8_t height = 1;

            while (doExtend)
            {
                if (y + height >= SPRITE_HEIGHT)
                {
                    break;
                }
                doExtend = true;
                for (int x2 = 0; x2 < width; x2++)
                {
                    if (sprite->sprite[x + x2][y + height] == TRANSPARENT)
                    {
                        doExtend = false;
                        break;
                    }
                }
                if (doExtend)
                {
                    height++;
                }
            }

            for (int y2 = 0; y2 < height; y2++)
            {
                for (int x2 = 0; x2 < width; x2++)
                {
                    renderedPixels[y + y2][x + x2] = true;
                }
            }

            Rect* rect = NewRect((float)x - (float)SPRITE_WIDTH / 2 + (float)width / 2, -(float)y + (float)SPRITE_HEIGHT / 2 - (float)height / 2, width, height, object);
            PushList(&allColliders, rect);

            int* rectID = malloc(sizeof(int));
            *rectID = rect->ID;
            PushList(&object->colliderRects, rectID);


            debugPrintf("rect count: %d\n", allColliders.count);
        }
    }
}

void RecalculateObjectColliders(EngineObject* object)
{
    while (object->colliderRects.count > 0)
    {
        int *ID = (int*)PopList(&object->colliderRects);
        DeleteRectFromAll(*ID);
        free(ID);
    }

    if (GetObjectDataByName(object, "meshType")->data.i == COLLIDER_RECT)
    {
        Rect* rect = NewRect(-SPRITE_WIDTH / 2,
            SPRITE_HEIGHT / 2,
            SPRITE_WIDTH,
            SPRITE_HEIGHT,
            object);

        PushList(&allColliders,
            rect);

        int* IDpointer = malloc(sizeof(int));
        *IDpointer = rect->ID;
        PushList(&object->colliderRects, IDpointer);
    }
    else
    {
        AddMeshToObject(object);
    }
}

void AddColliderToObject(EngineObject* object, uint16_t meshTypeSet, bool calculateColliders, float bounceSet)
{
    object->packages[0] = true;

    EngineVar* center = VarConstructor("colliderCenter", strlen("colliderCenter"), TYPE_VECTOR, true);
    center->data.XY.x = 0;
    center->data.XY.y = 0;
    AddDataToObject(object, center);

    EngineVar* size = VarConstructor("colliderSize", strlen("colliderSize"), TYPE_VECTOR, true);
    size->data.XY.x = 1;
    size->data.XY.y = 1;
    AddDataToObject(object, size);

    EngineVar* meshType = VarConstructor("meshType", strlen("meshType"), TYPE_INT, true);
    meshType->data.i = meshTypeSet;
    AddDataToObject(object, meshType);

    EngineVar* collisionType = VarConstructor("colliderType", strlen("colliderType"), TYPE_INT, true);
    collisionType->data.i = DYNAMIC;
    AddDataToObject(object, collisionType);

    EngineVar* mass = VarConstructor("mass", strlen("mass"), TYPE_INT, true);
    mass->data.i = 1;
    AddDataToObject(object, mass);

    EngineVar* sprite = VarConstructor("meshSprite", strlen("meshSprite"), TYPE_INT, true);
    sprite->data.i = GetObjectDataByName(object, "sprite")->data.i;
    AddDataToObject(object, sprite);

    EngineVar* bounce = VarConstructor("bounce", strlen("bounce"), TYPE_FLOAT, true);
    bounce->data.f = bounceSet;
    AddDataToObject(object, bounce);

    if (calculateColliders)
        RecalculateObjectColliders(object);
}

#pragma endregion

/////////////////////////////////// SAT Collision
#pragma region

//Returns true if they are overlapping
bool SATWithSlope(Rect* rectA, Rect* rectB, float slope, bool isVertical) {

    //first 4 points = rect a, rest = rect b
    float projectedPoints[8];


    Vector2 point;
    Vector2 pivot;

    pivot.x = rectA->globalCenter.x;
    pivot.y = rectA->globalCenter.y;

    for (int p = 0; p < 8; p++) {
        //once the second rect is reached, change pivot
        if (p == 4) {
            pivot.x = rectB->globalCenter.x;
            pivot.y = rectB->globalCenter.y;
        }

        Rect* currentRect;

        if (p < 4) {
            currentRect = rectA;
        }
        else {
            currentRect = rectB;
        }

        point.x = currentRect->globalCenter.x + (p % 2 == 0 ? 1 : -1) * (currentRect->globalScale.x / 2);
        point.y = currentRect->globalCenter.y + (p % 4 < 2 ? 1 : -1) * (currentRect->globalScale.y / 2);

        point = RotatePoint(point, pivot, currentRect->angle);

        if (!isVertical)
            projectedPoints[p] = ProjectPoint(point, slope);
        else
            projectedPoints[p] = point.y;
    }
    for (int i = 0; i < 8; i++) {
        debugPrintf("SAT POINTS: %f\n", projectedPoints[i]);
    }

    float aMin = Min4(projectedPoints[0], projectedPoints[1], projectedPoints[2], projectedPoints[3]);
    float aMax = Max4(projectedPoints[0], projectedPoints[1], projectedPoints[2], projectedPoints[3]);

    float bMin = Min4(projectedPoints[4], projectedPoints[5], projectedPoints[6], projectedPoints[7]);
    float bMax = Max4(projectedPoints[4], projectedPoints[5], projectedPoints[6], projectedPoints[7]);

    if (aMin < bMax &&
        aMax > bMin)
        return true;

    return false;
}

// returns true if the rects are colliding
bool SATRects(Rect* rectA, Rect* rectB)
{
    float slope;
    Vector2 point1;
    Vector2 point2;

    for (int i = 0; i < 4; i++) {
        Rect* currentRect;
        if (i < 2) {
            currentRect = rectA;
        }
        else {
            currentRect = rectB;
        }
        /*
        i  x y
        0: 1 1
        1: 1 0
        2: 0 0
        3: 0 1
        */

        point1.x = currentRect->globalCenter.x + (i % 2 == 0 ? 1 : -1) * (currentRect->globalScale.x / 2);
        point1.y = currentRect->globalCenter.y + (currentRect->globalScale.y / 2);

        point2.x = currentRect->globalCenter.x + 1 * (currentRect->globalScale.x / 2);
        point2.y = currentRect->globalCenter.y + (i % 2 == 1 ? 1 : -1) * (currentRect->globalScale.y / 2);

        point1 = RotatePoint(point1, currentRect->globalCenter, currentRect->angle);
        point2 = RotatePoint(point2, currentRect->globalCenter, currentRect->angle);

        debugPrintf("slope point 1 (%f,%f)\n", point1.x, point1.y);
        debugPrintf("slope point 2 (%f,%f)\n", point2.x, point2.y);


        //If Ys are equal (horizontal) then use vertical line (opposite)
        if (point2.y == point1.y) {
            debugPrint("vertical line");
            if (!SATWithSlope(rectA, rectB, 0, true)) {
                debugPrint("SAT done, result: not collide");
                return false;
            }
        }
        //If Xs are equal (Vertical) then use horizontal line (opposite)
        else if (point2.x == point1.x) {
            debugPrint("horizontal line");
            if (!SATWithSlope(rectA, rectB, 0, false)) {
                debugPrint("SAT done, result: not collide");
                return false;
            }
        }
        // otherwise use reciprocal slope
        else {
            float slope = (point2.y - point1.y) / (point2.x - point1.x);
            debugPrintf("slope: %f\n", slope);
            if (!SATWithSlope(rectA, rectB, slope, false)) {
                debugPrint("SAT done, result: not collide");
                return false;
            }
        }
        debugPrint("collide");
    }
    debugPrint("SAT done, result: collide");
    return true;
}

#pragma endregion

/////////////////////////////////// Space partitioning
#pragma region

Vector2 NearestCell(Vector2 input)
{
    Vector2 out;
    out.x = floor(input.x / CELL_SIZE_X) * CELL_SIZE_X;
    out.y = ceil(input.y / CELL_SIZE_Y) * CELL_SIZE_Y;
    return out;
}

int FindCellFromPosition(Vector2 position)
{
    for (int i = 0; i < cells.count; i++)
    {
        Cell* currentCell = (Cell*)ListGetIndex(&cells, i);
        if (currentCell->topLeft.x == position.x && currentCell->topLeft.y == position.y)
        {
            return i;
        }
    }
    return -1;
}

bool DoesCellContainRect(Cell* cell, Rect* rect)
{
    for (int i = 0; i < cell->rectsWithin.count; i++)
    {
        Rect* currentRect = ListGetIndex(&cell->rectsWithin, i);
        if (currentRect == rect)
        {
            return true;
        }
    }
    return false;
}

void FreeCells() {
    while (cells.count > 0) {
        Cell* cell = PopList(&cells);
        while (cell->rectsWithin.count > 0) {
            PopList(&cell->rectsWithin);
        }
        free(cell);
    }
}

#pragma endregion

////////////////////////////////// Collision Stepper and resolution
#pragma region

void JumpToFunction(ScriptData* scriptData, char* functionName);

void ResolveCollision(Rect* a, Rect* b)
{
    float aForce = 0;
    float bForce = 0;

    float massA = (float)GetObjectDataByName(a->link, "mass")->data.i;
    float massB = (float)GetObjectDataByName(b->link, "mass")->data.i;

    int typeA = GetObjectDataByName(a->link, "colliderType")->data.i;
    int typeB = GetObjectDataByName(b->link, "colliderType")->data.i;

    if (typeA == STATIC)
    {
        aForce = 0;
    }
    else if (typeA == DYNAMIC)
    {
        if (typeB == STATIC)
            aForce = 1;
        else
            aForce = massB / (massA + massB);
    }

    if (typeB == STATIC)
    {
        bForce = 0;
    }
    else if (typeB == DYNAMIC)
    {
        if (typeA == STATIC)
            bForce = 1;
        else
            bForce = massA / (massA + massB);
    }

    debugPrintf("aForce: %f\n", aForce);
    debugPrintf("bForce: %f\n", bForce);

    if (aForce == 0 && bForce == 0)
    {
        return;
    }

    Vector2 aPosition = GetObjectDataByName(a->link, "position")->data.XY;
    Vector2 bPosition = GetObjectDataByName(b->link, "position")->data.XY;

    debugPrintf("a pos: (%f,%f)\n", aPosition.x, aPosition.y);

    debugPrintf("b pos: (%f,%f)\n", bPosition.x, bPosition.y);

    int direction = 0;
    int costs[4];

    /*
    0: top of A to bottom of B
    1: right of A to left of B
    2: bottom of A to top of B
    3: left of A to right of B
    */
    Vector2 aDirection;
    Vector2 bDirection;

    // The perpendicular slope of rectA gives the movement vector
    Vector2 tempPoint;
    tempPoint.x = a->globalCenter.x - a->globalScale.x / 2;
    tempPoint.y = a->globalCenter.y;
    Vector2 aLeftPoint = RotatePoint(tempPoint, a->globalCenter, a->angle);
    tempPoint.x = a->globalCenter.x + a->globalScale.x / 2;
    Vector2 aRightPoint = RotatePoint(tempPoint, a->globalCenter, a->angle);

    aLeftPoint.y = -aLeftPoint.y;
    aRightPoint.y = -aRightPoint.y;


    bool isAVertical = false;
    float slopeA = 0;
    if (aRightPoint.x == aLeftPoint.x) {
        isAVertical = true;
    }
    else {
        slopeA = (aRightPoint.y - aLeftPoint.y) / (aRightPoint.x - aLeftPoint.x);
    }

    debugPrintf(" slope a = %f\n", slopeA);

    tempPoint.x = b->globalCenter.x - b->globalScale.x / 2;
    tempPoint.y = b->globalCenter.y;
    Vector2 bLeftPoint = RotatePoint(tempPoint, b->globalCenter, b->angle);
    tempPoint.x = b->globalCenter.x + b->globalScale.x / 2;
    Vector2 bRightPoint = RotatePoint(tempPoint, b->globalCenter, b->angle);

    bLeftPoint.y = -bLeftPoint.y;
    bRightPoint.y = -bRightPoint.y;

    debugPrintf("b points: (%f, %f) -> (%f, %f)\n", bLeftPoint.x, bLeftPoint.y, bRightPoint.x, bRightPoint.y);

    bool isBVertical = false;
    float slopeB = 0;
    if (bRightPoint.x == bLeftPoint.x) {
        isBVertical = true;
    }
    else {
        slopeB = (bRightPoint.y - bLeftPoint.y) / (bRightPoint.x - bLeftPoint.x);
    }
    debugPrintf(" slope b = %f\n", slopeB);

    float combinedSlope = 0;
    bool isHorizontal = false;
    if (isAVertical && isBVertical) {
        debugPrint("both vertical");
        isHorizontal = true;
        combinedSlope = 0;
    }
    else {
        debugPrint("avging slopes");
        combinedSlope = (slopeA + slopeB) / 2;
    }
    debugPrintf("combined slope = %f\n", combinedSlope);
    float aMultiplier = 1;
    float bMultiplier = 1;
    if (isHorizontal) {
        debugPrint("horizontal movement");

        aMultiplier = a->globalCenter.x < b->globalCenter.x ? -1 : 1;
        bMultiplier = b->globalCenter.x < a->globalCenter.x ? -1 : 1;
    }
    else if (combinedSlope == 0) {
        debugPrint("vertical movement");

        aMultiplier = a->globalCenter.y < b->globalCenter.y ? -1 : 1;
        bMultiplier = b->globalCenter.y < a->globalCenter.y ? -1 : 1;
    }
    else {
        debugPrintf("movement along y = %fx\n", -1 / combinedSlope);

        float aProjected = ProjectPoint(a->globalCenter, -1 / combinedSlope);
        float bProjected = ProjectPoint(b->globalCenter, -1 / combinedSlope);

        debugPrintf("projected relations: a:%f, b:%f\n", aProjected, bProjected);
    }

    /*costs[0] = abs((b->globalCenter.y - b->globalScale.y / 2) - (a->globalCenter.y + a->globalScale.y / 2));
    costs[2] = abs((b->globalCenter.y + b->globalScale.y / 2) - (a->globalCenter.y - a->globalScale.y / 2));

    debugPrint("calculated y costs");

    costs[1] = abs((b->globalCenter.x - b->globalScale.x / 2) - (a->globalCenter.x + a->globalScale.x / 2));
    costs[3] = abs((b->globalCenter.x + b->globalScale.x / 2) - (a->globalCenter.x - a->globalScale.x / 2));

    debugPrint("calculated x costs");*/

    /*int lowestCost = -1;
    for (int i = 0; i < 4; i++)
    {
        debugPrintf("check dir %d: %d, lowest: %d\n", i, costs[i], lowestCost);
        if (lowestCost == -1 || costs[i] < lowestCost)
        {
            direction = i;
            lowestCost = costs[i];
        }
    }

    debugPrintf("Direction: %d\n", direction);

    EngineVar* aPos = GetObjectDataByName(a->link, "position");
    EngineVar* bPos = GetObjectDataByName(b->link, "position");

    switch (direction)
    {
    case 0:
        debugPrintf("0: a-%f, b-%f\n", (float)costs[0] * aForce, -(float)costs[0] * bForce);
        aPos->data.XY.y += (float)costs[0] * aForce;
        bPos->data.XY.y -= (float)costs[0] * bForce;

        GetObjectDataByName(a->link, "velocity")->data.XY.y = 0;
        GetObjectDataByName(b->link, "velocity")->data.XY.y = 0;
        break;
    case 1:
        debugPrintf("1: a-%f, b-%f\n", (float)costs[1] * aForce, -(float)costs[1] * bForce);
        aPos->data.XY.x += (float)costs[1] * aForce;
        bPos->data.XY.x -= (float)costs[1] * bForce;

        GetObjectDataByName(a->link, "velocity")->data.XY.x = 0;
        GetObjectDataByName(b->link, "velocity")->data.XY.x = 0;
        break;
    case 2:
        debugPrintf("2: a-%f, b-%f\n", -(float)costs[2] * aForce, (float)costs[2] * bForce);
        aPos->data.XY.y -= (float)costs[2] * aForce;
        bPos->data.XY.y += (float)costs[2] * bForce;

        GetObjectDataByName(a->link, "velocity")->data.XY.y = 0;
        GetObjectDataByName(b->link, "velocity")->data.XY.y = 0;
        break;
    case 3:
        debugPrintf("3: a-%f, b-%f\n", -(float)costs[3] * aForce, (float)costs[3] * bForce);
        aPos->data.XY.x -= (float)costs[3] * aForce;
        bPos->data.XY.x += (float)costs[3] * bForce;

        GetObjectDataByName(a->link, "velocity")->data.XY.x = 0;
        GetObjectDataByName(b->link, "velocity")->data.XY.x = 0;
        break;
    }

    debugPrint("resolved");*/
}

void PartitionColliders() {
    debugPrint("partition colliders");

    FreeCells();


    for (int i = 0; i < allColliders.count; i++)
    {

        Rect* currentRect = (Rect*)ListGetIndex(&allColliders, i);

        //check if collision exitted, run function

        if (!currentRect->didCollide && currentRect->couldExit) {
            debugPrintf("exit collision");
            currentRect->couldExit = false;
            for (int s = 0; s < currentRect->link->scriptCount; s++) {
                JumpToFunction(currentRect->link->scriptData[s], "CollideExit");
            }
        }

        currentRect->didCollide = false;

        // convert rect data to world

        currentRect->globalCenter.x = (currentRect->localCenter.x) * GetObjectDataByName(currentRect->link, "scale")->data.XY.x * GetObjectDataByName(currentRect->link, "colliderSize")->data.XY.x + GetObjectDataByName(currentRect->link, "position")->data.XY.x;
        currentRect->globalCenter.y = (currentRect->localCenter.y) * GetObjectDataByName(currentRect->link, "scale")->data.XY.y * GetObjectDataByName(currentRect->link, "colliderSize")->data.XY.y + GetObjectDataByName(currentRect->link, "position")->data.XY.y;

        currentRect->globalScale.x = currentRect->localScale.x * GetObjectDataByName(currentRect->link, "scale")->data.XY.x * GetObjectDataByName(currentRect->link, "colliderSize")->data.XY.x;
        currentRect->globalScale.y = currentRect->localScale.y * GetObjectDataByName(currentRect->link, "scale")->data.XY.y * GetObjectDataByName(currentRect->link, "colliderSize")->data.XY.y;

        currentRect->angle = GetObjectDataByName(currentRect->link, "angle")->data.f;

        // Create cells

        Vector2 point;
        Vector2 pivot;

        pivot.x = currentRect->globalCenter.x;
        pivot.y = currentRect->globalCenter.y;

        debugPrintf("rect center: %f,%f\n", currentRect->globalCenter.x, currentRect->globalCenter.y);

        Vector2 topLeft;
        Vector2 bottomRight;

        for (int x = 0; x < 4; x++)
        {
            point.x = currentRect->globalCenter.x + (x % 2 == 0 ? 1 : -1) * (currentRect->globalScale.x / 2);
            point.y = currentRect->globalCenter.y + (x < 2 ? 1 : -1) * (currentRect->globalScale.y / 2);

            debugPrintf("point after rot: (%f,%f)\n", RotatePoint(point, pivot, currentRect->angle).x, RotatePoint(point, pivot, currentRect->angle).y);

            Vector2 cellPos = NearestCell(RotatePoint(point, pivot, currentRect->angle));

            if (x == 0 || cellPos.x < topLeft.x) {
                topLeft.x = cellPos.x;
            }
            if (x == 0 || cellPos.y > topLeft.y) {
                topLeft.y = cellPos.y;
            }
            if (x == 0 || cellPos.x > bottomRight.x) {
                bottomRight.x = cellPos.x;
            }
            if (x == 0 || cellPos.y < bottomRight.y) {
                bottomRight.y = cellPos.y;
            }
        }
        debugPrintf("topLeft: (%f,%f)\n", topLeft.x, topLeft.y);
        debugPrintf("bottomRight: (%f,%f)\n", bottomRight.x, bottomRight.y);

        for (int x = topLeft.x; x <= bottomRight.x; x += CELL_SIZE_X)
        {
            for (int y = bottomRight.y; y <= topLeft.y; y += CELL_SIZE_Y)
            {
                Vector2 cellPos;
                cellPos.x = x;
                cellPos.y = y;

                if (FindCellFromPosition(cellPos) == -1)
                {
                    debugPrintf("new cell: (%f,%f)\n", cellPos.x, cellPos.y);
                    Cell* newCell = NewCell(cellPos.x, cellPos.y);
                    PushList(&cells, newCell);
                    PushList(&newCell->rectsWithin, currentRect);
                }
                else
                {
                    debugPrintf("cell exists!!!: (%f,%f)\n", cellPos.x, cellPos.y);
                    Cell* cell = (Cell*)ListGetIndex(&cells, FindCellFromPosition(cellPos));
                    if (!DoesCellContainRect(cell, currentRect)) {
                        PushList(&cell->rectsWithin, currentRect);
                    }
                }
            }
        }
    }
}

#define COLLIDER_ALL 0
#define COLLIDER_FIRST 1
#define COLLIDER_SECOND 2

void ColliderStep(uint8_t mode)
{



    debugPrint("collider step");


    uint16_t start = 0;
    uint16_t end = 0;
    if (mode == COLLIDER_ALL) {
        end = cells.count;
    }
    if (mode == COLLIDER_FIRST) {
        end = ceil(cells.count / 2);
    }
    if (mode == COLLIDER_SECOND) {
        end = cells.count;
        start = ceil(cells.count / 2);
    }


    for (int cellIndex = start; cellIndex < end; cellIndex++) {
        Cell* cell = ListGetIndex(&cells, cellIndex);
        for (int indexA = 0; indexA < cell->rectsWithin.count; indexA++) {
            for (int indexB = indexA + 1; indexB < cell->rectsWithin.count; indexB++) {
                Rect* rectA = ListGetIndex(&cell->rectsWithin, indexA);
                Rect* rectB = ListGetIndex(&cell->rectsWithin, indexB);
                if (rectA->link == NULL) {
                    debugPrint("RECT A NOT LINKED");
                    while (1)sleep_ms(100);
                }
                else {
                    debugPrintf("rect a name: %s\n", rectA->link->name);
                }
                if (rectB->link == NULL) {
                    debugPrint("RECT B NOT LINKED");
                    while (1)sleep_ms(100);
                }
                else {
                    debugPrintf("rect b name: %s\n", rectB->link->name);
                }
                if (rectA->link == rectB->link)
                    continue;

                if (SATRects(rectA, rectB)) {


                    // Rect A Enter
                    if (!rectA->didCollide) {
                        for (int s = 0; s < rectA->link->scriptCount; s++) {
                            JumpToFunction(rectA->link->scriptData[s], "CollideEnter");
                        }
                    }
                    // Rect A stay
                    rectA->didCollide = true;
                    for (int s = 0; s < rectA->link->scriptCount; s++) {
                        JumpToFunction(rectA->link->scriptData[s], "CollideStay");
                    }


                    // Rect B Enter
                    if (!rectB->didCollide) {
                        for (int s = 0; s < rectB->link->scriptCount; s++) {
                            JumpToFunction(rectB->link->scriptData[s], "CollideEnter");
                        }
                    }
                    // Rect B stay
                    rectB->didCollide = true;
                    for (int s = 0; s < rectB->link->scriptCount; s++) {
                        JumpToFunction(rectB->link->scriptData[s], "CollideStay");
                    }

                    rectA->couldExit = true;
                    rectB->couldExit = true;

                    rectA->link->didCollide = true;
                    rectB->link->didCollide = true;

                    //Function defined in physics package


                    void ResolveCollision(Rect * a, Rect * b);
                    ResolveCollision(rectA, rectB);
                }
                else {
                    // Rect A exit
                    if (rectA->couldExit) {
                        debugPrintf("exit collision");
                        for (int s = 0; s < rectA->link->scriptCount; s++) {
                            JumpToFunction(rectA->link->scriptData[s], "CollideExit");
                        }
                        rectA->couldExit = false;
                    }
                    // Rect B exit
                    if (rectB->couldExit) {
                        debugPrintf("exit collision");
                        for (int s = 0; s < rectB->link->scriptCount; s++) {
                            JumpToFunction(rectB->link->scriptData[s], "CollideExit");
                        }
                        rectB->couldExit = false;
                    }

                }
            }
        }
    }
    debugPrint("fin collision step");

}


#pragma endregion

#endif