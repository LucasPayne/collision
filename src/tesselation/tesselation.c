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

    // getchar();
    // ResourceHandle mat_r = Material_create("Materials/test_tesselation");
    // Material *mat = resource_data(Material, mat_r);
    // getchar();

    glPatchParameteri(GL_PATCH_VERTICES, 4);


    open_scene(g_scenes, "block_on_floor");
    test_directional_light_controlled();
}

extern void loop_program(void)
{
    
}
extern void close_program(void)
{
}
