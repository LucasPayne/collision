/*================================================================================
    Input
================================================================================*/
#include "Engine.h"

AspectType Input_TYPE_ID;
void Input_init(Input *inp, uint8_t input_type, /* generic function pointer (no type safety) */ void *callback, bool listening)
{
    switch (input_type) {
        case INPUT_KEY: break;
        case INPUT_MOUSE_POSITION: break;
        case INPUT_MOUSE_MOVE: break;
        default:
            fprintf(stderr, ERROR_ALERT "Invalid input type given when creating an Input aspect.\n");
            exit(EXIT_FAILURE);
    }
    inp->input_type = input_type;
    inp->callback.key = (KeyListener) callback; // cast to a function, does not matter which type.
    inp->listening = listening;
}

