/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

extern void input_event(int key, int action, int mods)
{
}
extern void mouse_button_event(int button, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    create_key_camera_man(0,0,0,  0,0,0);

    EntityID e = new_entity(4);
    Transform_set(add_aspect(e, Transform), 0,0,-200, 0,0,0);
    Body *body = add_aspect(e, Body);
    body->scale = 100;
    body->geometry = new_resource_handle(Geometry, "Models/quad");
    body->material = Material_create("Materials/texture_normal");
}
extern void loop_program(void)
{
    
}
extern void close_program(void)
{
}
