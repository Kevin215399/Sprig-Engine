#ifndef COLLIDERPACKAGE
#define COLLIDERPACKAGE

#include <stdio.h>
#include <stdlib.h>
#include "EngineStructs.h"
#include "TFT.h"

#include "pico/multicore.h"
#include "pico/sync.h"

#define COLLIDER_RECT 0
#define COLLIDER_MESH 1

#define STATIC 0
#define DYNAMIC 1

#include "DebugPrint.h"

typedef enum ResolutionType {
    ESTIMATE,
    ACCURATE,
    VERY_ACCURATE
} ResolutionType;

ResolutionType resolveAlgorithm;

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

typedef struct CollisionChecked {
    int ID1;
    int ID2;
} CollisionChecked;

typedef struct CollisionPenetration {
    Vector2 axis;
    float amount;
} CollisionPenetration;

GeneralList resolvedCollisions;

GeneralList allColliders;
GeneralList cells;

critical_section_t  resolveCollisionSection;

int setRectID = 0;


#define CELL_SIZE_X 64
#define CELL_SIZE_Y 64

/////////////////////////////////////Helper functions
#pragma region

//////////////////////////////// HELPER FUNCTIONS

float TruncateFloat(float value, int digits)
{
    return floor(value * pow(10, digits)) / pow(10, digits);
}


// gets the dot product of a point on a line of a slope
/*float ProjectPoint(Vector2 point, float slope, bool vertical) {
    if (vertical) {
        return point.y;
    }
    else {
        return ((1.0 / sqrt(1.0 + pow(slope, 2.0))) * point.x) + ((slope / sqrt(1.0 + pow(slope, 2.0))) * point.y);
    }
}*/

Vector2 Normalize(Vector2 axis) {
    Vector2 out;
    out.x = axis.x / sqrt(pow(axis.x, 2) + pow(axis.y, 2));
    out.y = axis.y / sqrt(pow(axis.x, 2) + pow(axis.y, 2));
    return out;
}

float minF(float a, float b);
float maxF(float a, float b);

float Min4(float a, float b, float c, float d) {
    return minF(minF(a, b), minF(c, d));
}

float Max4(float a, float b, float c, float d) {
    return maxF(maxF(a, b), maxF(c, d));
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

// rotates a point around a point in degrees
Vector2 RotateAroundOrigin(Vector2 point, float angle)
{
    Vector2 output;

    angle *= (3.1415 / 180);

    output.x = (point.x) * cos(angle) - (point.y) * sin(angle);
    output.y = (point.x) * sin(angle) + (point.y) * cos(angle);

    return output;
}

float DotProduct(Vector2 a, Vector2 b) {
    return a.x * b.x + a.y * b.y;
}

Vector2 PreciseProjectPoint(Vector2 point, float projectSlope, float slope, Vector2 lineOffset) {
    Vector2 output;



    output.x = (projectSlope * point.x - slope * lineOffset.x + lineOffset.y - point.y) / (projectSlope - slope);
    output.y = slope * (output.x - lineOffset.x) + lineOffset.y;


    return output;
}
float Distance(Vector2 a, Vector2 b) {
    return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
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
    free(DeleteListElement(&allColliders, elementIndex));
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
    InitializeList(&resolvedCollisions);
    critical_section_init(&resolveCollisionSection);
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
        int* ID = (int*)PopList(&object->colliderRects);
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
bool SATWithSlope(Rect* rectA, Rect* rectB, Vector2 axis, CollisionPenetration* collisionPenetration) {

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

        projectedPoints[p] = DotProduct(point, axis);
    }
#ifdef DEBUG
    for (int i = 0; i < 8; i++) {
        debugPrintf("SAT POINTS [%d]: %f\n", i, projectedPoints[i]);
    }
#endif

    float aMin = Min4(projectedPoints[0], projectedPoints[1], projectedPoints[2], projectedPoints[3]);
    float aMax = Max4(projectedPoints[0], projectedPoints[1], projectedPoints[2], projectedPoints[3]);

    float bMin = Min4(projectedPoints[4], projectedPoints[5], projectedPoints[6], projectedPoints[7]);
    float bMax = Max4(projectedPoints[4], projectedPoints[5], projectedPoints[6], projectedPoints[7]);

    if (aMin < bMax &&
        aMax > bMin) {

        debugPrintf("aMin %f\n", aMin);
        debugPrintf("aMax %f\n", aMax);
        debugPrintf("bMin %f\n", bMin);
        debugPrintf("bMax %f\n", bMax);

        debugPrintf("min %f\n", minF(aMax, bMax));
        debugPrintf("max %f\n", maxF(aMin, bMin));

        float penetration = minF(aMax, bMax) - maxF(aMin, bMin);

        if (collisionPenetration->amount == 0 || penetration < collisionPenetration->amount) {



            collisionPenetration->amount = penetration;

            debugPrintf("new smallest penetration: %f along axis (%f,%f)\n", collisionPenetration->amount, axis.x, axis.y);
            collisionPenetration->axis = axis;
        }

        return true;
    }
    return false;
}

// returns true if the rects are colliding
bool SATRects(Rect* rectA, Rect* rectB, CollisionPenetration* penetration)
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

        Vector2 tangent;
        tangent.x = point2.x - point1.x;
        tangent.y = point2.y - point1.y;

        tangent = Normalize(tangent);

        Vector2 normal = RotateAroundOrigin(tangent, 90);

        if (!SATWithSlope(rectA, rectB, normal, penetration)) {
            debugPrint("SAT done, result: not collide");
            return false;
        }

        debugPrint("checked axis");
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

typedef struct Slope {
    float m;
    bool isVertical;
} Slope;


void ResolveCollision(Rect* a, Rect* b, CollisionPenetration* collisionPenetration)
{
    debugPrint("resolveCollision");
    debug_sleep(100);

    debugPrint("enter crit section");
    critical_section_enter_blocking(&resolveCollisionSection);

    GeneralListNode* currentNode = resolvedCollisions.firstElement;
    while (currentNode != NULL) {
        CollisionChecked* check = ((CollisionChecked*)currentNode->content);

        if (check->ID1 == a->ID && check->ID2 == b->ID)
            goto wasChecked;
        if (check->ID1 == b->ID && check->ID2 == a->ID)
            goto wasChecked;


        currentNode = currentNode->next;
        continue;


    wasChecked:
        debugPrintf("%d + %d was already checked\n", check->ID1, check->ID2);
        debug_sleep(100);
        critical_section_exit(&resolveCollisionSection);
        return;

    }
    CollisionChecked* check = malloc(sizeof(CollisionChecked));
    check->ID1 = a->ID;
    check->ID2 = b->ID;
    PushList(&resolvedCollisions, check);

    debugPrint("exit crit section");
    critical_section_exit(&resolveCollisionSection);

    ///////////////////////////////////////////////////////////////// Force distribution
    debugPrintf("resolving collision between %s and %s\n", a->link->name, b->link->name);
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

    EngineVar* aPosition = GetObjectDataByName(a->link, "position");
    EngineVar* bPosition = GetObjectDataByName(b->link, "position");

    debugPrintf("a pos: (%f,%f)\n", aPosition->data.XY.x, aPosition->data.XY.y);

    debugPrintf("b pos: (%f,%f)\n", bPosition->data.XY.x, bPosition->data.XY.y);

    debugPrintf("move amount: %f, along (%f,%f)\n", collisionPenetration->amount, collisionPenetration->axis.x, collisionPenetration->axis.y);

    float aDot = collisionPenetration->axis.x * a->globalCenter.x + collisionPenetration->axis.y * a->globalCenter.y;
    float bDot = collisionPenetration->axis.x * b->globalCenter.x + collisionPenetration->axis.y * b->globalCenter.y;


    EngineVar* aVel = GetObjectDataByName(a->link, "velocity");
    EngineVar* bVel = GetObjectDataByName(b->link, "velocity");

    float aBounce = GetObjectDataByName(a->link, "bounce")->data.f;
    float bBounce = GetObjectDataByName(b->link, "bounce")->data.f;

    float aMultiplier = 1;
    float bMultiplier = 1;

    if (aDot < bDot) {
        debugPrint("a dot is less, negating");

        aMultiplier = -1;
        bMultiplier = 1;

    }
    else {
        debugPrint("b dot is less, negating");

        aMultiplier = 1;
        bMultiplier = -1;
    }



    aPosition->data.XY.x += aMultiplier * (collisionPenetration->amount * collisionPenetration->axis.x) * aForce;
    aPosition->data.XY.y += aMultiplier * (collisionPenetration->amount * collisionPenetration->axis.y) * aForce;

    aVel->data.XY.x += aMultiplier * collisionPenetration->axis.x * aForce * aBounce;
    aVel->data.XY.y += aMultiplier * collisionPenetration->axis.y * aForce * aBounce;


    bPosition->data.XY.x += bMultiplier * (collisionPenetration->amount * collisionPenetration->axis.x) * bForce;
    bPosition->data.XY.y += bMultiplier * (collisionPenetration->amount * collisionPenetration->axis.y) * bForce;

    bVel->data.XY.x += bMultiplier * collisionPenetration->axis.x * bForce * bBounce;
    bVel->data.XY.y += bMultiplier * collisionPenetration->axis.y * bForce * bBounce;

    /*


    // The slope of the rects avged gives the movement vector

    /////////////////////////////////////////////////////////////////////////calculation for rect A
    Vector2 tempPoint;
    tempPoint.x = a->globalCenter.x - a->globalScale.x / 2;
    tempPoint.y = a->globalCenter.y;
    Vector2 aLeftPoint = RotatePoint(tempPoint, a->globalCenter, a->angle);
    tempPoint.x = a->globalCenter.x + a->globalScale.x / 2;
    Vector2 aRightPoint = RotatePoint(tempPoint, a->globalCenter, a->angle);

    aLeftPoint.y = aLeftPoint.y;
    aRightPoint.y = aRightPoint.y;

    Slope slopeA;
    memset(&slopeA, 0, sizeof(Slope));
    if (aRightPoint.x == aLeftPoint.x) {
        slopeA.isVertical = true;
    }
    else {
        slopeA.m = (aRightPoint.y - aLeftPoint.y) / (aRightPoint.x - aLeftPoint.x);
    }

    debugPrintf(" slope a = %f\n", slopeA.m);

    ////////////////////////////////////////////////////////////////////////////calculation for rect B

    tempPoint.x = b->globalCenter.x - b->globalScale.x / 2;
    tempPoint.y = b->globalCenter.y;
    Vector2 bLeftPoint = RotatePoint(tempPoint, b->globalCenter, b->angle);
    tempPoint.x = b->globalCenter.x + b->globalScale.x / 2;
    Vector2 bRightPoint = RotatePoint(tempPoint, b->globalCenter, b->angle);

    bLeftPoint.y = bLeftPoint.y;
    bRightPoint.y = bRightPoint.y;

    debugPrintf("b points: (%f, %f) -> (%f, %f)\n", bLeftPoint.x, bLeftPoint.y, bRightPoint.x, bRightPoint.y);

    Slope slopeB;
    memset(&slopeB, 0, sizeof(Slope));
    if (bRightPoint.x == bLeftPoint.x) {
        slopeB.isVertical = true;
    }
    else {
        slopeB.m = (bRightPoint.y - bLeftPoint.y) / (bRightPoint.x - bLeftPoint.x);
    }
    debugPrintf(" slope b = %f\n", slopeB.m);

    //////////////////////////////////////////////////////////////////////Combine slope

    Slope combinedSlope;
    memset(&combinedSlope, 0, sizeof(Slope));

    if (slopeA.isVertical && slopeB.isVertical) {
        debugPrint("both vertical");
        combinedSlope.isVertical = true;
        combinedSlope.m = 0;
    }
    else {
        debugPrint("avging slopes");
        combinedSlope.m = (slopeA.m + slopeB.m) / 2;
    }
    debugPrintf("combined slope = %f\n", combinedSlope.m);

    ///////////////////////////////////////////////////////////////////// Get Distance 1
    float distance1 = 0;

    Vector2 projectM;
    float _projectSlope = combinedSlope.m;
    float _slopeB = slopeB.m;
    if (combinedSlope.isVertical) {
        _projectSlope = 10000;
    }
    if (slopeB.isVertical) {
        _slopeB = 10000;
    }
    if (_projectSlope == _slopeB) {
        if ((a->globalCenter.y - _projectSlope * a->globalCenter.x) != (b->globalCenter.y - _slopeB * b->globalCenter.x)) {
            debugPrint("Lines never touch!");
            distance1 = 10000;
        }
        else {
            debugPrint("Lines always touch!");
            distance1 = 0;
        }
        goto distance1Done;
    }
    projectM = PreciseProjectPoint(a->globalCenter, _projectSlope, _slopeB, b->globalCenter);
    debugPrintf("project m = (%f,%f)\n", projectM.x, projectM.y);

    distance1 = Distance(projectM, a->globalCenter);

distance1Done:

    /////////////////////////////////////////////////////////////////////Get distance 2
    float distance2 = 0;

    Vector2 projectPerpM;

    _projectSlope = -1 / combinedSlope.m;
    _slopeB = slopeB.m;
    if (combinedSlope.m == 0) {
        _projectSlope = 10000;
    }
    else if (combinedSlope.isVertical) {
        _projectSlope = 0;
    }
    if (slopeB.isVertical) {
        _slopeB = 10000;
    }
    if (_projectSlope == _slopeB) {
        if ((a->globalCenter.y - _projectSlope * a->globalCenter.x) != (b->globalCenter.y - _slopeB * b->globalCenter.x)) {
            debugPrint("Lines never touch!");
            distance2 = 10000;
        }
        else {
            debugPrint("Lines always touch!");
            distance2 = 0;
        }
        goto distance2Done;
    }
    projectPerpM = PreciseProjectPoint(a->globalCenter, _projectSlope, _slopeB, b->globalCenter);
    debugPrintf("project m = (%f,%f)\n", projectPerpM.x, projectPerpM.y);

    distance2 = Distance(projectPerpM, a->globalCenter);

distance2Done:

    ////////////////////////////////////////////////////////////////////


    debugPrintf("dis1: %f, dis2: %f\n", distance1, distance2);

    EngineVar* aPos = GetObjectDataByName(a->link, "position");
    EngineVar* bPos = GetObjectDataByName(b->link, "position");

    EngineVar* aVel = GetObjectDataByName(a->link, "velocity");
    EngineVar* bVel = GetObjectDataByName(b->link, "velocity");

    float aMultiplier = 1;
    float bMultiplier = 1;


    if (distance1 < distance2) {
        //use normal slope
        debugPrint("dist1 is shorter");

        if (combinedSlope.isVertical) {
            combinedSlope.m = 10000;
        }
    }
    else {
        //use perpendicular slope
        debugPrint("dist2 is shorter");
        if (combinedSlope.isVertical) {
            combinedSlope.m = 0;
        }
        else if (combinedSlope.m == 0) {
            combinedSlope.m = 10000;
        }
        else {
            combinedSlope.m = -1 / combinedSlope.m;
        }
    }

    Slope opposite;
    if (combinedSlope.m != 0)
        opposite.m = -1 / combinedSlope.m;
    else
        opposite.m = 10000;


    aMultiplier = a->globalCenter.y < b->globalCenter.y ? -1 : 1;
    bMultiplier = b->globalCenter.y < a->globalCenter.y ? -1 : 1;



    //first 4 points = rect a, rest = rect b
    float projectedPoints[8];

    Vector2 point;

    for (int p = 0; p < 8; p++) {


        Rect* currentRect;

        if (p < 4) {
            currentRect = a;
        }
        else {
            currentRect = b;
        }

        point.x = currentRect->globalCenter.x + (p % 2 == 0 ? 1 : -1) * (currentRect->globalScale.x / 2);
        point.y = currentRect->globalCenter.y + (p % 4 < 2 ? 1 : -1) * (currentRect->globalScale.y / 2);

        point = RotatePoint(point, currentRect->globalCenter, currentRect->angle);

        Vector2 lineOffset;
        lineOffset.x = 0;
        lineOffset.y = 0;
        projectedPoints[p] = PreciseProjectPoint(point, opposite.m, combinedSlope.m, lineOffset).y;

    }
#ifdef DEBUG
    for (int i = 0; i < 8; i++) {
        debugPrintf("PROJECTED POINTS: %f\n", projectedPoints[i]);
    }
#endif

    float p1Max = Max4(projectedPoints[0], projectedPoints[1], projectedPoints[2], projectedPoints[3]);
    float p2Max = Max4(projectedPoints[4], projectedPoints[5], projectedPoints[6], projectedPoints[7]);

    float p1Min = Min4(projectedPoints[0], projectedPoints[1], projectedPoints[2], projectedPoints[3]);
    float p2Min = Min4(projectedPoints[4], projectedPoints[5], projectedPoints[6], projectedPoints[7]);

    float moveAmount = maxF(p1Min, p2Min) - minF(p1Max, p2Max);

    debugPrintf("collision MOVE amt %f\n", moveAmount);

    aPos->data.XY.x += ((moveAmount - a->globalCenter.y) / combinedSlope.m + a->globalCenter.x) * aForce * aMultiplier;
    aPos->data.XY.y += (moveAmount)*aForce * aMultiplier;

    bPos->data.XY.x += ((moveAmount - b->globalCenter.y) / combinedSlope.m + b->globalCenter.x) * bForce * bMultiplier;
    bPos->data.XY.y += (moveAmount)*bForce * bMultiplier;


    */


    /*Slope movementVector;
    memset(&movementVector, 0, sizeof(Slope));

    if (abs(b->globalCenter.y - a->globalCenter.y) > abs(b->globalCenter.x - a->globalCenter.x)) {
        //Change in y is greater, using more vertical slope
        if (combinedSlope.m == 0 || combinedSlope.isVertical) {
            // if slope or perp slope is vertical, choose vertical
            movementVector.isVertical = true;
        }
        else {
            // choose the slope with the greatest abs value (more vertical)
            if (abs(combinedSlope.m) > abs(-1 / combinedSlope.m)) {
                movementVector.m = combinedSlope.m;
            }
            else {
                movementVector.m = -1 / combinedSlope.m;
            }
        }
    }
    else {
        //Change in x is greater, using lower slope

        if (combinedSlope.m == 0 || combinedSlope.isVertical) {
            // if slope or perp slope is horizontal, choose horizontal
            movementVector.m = 0;
        }
        else {
            // choose the slope with the smallest abs value (more horizontal)
            if (abs(combinedSlope.m) < abs(-1 / combinedSlope.m)) {
                movementVector.m = combinedSlope.m;
            }
            else {
                movementVector.m = -1 / combinedSlope.m;
            }
        }
    }

    float aMultiplier = 1;
    float bMultiplier = 1;

    if (movementVector.m == 0) {
        debugPrint("horizontal movement");

        aMultiplier = a->globalCenter.x < b->globalCenter.x ? -1 : 1;
        bMultiplier = b->globalCenter.x < a->globalCenter.x ? -1 : 1;
    }
    else if (movementVector.isVertical) {
        debugPrint("vertical movement");

        aMultiplier = a->globalCenter.y < b->globalCenter.y ? -1 : 1;
        bMultiplier = b->globalCenter.y < a->globalCenter.y ? -1 : 1;
    }
    else {
        debugPrintf("movement along y = %fx\n", movementVector.m);

        float aProjected = ProjectPoint(a->globalCenter, -1 / combinedSlope);
        float bProjected = ProjectPoint(b->globalCenter, -1 / combinedSlope);

        debugPrintf("projected relations: a:%f, b:%f\n", aProjected, bProjected);
    }*/

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
            for (int s = 0; s < currentRect->link->scriptData.count; s++) {
                JumpToFunction((ScriptData*)ListGetIndex(&currentRect->link->scriptData, s), "CollideExit");
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
        if (cell == NULL)
        {
            debugPrint("WHAT?? why are the cells empty?????");
            return;
        }
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


                CollisionPenetration penetration;
                memset(&penetration, 0, sizeof(penetration));

                if (SATRects(rectA, rectB, &penetration)) {


                    // Rect A Enter
                    if (!rectA->didCollide) {
                        for (int s = 0; s < rectA->link->scriptData.count; s++) {
                            JumpToFunction((ScriptData*)ListGetIndex(&rectA->link->scriptData, s), "CollideEnter");
                        }
                    }
                    // Rect A stay
                    rectA->didCollide = true;
                    for (int s = 0; s < rectA->link->scriptData.count; s++) {
                        JumpToFunction((ScriptData*)ListGetIndex(&rectA->link->scriptData, s), "CollideStay");
                    }


                    // Rect B Enter
                    if (!rectB->didCollide) {
                        for (int s = 0; s < rectB->link->scriptData.count; s++) {
                            JumpToFunction((ScriptData*)ListGetIndex(&rectB->link->scriptData, s), "CollideEnter");
                        }
                    }
                    // Rect B stay
                    rectB->didCollide = true;
                    for (int s = 0; s < rectB->link->scriptData.count; s++) {
                        JumpToFunction((ScriptData*)ListGetIndex(&rectB->link->scriptData, s), "CollideStay");
                    }

                    rectA->couldExit = true;
                    rectB->couldExit = true;

                    rectA->link->didCollide = true;
                    rectB->link->didCollide = true;



                    ResolveCollision(rectA, rectB, &penetration);
                }
                else {
                    // Rect A exit
                    if (rectA->couldExit) {
                        debugPrintf("exit collision");
                        for (int s = 0; s < rectA->link->scriptData.count; s++) {
                            JumpToFunction((ScriptData*)ListGetIndex(&rectA->link->scriptData, s), "CollideExit");
                        }
                        rectA->couldExit = false;
                    }
                    // Rect B exit
                    if (rectB->couldExit) {
                        debugPrintf("exit collision");
                        for (int s = 0; s < rectB->link->scriptData.count; s++) {
                            JumpToFunction((ScriptData*)ListGetIndex(&rectB->link->scriptData, s), "CollideExit");
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