#ifndef PHYSICS
#define PHYSICS

#include <stdio.h>
#include <stdlib.h>
#include "EngineStructs.h"
#include "TFT.h"
#include "DebugPrint.h"



#define DEFAULT_GRAV 1



void AddPhysicsToObject(EngineObject* object, float gravity)
{
    object->packages[1] = true;

    EngineVar* gravScale = VarConstructor("gravityScale", strlen("gravityScale"), TYPE_FLOAT, true);
    gravScale->data.f = gravity;
    AddDataToObject(object, gravScale);
}

void PhysicsStep(EngineObject* object) {
    if (!object->packages[1])
        return;
    GetObjectDataByName(object, "velocity")->data.XY.y -= GetObjectDataByName(object, "gravityScale")->data.f / 5;

}





#endif