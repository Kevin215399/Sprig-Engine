#ifndef COLLIDERPACKAGE
#define COLLIDERPACKAGE

#include <stdio.h>
#include <stdlib.h>
#include "EngineStructs.h"
#include "TFT.h"

#define COLLIDER_RECT 0
#define COLLIDER_MESH 1

typedef struct Rect
{
    Vector2 center;
    Vector2 scale;
    struct Rect *previous;
    struct Rect *next;

    uint16_t ID;
} Rect;

Rect *rectTail;
uint16_t rectCount = 0;

uint16_t NewRect(int x, int y, int w, int h)
{
    if (rectTail == NULL)
    {
        rectTail = (Rect *)malloc(sizeof(Rect));

        rectTail->previous = NULL;
        rectTail->next = NULL;
    }
    else
    {
        Rect *newRect = (Rect *)malloc(sizeof(Rect));
        rectTail->next = newRect;
        newRect->previous = rectTail;
        newRect->next = NULL;

        rectTail = newRect;
    }

    printf("new rect: %d,%d: %d,%d\n", x, y, w, h);

    rectTail->center.x = x;
    rectTail->center.y = y;
    rectTail->scale.x = w;
    rectTail->scale.y = h;
    rectTail->ID = rectCount;
    return rectCount++;
}

Rect *GetRectByID(uint16_t ID)
{
    Rect *currentRect = rectTail;
    while (rectTail != NULL)
    {
        if (currentRect->ID == ID)
        {
            return currentRect;
        }
        currentRect = currentRect->previous;
    }
    return NULL;
}

void DeleteRect(uint16_t ID)
{
    Rect *rectToDelete = GetRectByID(ID);

    rectToDelete->previous->next = rectToDelete->previous;
    rectToDelete->next->previous = rectToDelete->previous;

    rectToDelete->next = NULL;
    rectToDelete->previous = NULL;
    free(rectToDelete);
}

void ClearAllRects()
{
    Rect *currentRect = rectTail;
    while (rectTail->previous != NULL)
    {
        currentRect = currentRect->previous;
        free(currentRect->next);
        currentRect->next = NULL;
    }
    free(currentRect);
    rectCount = 0;
    rectTail = NULL;
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

            NewRect(x - SPRITE_WIDTH / 2, y - SPRITE_HEIGHT / 2, width, height);
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
            DeleteRect(object->colliderBoxes[i]);
        }
        free(object->colliderBoxes);
        object->colliderCount = 0;
    }

    if (object->objectData[5]->data.i == COLLIDER_RECT)
    {
        object->colliderBoxes = (uint16_t *)malloc(sizeof(uint16_t));
        object->colliderCount = 1;

        object->colliderBoxes[0] = NewRect(
            -SPRITE_WIDTH / 2,
            -SPRITE_HEIGHT / 2,
            SPRITE_WIDTH,
            SPRITE_HEIGHT);
    }
    else
    {
        uint8_t rectsInMesh = SpriteToMesh(&sprites[object->objectData[1]->data.i]);

        uint16_t *mesh = (uint16_t *)malloc(sizeof(uint16_t) * rectsInMesh);
        for (int i = 0; i < rectsInMesh; i++)
        {
            mesh[i] = rectCount - rectsInMesh + i;
        }

        object->colliderBoxes = mesh;
        object->colliderCount = rectsInMesh;
    }
}
void AddColliderToObject(EngineObject *object, uint16_t colliderType, bool calculateColliders)
{
    object->objectDataCount+=COLLIDER_VARS;
    object->packages[0]=true;

    object->objectData[3] = VarConstructor("colliderCenter", strlen("colliderCenter"), TYPE_VECTOR);
    object->objectData[3]->data.XY.x = 0;
    object->objectData[3]->data.XY.y = 0;

    object->objectData[4] = VarConstructor("colliderSize", strlen("colliderSize"), TYPE_VECTOR);
    object->objectData[4]->data.XY.x = 1;
    object->objectData[4]->data.XY.y = 1;

    object->objectData[5] = VarConstructor("colliderType", strlen("colliderType"), TYPE_INT);
    object->objectData[5]->data.i = colliderType;

    object->objectData[6] = VarConstructor("meshSprite", strlen("meshSprite"), TYPE_INT);
    object->objectData[6]->data.i = object->objectData[1]->data.i;

    if (calculateColliders)
        RecalculateObjectColliders(object);
}

#endif