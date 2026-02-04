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
    Vector2 topLeft;
    Vector2 scale;
    struct Rect *previous;
    struct Rect *next;

    EngineObject *link;

    uint16_t ID;
} Rect;

typedef struct RectReferer
{

    Rect *ref;

    struct RectReferer *previous;
    struct RectReferer *next;
} RectReferer;

Vector2 cellSize;

typedef struct Cell
{
    Vector2 topleft;

    RectReferer *rectsWithinCell;
    uint16_t rectCount;

    struct Cell *previous;
    struct Cell *next;
} Cell;

Cell *cells = NULL;
uint16_t cellCount = 0;

Rect *colliderRects = NULL;
uint16_t rectCount = 0;

#define FLOAT_DIGITS 4

/////////////////////////// RECT HELPERS //////////////////////

float TruncateFloat(float value, int digits)
{
    return floor(value * pow(10, digits)) / pow(10, digits);
}

#pragma region
uint16_t NewRect(int x, int y, int w, int h, Rect **rectTail, uint16_t *count)
{
    if ((*rectTail) == NULL)
    {
        (*rectTail) = (Rect *)malloc(sizeof(Rect));

        (*rectTail)->previous = NULL;
        (*rectTail)->next = NULL;
    }
    else
    {
        Rect *newRect = (Rect *)malloc(sizeof(Rect));
        (*rectTail)->next = newRect;
        newRect->previous = (*rectTail);
        newRect->next = NULL;

        (*rectTail) = newRect;
    }

    debugPrintf("new rect: %d,%d: %d,%d ID:%d\n", x, y, w, h, *count);

    (*rectTail)->topLeft.x = x;
    (*rectTail)->topLeft.y = y;
    (*rectTail)->scale.x = w;
    (*rectTail)->scale.y = h;
    (*rectTail)->ID = *count;
    return (*count)++;
}

Rect *GetRectByID(uint16_t ID, Rect *rectTail)
{
    Rect *currentRect = rectTail;
    while (currentRect != NULL)
    {
        if (currentRect->ID == ID)
        {
            return currentRect;
        }
        currentRect = currentRect->previous;
    }
    return NULL;
}

void DeleteRect(uint16_t ID, Rect *rectTail)
{
    Rect *rectToDelete = GetRectByID(ID, rectTail);

    rectToDelete->previous->next = rectToDelete->previous;
    rectToDelete->next->previous = rectToDelete->previous;

    rectToDelete->next = NULL;
    rectToDelete->previous = NULL;
    free(rectToDelete);
}

void ClearAllRects(Rect **rectTail, uint16_t *count)
{
    if (*count == 0)
    {
        return;
    }
    Rect *currentRect = *rectTail;
    while (currentRect->previous != NULL)
    {
        currentRect = currentRect->previous;
        free(currentRect->next);
        currentRect->next = NULL;
    }
    free(currentRect);
    *count = 0;
    *rectTail = NULL;
}
#pragma endregion

/////////////////////////// RECT REFERER HELPERS //////////////////////
#pragma region
uint16_t NewRectReferer(Rect *ref, Cell *cell)
{
    if (cell->rectsWithinCell == NULL)
    {
        cell->rectsWithinCell = (RectReferer *)malloc(sizeof(RectReferer));

        cell->rectsWithinCell->previous = NULL;
        cell->rectsWithinCell->next = NULL;
    }
    else
    {
        RectReferer *newRect = (RectReferer *)malloc(sizeof(RectReferer));
        cell->rectsWithinCell->next = newRect;
        newRect->previous = cell->rectsWithinCell;
        newRect->next = NULL;

        cell->rectsWithinCell = newRect;
    }

    cell->rectsWithinCell->ref = ref;
    debugPrintf("new referer: %d\n", ref->ID);
    return cell->rectCount++;
}

RectReferer *GetRectRefererInCell(int index, Cell *cell)
{
    RectReferer *currentCell = cell->rectsWithinCell;
    for (int i = 0; i < index; i++)
    {
        currentCell = currentCell->previous;
        if (currentCell == NULL)
        {
            return NULL;
        }
    }
    return currentCell;
}

void ClearRectReferersInCell(Cell *cell)
{
    RectReferer *currentCell = cell->rectsWithinCell;
    if (currentCell == NULL)
    {
        cell->rectCount = 0;
        return;
    }
    while (currentCell->previous != NULL)
    {
        currentCell = currentCell->previous;
        free(currentCell->next);
        currentCell->next = NULL;
    }
    free(currentCell);
    cell->rectCount = 0;
    cell->rectsWithinCell = NULL;
}
#pragma endregion

/////////////////////////// CELL HELPERS //////////////////////
#pragma region
uint16_t NewCell(int x, int y, Cell **cellTail)
{
    if ((*cellTail) == NULL)
    {
        (*cellTail) = (Cell *)malloc(sizeof(Cell));

        (*cellTail)->previous = NULL;
        (*cellTail)->next = NULL;
    }
    else
    {
        Cell *newRect = (Cell *)malloc(sizeof(Cell));
        (*cellTail)->next = newRect;
        newRect->previous = (*cellTail);
        newRect->next = NULL;

        (*cellTail) = newRect;
    }

    debugPrintf("new cell: %d,%d\n", x, y);

    (*cellTail)->topleft.x = x;
    (*cellTail)->topleft.y = y;

    (*cellTail)->rectsWithinCell = NULL;
    (*cellTail)->rectCount = 0;

    return cellCount++;
}

Cell *GetCellByPosition(int x, int y, Cell *cellTail)
{
    debugPrint("GETTING CELL BY POS");
    if (cellTail == NULL)
    {
        debugPrint("not found, null");
        return NULL;
    }
    Cell *currentCell = cellTail;
    while (currentCell != NULL)
    {
        float a = TruncateFloat(currentCell->topleft.x, FLOAT_DIGITS);
        float b = TruncateFloat(currentCell->topleft.y, FLOAT_DIGITS);
        // debugPrintf("pos: %f, %f\n", a, b);
        if (a == x && b == y)
        {
            return currentCell;
        }
        currentCell = currentCell->previous;
    }
    debugPrint("not found");
    return NULL;
}

Cell *GetCellByIndex(int index, Cell *cellTail)
{
    Cell *currentCell = cellTail;
    for (int i = 0; i < index; i++)
    {
        currentCell = currentCell->previous;
        if (currentCell == NULL)
        {
            return NULL;
        }
    }
    return currentCell;
}

void ClearAllCells(Cell **cellTail)
{
    Cell *currentCell = *cellTail;
    while (currentCell->previous != NULL)
    {
        currentCell = currentCell->previous;
        free(currentCell->next);
        currentCell->next = NULL;
    }
    free(currentCell);
    cellCount = 0;
    *cellTail = NULL;
}
#pragma endregion

Rect RectToWorld(Rect *rect)
{
    Rect output;
    output.topLeft.x = (rect->topLeft.x) * GetObjectDataByName(rect->link, "scale")->data.XY.x + GetObjectDataByName(rect->link, "position")->data.XY.x;
    output.topLeft.y = (rect->topLeft.y) * GetObjectDataByName(rect->link, "scale")->data.XY.y + GetObjectDataByName(rect->link, "position")->data.XY.y;

    output.scale.x = rect->scale.x * GetObjectDataByName(rect->link, "scale")->data.XY.x;
    output.scale.y = rect->scale.y * GetObjectDataByName(rect->link, "scale")->data.XY.x;

    return output;
}

// Converts a shape into a series of rectangles. Efficient, but CPU heavy
uint8_t SpriteToMesh(EngineSprite *sprite)
{
    uint8_t rectsInMesh = 0;
    bool renderedPixels[SPRITE_WIDTH][SPRITE_HEIGHT];

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

            NewRect(x - SPRITE_WIDTH / 2, -y + SPRITE_HEIGHT / 2, width, height, &colliderRects, &rectCount);
            debugPrintf("rect count: %d\n", rectCount);
            rectsInMesh++;
        }
    }

    return rectsInMesh;
}

void RecalculateObjectColliders(EngineObject *object)
{
    if (object->colliderCount > 0)
    {
        for (int i = 0; i < object->colliderCount; i++)
        {
            DeleteRect(object->colliderBoxes[i], colliderRects);
        }
        free(object->colliderBoxes);
        object->colliderCount = 0;
    }

    if (GetObjectDataByName(object, "meshType")->data.i == COLLIDER_RECT)
    {
        object->colliderBoxes = (uint16_t *)malloc(sizeof(uint16_t));
        object->colliderCount = 1;

        object->colliderBoxes[0] = NewRect(
            -SPRITE_WIDTH / 2,
            SPRITE_HEIGHT / 2,
            SPRITE_WIDTH,
            SPRITE_HEIGHT,
            &colliderRects,
            &rectCount);

        GetRectByID(0, colliderRects)->link = object;
    }
    else
    {
        uint8_t rectsInMesh = SpriteToMesh(&sprites[GetObjectDataByName(object, "sprite")->data.i]);

        uint16_t *mesh = (uint16_t *)malloc(sizeof(uint16_t) * rectsInMesh);
        for (int i = 0; i < rectsInMesh; i++)
        {
            mesh[i] = rectCount - rectsInMesh + i;
            GetRectByID(rectCount - rectsInMesh + i, colliderRects)->link = object;
        }

        object->colliderBoxes = mesh;
        object->colliderCount = rectsInMesh;
    }
}

void AddColliderToObject(EngineObject *object, uint16_t meshTypeSet, bool calculateColliders, float bounceSet)
{
    object->packages[0] = true;

    EngineVar *center = VarConstructor("colliderCenter", strlen("colliderCenter"), TYPE_VECTOR, true);
    center->data.XY.x = 0;
    center->data.XY.y = 0;
    AddDataToObject(object, center);

    EngineVar *size = VarConstructor("colliderSize", strlen("colliderSize"), TYPE_VECTOR, true);
    size->data.XY.x = 1;
    size->data.XY.y = 1;
    AddDataToObject(object, size);

    EngineVar *meshType = VarConstructor("meshType", strlen("meshType"), TYPE_INT, true);
    meshType->data.i = meshTypeSet;
    AddDataToObject(object, meshType);

    EngineVar *collisionType = VarConstructor("colliderType", strlen("colliderType"), TYPE_INT, true);
    collisionType->data.i = DYNAMIC;
    AddDataToObject(object, collisionType);

    EngineVar *mass = VarConstructor("mass", strlen("mass"), TYPE_INT, true);
    mass->data.i = 1;
    AddDataToObject(object, mass);

    EngineVar *sprite = VarConstructor("meshSprite", strlen("meshSprite"), TYPE_INT, true);
    sprite->data.i = GetObjectDataByName(object, "sprite")->data.i;
    AddDataToObject(object, sprite);

    EngineVar *bounce = VarConstructor("bounce", strlen("bounce"), TYPE_FLOAT, true);
    bounce->data.f = bounceSet;
    AddDataToObject(object, bounce);

    if (calculateColliders)
        RecalculateObjectColliders(object);
}

void AverageCellSize()
{
    cellSize.x = 0;
    cellSize.y = 0;
    Rect *currentRect = colliderRects;
    while (currentRect != NULL)
    {
        debugPrintf("added rect (%f,%f)\n", RectToWorld(currentRect).scale.x, RectToWorld(currentRect).scale.y);
        cellSize.x += RectToWorld(currentRect).scale.x;
        cellSize.y += RectToWorld(currentRect).scale.y;
        currentRect = currentRect->previous;
    }
    cellSize.x /= rectCount;
    cellSize.y /= rectCount;

    debugPrintf("cell size averaged: (%f,%f)\n", cellSize.x, cellSize.y);
}

Vector2 NearestCell(int x, int y)
{
    Vector2 out;
    out.x = floor(x / cellSize.x) * cellSize.x;
    out.y = ceil(y / cellSize.y) * cellSize.y;
    return out;
}

void CreateCells()
{
    Rect *currentRect = colliderRects;
    while (currentRect != NULL)
    {

        Rect worldRect = RectToWorld(currentRect);

        Vector2 leftTopCell = NearestCell(worldRect.topLeft.x, worldRect.topLeft.y);
        Vector2 rightTopCell = NearestCell(worldRect.topLeft.x + worldRect.scale.x, worldRect.topLeft.y);
        Vector2 leftBottomCell = NearestCell(worldRect.topLeft.x, worldRect.topLeft.y - worldRect.scale.y);
        Vector2 rightBottomCell = NearestCell(worldRect.topLeft.x + worldRect.scale.x, worldRect.topLeft.y - worldRect.scale.y);

        for (float x = leftBottomCell.x; x <= rightTopCell.x; x += cellSize.x)
        {
            for (float y = leftBottomCell.y; y <= rightTopCell.y; y += cellSize.y)
            {
                debugPrintf("rect point(%f,%f)\n", x, y);

                if (GetCellByPosition(x, y, cells) == NULL)
                {
                    NewCell(x, y, &cells);
                }

                NewRectReferer(currentRect, GetCellByPosition(x, y, cells));
                debugPrint("added rect to cell");
            }
        }

        currentRect = currentRect->previous;
    }
    debugPrint("cells created");
}

bool AABB(Rect *a, Rect *b)
{
    Rect worldA = RectToWorld(a);
    Rect worldB = RectToWorld(b);

    if (worldA.topLeft.x < worldB.topLeft.x + worldB.scale.x &&
        worldA.topLeft.x + worldA.scale.x > worldB.topLeft.x &&
        worldA.topLeft.y > worldB.topLeft.y - worldB.scale.y &&
        worldA.topLeft.y - worldA.scale.y < worldB.topLeft.y)
        return true;
    return false;
}

void ResolveCollision(Rect *a, Rect *b)
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

    /*Vector2 COMA;
    Vector2 count;
    for (int i = 0; i < a->link->colliderCount; i++)
    {
        Rect worldRect = RectToWorld(GetRectByID(a->link->colliderBoxes[i], colliderRects));
        COMA.x += (worldRect.topLeft.x + worldRect.scale.x / 2) * worldRect.scale.x;
        count.x += worldRect.scale.x;

        COMA.y += (worldRect.topLeft.y + worldRect.scale.y / 2) * worldRect.scale.y;
        count.y += worldRect.scale.y;
    }*/

    Rect worldA = RectToWorld(a);
    debugPrintf("a pos: (%f,%f)\n", worldA.topLeft.x, worldA.topLeft.y);

    Rect worldB = RectToWorld(b);
    debugPrintf("b pos: (%f,%f)\n", worldB.topLeft.x, worldB.topLeft.y);

    int direction = 0;
    int costs[4];

    costs[0] = abs((worldB.topLeft.y + worldA.scale.y / 2) - (worldA.topLeft.y - worldA.scale.y / 2));
    costs[2] = abs((worldB.topLeft.y - worldB.scale.y - worldA.scale.y / 2) - (worldA.topLeft.y - worldA.scale.y / 2));

    debugPrint("calculated y costs");

    costs[1] = abs((worldB.topLeft.x + worldB.scale.x + worldA.scale.x / 2) - (worldA.topLeft.x + worldA.scale.x / 2));
    costs[3] = abs((worldB.topLeft.x - worldA.scale.x / 2) - (worldA.topLeft.x + worldA.scale.x / 2));

    debugPrint("calculated x costs");

    int lowestCost = -1;
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

    EngineVar *aPos = GetObjectDataByName(a->link, "position");
    EngineVar *bPos = GetObjectDataByName(b->link, "position");

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

    debugPrint("resolved");
}

bool CheckCell(Cell *cell)
{
    if (cell->rectCount <= 1)
    {
        return false;
    }
    bool isCollision = false;
    for (int a = 0; a < cell->rectCount; a++)
    {
        for (int b = a; b < cell->rectCount; b++)
        {
            debugPrintf("a: %d, b: %d\n", a, b);
            if (a == b)
            {
                continue;
            }

            Rect *rectA = GetRectRefererInCell(a, cell)->ref;
            Rect *rectB = GetRectRefererInCell(b, cell)->ref;

            bool isCollide = AABB(rectA, rectB);
            if (isCollide)
            {
                debugPrintf("hit: %d and %d\n", rectA->ID, rectB->ID);
                ResolveCollision(rectA, rectB);
                isCollision = true;
            }
        }
    }

    debugPrint("cell checked");
    return isCollision;
}

void ColliderStep()
{

    debugPrintf("rect count: %d\n", rectCount);
    if (rectCount == 0)
    {
        return;
    }
    AverageCellSize();

    for (int x = 0; x < 5; x++)
    {
        bool didCollide = false;
        debugPrintf("COLLISION ITERATION %d\n", x);
        CreateCells();

        for (int i = 0; i < cellCount; i++)
        {
            debugPrintf("get cell: %d\n", i);
            Cell *cell = GetCellByIndex(i, cells);
            if (CheckCell(cell))
            {
                didCollide = true;
            }

            ClearRectReferersInCell(cell);
        }
        ClearAllCells(&cells);

        if (!didCollide)
        {
            break;
        }
    }

    cells = NULL;
}

#endif