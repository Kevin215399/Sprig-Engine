#ifndef PHYSICS
#define PHYSICS

#include <stdio.h>
#include <stdlib.h>
#include "EngineStructs.h"
#include "TFT.h"
#include "DebugPrint.h"

#define DEFAULT_GRAV 1

//#define DYNAMIC 0
#define KINEMATIC 1

void AddPhysicsToObject(EngineObject* object, int moveType, float gravity)
{
    object->packages[1] = true;

    EngineVar* type = VarConstructor("moveType", strlen("moveType"), TYPE_INT, true);
    type->data.i = moveType;
    AddDataToObject(object, type);

    EngineVar* gravScale = VarConstructor("gravityScale", strlen("gravityScale"), TYPE_FLOAT, true);
    gravScale->data.f = gravity;
    AddDataToObject(object, gravScale);
}

void PhysicsStep(EngineObject* object) {
    if (!object->packages[1])
        return;
    if (object->didCollide)
        return;
    if (GetObjectDataByName(object, "moveType")->data.i == DYNAMIC)
        GetObjectDataByName(object, "velocity")->data.XY.y -= GetObjectDataByName(object, "gravityScale")->data.f;
    
}



void ResolveCollision(EngineObject *a, EngineObject*b)
{
    float aForce = 0;
    float bForce = 0;

    float massA = (float)GetObjectDataByName(a, "mass")->data.i;
    float massB = (float)GetObjectDataByName(b, "mass")->data.i;

    int typeA = GetObjectDataByName(a, "colliderType")->data.i;
    int typeB = GetObjectDataByName(b, "colliderType")->data.i;

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

    Vector2 aPosition = GetObjectDataByName(a, "position")->data.XY;
    Vector2 bPosition = GetObjectDataByName(b, "position")->data.XY;

    debugPrintf("a pos: (%f,%f)\n", aPosition.x, aPosition.y);

    debugPrintf("b pos: (%f,%f)\n", bPosition.x, bPosition.y);

    int direction = 0;
    int costs[4];

    costs[0] = abs((b->topLeft.y + a->scale.y / 2) - (a->topLeft.y - a->scale.y / 2));
    costs[2] = abs((b->topLeft.y - b->scale.y - a->scale.y / 2) - (a->topLeft.y - a->scale.y / 2));

    debugPrint("calculated y costs");

    costs[1] = abs((b->topLeft.x + b->scale.x + a->scale.x / 2) - (a->topLeft.x + a->scale.x / 2));
    costs[3] = abs((b->topLeft.x - a->scale.x / 2) - (a->topLeft.x + a->scale.x / 2));

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

    debugPrint("resolved");
}

#endif