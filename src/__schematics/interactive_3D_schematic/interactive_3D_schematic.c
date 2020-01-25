/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

extern void input_event(int key, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    painting_init();
    create_camera_man(0,0,0,  0,0,0);
}
extern void loop_program(void)
{
    
}
extern void close_program(void)
{
}
