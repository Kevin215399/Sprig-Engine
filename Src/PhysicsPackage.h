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





#endif