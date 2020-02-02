/*================================================================================
    Logic
================================================================================*/
#include "Engine.h"

AspectType Logic_TYPE_ID;
void Logic_init(Logic *logic, LogicUpdate update)
{
    logic->update = update;
}

