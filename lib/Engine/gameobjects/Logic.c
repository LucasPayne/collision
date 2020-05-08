/*================================================================================
    Logic
================================================================================*/
#include "Engine.h"

AspectType Logic_TYPE_ID;
void Logic_init(Logic *logic, LogicUpdate update)
{  
    logic->update = update;
    logic->updating = true;
}

Logic *___add_logic(EntityID entity, LogicUpdate update_function, size_t data_size)
{
    Logic *logic = add_aspect(entity, Logic);
    logic->updating = true;
    logic->update = update_function;
    logic->data = calloc(1, data_size);
    mem_check(logic->data);
    return logic;
}
#define add_logic(ENTITY_ID,UPDATE_FUNCTION,LOGIC_DATA_STRUCT_NAME)\
    ___add_logic(ENTITY_ID,UPDATE_FUNCTION, sizeof(LOGIC_DATA_STRUCT_NAME))

// "Empty logic" has no data. The aspect just stores the update function.
Logic *add_empty_logic(EntityID entity, LogicUpdate update_function)
{
    Logic *logic = add_aspect(entity, Logic);
    logic->updating = true;
    logic->update = update_function;
    return logic;
}
