/*================================================================================
    Input
================================================================================*/
#include "Engine.h"

AspectType Input_TYPE_ID;

void Input_init(Input *input, uint8_t input_type, /* generic function pointer (no type safety) */ void *callback, bool listening)
{
    switch (input_type) {
        case INPUT_KEY: break;
        case INPUT_MOUSE_POSITION: break;
        case INPUT_MOUSE_MOVE: break;
        case INPUT_MOUSE_BUTTON: break;
        default:
            fprintf(stderr, ERROR_ALERT "Invalid input type given when creating an Input aspect.\n");
            exit(EXIT_FAILURE);
    }
    input->input_type = input_type;
    input->callback.key = (KeyListener) callback; // cast to a function, does not matter which type.
    input->listening = listening;
}

Input *Input_add(EntityID e, uint8_t input_type, void *callback, bool listening)
{
    Input *input = entity_add_aspect(e, Input);
    Input_init(input, input_type, callback, listening);
    return input;
}
