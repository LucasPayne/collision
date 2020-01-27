/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_C) test_spawn_cubes(5);
    }
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    painting_init();
    create_key_camera_man(0,20,60,  0,0,0);

    test_floor("Textures/minecraft/stone_bricks");
    EntityID cube = new_entity(4);
    Transform_set(entity_add_aspect(cube, Transform), 0,3,0,  0,0,0);
    Body *body = entity_add_aspect(cube, Body);
    body->scale = 20;
    body->material = Material_create("Materials/textured_phong_shadows");
    material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/minecraft/dirt");
    body->geometry = new_resource_handle(Geometry, "Models/block");

    // test_directional_light_auto();
    test_directional_light_controlled();
}
extern void loop_program(void)
{
    
}
extern void close_program(void)
{
}
