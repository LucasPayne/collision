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
    open_scene(g_scenes, "block_on_floor");

    create_key_camera_man(0,50,100,  0,0,0);
    test_directional_light_controlled();
    test_directional_light_auto();
    // test_point_light_1();
}
extern void loop_program(void)
{
    for_aspect(DirectionalLight, light)
        Transform *t = get_sibling_aspect(light, Transform);
        vec3 dir = DirectionalLight_direction(light);
        vec3 pos = Transform_position(t);
        paint_line_cv(pos, vec3_add(pos, vec3_mul(dir, 10)), "r");

        vec3 near_plane[] = {
            Transform_relative_position(t, new_vec3(-light->shadow_width/2,-light->shadow_height/2,0)),
            Transform_relative_position(t, new_vec3(light->shadow_width/2,-light->shadow_height/2,0)),
            Transform_relative_position(t, new_vec3(light->shadow_width/2,light->shadow_height/2,0)),
            Transform_relative_position(t, new_vec3(-light->shadow_width/2,light->shadow_height/2,0)),
        };
        vec3 far_plane[] = {
            Transform_relative_position(t, new_vec3(-light->shadow_width/2,-light->shadow_height/2,light->shadow_depth)),
            Transform_relative_position(t, new_vec3(light->shadow_width/2,-light->shadow_height/2,light->shadow_depth)),
            Transform_relative_position(t, new_vec3(light->shadow_width/2,light->shadow_height/2,light->shadow_depth)),
            Transform_relative_position(t, new_vec3(-light->shadow_width/2,light->shadow_height/2,light->shadow_depth)),
        };
        paint_loop_c((float *) near_plane, 4, "y");
        paint_loop_c((float *) far_plane, 4, "y");
        for (int i = 0; i < 4; i++) paint_line_cv(near_plane[i], far_plane[i], "g");
        
    end_for_aspect()



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
