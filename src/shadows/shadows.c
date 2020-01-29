/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_C) test_spawn_cubes(5);
        if (key == GLFW_KEY_U) print_shader_blocks();
    }
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
    // printf("getting. ...\n");
    // getchar();
    // ResourceHandle mat = Material_create("Materials/test_texture");
    // printf("got\n");
    // paint2d_sprite_m(0.9,0,  0.1,0.1,  mat);
    // destroy_resource_handle(&mat);
}
extern void close_program(void)
{
}
