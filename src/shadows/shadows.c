/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

extern void mouse_button_event(int button, int action, int mods)
{
}
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

    Camera *camera = get_aspect_type(create_key_camera_man(0,50,100,  0,0,0), Camera);
#if 0
{
    Camera *camera = get_aspect_type(create_key_camera_man(0,50,100,  0,0,0), Camera);
    camera->trx = 0.5;
    camera->try = 0.5;
}
{
    Camera *camera = get_aspect_type(create_key_camera_man(-20,80,30,  0,0,0), Camera);
    camera->blx = 0.5;
    camera->bly = 0.5;
}
#endif

    test_directional_light_controlled();
    // test_directional_light_auto();
    // test_point_light_1();
    for (int i = 0; i < 600; i++) {
        EntityID e = new_entity(4);
        Transform_set(add_aspect(e, Transform), -50+frand()*100,10,-50+frand()*100,  0,2*M_PI*frand(),0);
        Body *body = add_aspect(e, Body);
        body->scale = 5;
        body->geometry = new_resource_handle(Geometry, "Models/quad");
        body->material = Material_create("Materials/textured_phong_shadows");
        material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/grass");
    }

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            EntityID e = new_entity(4);
            Transform_set(add_aspect(e, Transform), 200+i*70,-10,200+j*70, 0,0,0);
            Body *body = add_aspect(e, Body);
            body->scale = 400;
            body->geometry = new_resource_handle(Geometry, "Models/stanford_bunny");
            body->material = Material_create("Materials/textured_phong_shadows");
            material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/marble_tile");
        }
    }

    EntityID text_entity = new_entity(3);
    Transform_set(entity_add_aspect(text_entity, Transform),  0,160,-50,  0,0,0);
    Text_init(entity_add_aspect(text_entity, Text), TextOriented, "Fonts/computer_modern", "Shadow testing arena", 0.5);
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
