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


void bunny_update(Logic *logic)
{
    Transform *t = other_aspect(logic, Transform);
    vec3 model_pos = Transform_position(t);
    t->theta_y += dt;
    Body *b = other_aspect(logic, Body);
    Geometry *g = resource_data(Geometry, b->geometry);

    mat4x4 model_matrix = Transform_matrix(t);
    mat4x4 normal_matrix;
    euler_rotation_mat4x4(&normal_matrix, t->theta_x, t->theta_y, t->theta_z);

    vec3 *positions = (vec3 *) g->mesh_data->attribute_data[Position];
    vec3 *normals = (vec3 *) g->mesh_data->attribute_data[Normal];
    vec2 *tex_coords = (vec2 *) g->mesh_data->attribute_data[TexCoord];
    vec3 *tangents = (vec3 *) g->mesh_data->attribute_data[Tangent];

    for (int i = 0; i < g->mesh_data->num_vertices; i++) {
        vec3 pos = vec3_add(model_pos, vec3_mul(mat4x4_vec3(&normal_matrix, positions[i]), b->scale));
        float length = Body_radius(b) * 0.035;
        vec3 normal = vec3_mul(mat4x4_vec3(&normal_matrix, normals[i]), length);
        vec3 tangent = vec3_mul(mat4x4_vec3(&normal_matrix, tangents[i]), length);
        // vec3 binormal = vec3_mul(mat4x4_vec3(&normal_matrix, binormals[i]), length);
        paint_line_cv(Canvas3D, pos, vec3_add(pos, normal), "b", 1);
        paint_line_cv(Canvas3D, pos, vec3_add(pos, tangent), "g", 1);
        // paint_line_cv(Canvas3D, pos, vec3_add(pos, binormal), "r", 1);
    }
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

    DirectionalLight *light = test_directional_light_controlled();
    Transform_set(other_aspect(light, Transform), 0, 1000, 0, crand(),crand(),crand());
    // test_directional_light_auto();
    // test_point_light_1();
    for (int i = 0; i < 600; i++) {
        EntityID e = new_entity(4);
        Transform_set(add_aspect(e, Transform), -50+frand()*100,10,-50+frand()*100,  0,2*M_PI*frand(),0);
        Body *body = add_aspect(e, Body);
        body->visible = true;
        body->scale = 5;
        body->geometry = new_resource_handle(Geometry, "Models/quad");
        body->material = Material_create("Materials/textured_phong_shadows");
        material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/grass");
    }

    int bunny_square_root = 2;
    for (int i = 0; i < bunny_square_root; i++) {
        for (int j = 0; j < bunny_square_root; j++) {
            EntityID e = new_entity(4);
            float apart = 400;
            Transform_set(add_aspect(e, Transform), 200+i*apart,-10,200+j*apart, 0,0,0);
            Body *body = add_aspect(e, Body);
            //body->scale = 400;
            body->scale = 1870;
            body->visible = true;
            body->geometry = new_resource_handle(Geometry, "Models/stanford_bunny -a");
            body->material = Material_create("Materials/textured_phong_shadows_normal_mapped");
            material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/brick_wall");
            material_set_texture_path(resource_data(Material, body->material), "normal_map", "Textures/brick_wall_normal");
            Logic_init(add_aspect(e, Logic), bunny_update);
        }
    }

#if 0
    EntityID text_entity = new_entity(3);
    Transform_set(entity_add_aspect(text_entity, Transform),  0,160,-50,  0,0,0);
    Text_init(entity_add_aspect(text_entity, Text), TextOriented, "Fonts/computer_modern", "Shadow testing arena", 0.5);
#endif
}
extern void loop_program(void)
{

    // printf("getting. ...\n");
    // getchar();
    // ResourceHandle mat = Material_create("Materials/test_texture");
    // printf("got\n");
    // paint2d_sprite_m(0.9,0,  0.1,0.1,  mat);
    // destroy_resource_handle(&mat);

    // draw lights
    for_aspect(DirectionalLight, light)
        Transform *t = get_sibling_aspect(light, Transform);
        vec3 dir = DirectionalLight_direction(light);
        vec3 up = Transform_up(t);
        vec3 right = Transform_right(t);
        vec3 pos = Transform_position(t);
        Transform_draw_axes(t, light->shadow_height/2.0, 6);

        vec3 box_points[] = {
            Transform_relative_position(t, new_vec3(-light->shadow_width/2,-light->shadow_height/2,0)),
            Transform_relative_position(t, new_vec3(light->shadow_width/2,-light->shadow_height/2,0)),
            Transform_relative_position(t, new_vec3(light->shadow_width/2,light->shadow_height/2,0)),
            Transform_relative_position(t, new_vec3(-light->shadow_width/2,light->shadow_height/2,0)),
            Transform_relative_position(t, new_vec3(-light->shadow_width/2,-light->shadow_height/2,light->shadow_depth)),
            Transform_relative_position(t, new_vec3(light->shadow_width/2,-light->shadow_height/2,light->shadow_depth)),
            Transform_relative_position(t, new_vec3(light->shadow_width/2,light->shadow_height/2,light->shadow_depth)),
            Transform_relative_position(t, new_vec3(-light->shadow_width/2,light->shadow_height/2,light->shadow_depth)),
        };
        paint_box_c(Canvas3D, box_points, "ty");
        
    end_for_aspect()
}
extern void close_program(void)
{
}
