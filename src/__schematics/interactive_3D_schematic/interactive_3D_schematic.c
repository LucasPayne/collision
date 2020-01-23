/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

extern void input_event(int key, int action, int mods)
{
}
extern void init_program(void)
{
    EntityID camera_man = new_entity(4);
    Transform_set(entity_add_aspect(camera_man, Transform), 0,0,0,0,0,0);
    Camera *camera = entity_add_aspect(camera_man, Camera);
    Camera_init(camera, ASPECT_RATIO, 1, 0.9, 10);
    Logic *logic = entity_add_aspect(camera_man, Logic);
    logic->update = camera_controls;
}
extern void loop_program(void)
{
    
}
extern void close_program(void)
{
}
