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
    create_key_camera_man(0,50,100,  0,0,0);
    test_directional_light_controlled();
    open_scene(g_scenes, "block_on_floor");
}
extern void loop_program(void)
{
}
extern void close_program(void)
{
}
