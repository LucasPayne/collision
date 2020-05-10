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

void Logic_add_input(Logic *logic, uint8_t input_type, void *callback)
{
    switch (input_type) {
        case INPUT_KEY:
            logic->key_listening = true;
            logic->key_listener = callback;
            break;
        case INPUT_MOUSE_POSITION:
            logic->mouse_position_listening = true;
            logic->mouse_position_listener = callback;
            break;
        case INPUT_MOUSE_MOVE:
            logic->mouse_move_listening = true;
            logic->mouse_move_listener = callback;
            break;
        case INPUT_MOUSE_BUTTON:
            logic->mouse_button_listening = true;
            logic->mouse_button_listener = callback;
            break;
        case INPUT_SCROLL_WHEEL:
            logic->scroll_listening = true;
            logic->scroll_listener = callback;
            break;
        default:
            fprintf(stderr, ERROR_ALERT "Invalid input type given when adding an input listener to a Logic aspect.\n");
            exit(EXIT_FAILURE);
    }
}
