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
    // t->theta_y += dt;
    Body *b = other_aspect(logic, Body);
    Geometry *g = resource_data(Geometry, b->geometry);

    mat4x4 model_matrix = Transform_matrix(t);
    mat4x4 normal_matrix;
    euler_rotation_matrix4x4f(&normal_matrix, t->theta_x, t->theta_y, t->theta_z);

    vec3 *positions = (vec3 *) g->mesh_data->attribute_data[Position];
    vec3 *normals = (vec3 *) g->mesh_data->attribute_data[Normal];
    vec2 *tex_coords = (vec3 *) g->mesh_data->attribute_data[TexCoord];

    vec3 *tangents = (vec3 *) calloc(1, sizeof(vec3) * g->mesh_data->num_vertices);
    mem_check(tangents);
    vec3 *binormals = (vec3 *) calloc(1, sizeof(vec3) * g->mesh_data->num_vertices);
    mem_check(binormals);

    for (int i = 0; i < g->mesh_data->num_triangles; i++) {
        uint32_t a = g->mesh_data->triangles[3*i + 0];
        uint32_t b = g->mesh_data->triangles[3*i + 1];
        uint32_t c = g->mesh_data->triangles[3*i + 2];
        vec3 p0 = positions[a];
        vec3 p1 = positions[b];
        vec3 p2 = positions[c];
        float u0 = tex_coords[a].vals[0];
        float v0 = tex_coords[a].vals[1];
        float u1 = tex_coords[b].vals[0];
        float v1 = tex_coords[b].vals[1];
        float u2 = tex_coords[c].vals[0];
        float v2 = tex_coords[c].vals[1];
        float dx1 = u1 - u0;
        float dy1 = v1 - v0;
        float dx2 = u2 - u0;
        float dy2 = v2 - v0;
        
        float inverse_det = 1.0 / (dx1 * dy2 - dy1 * dx2);

        vec3 e1 = vec3_sub(p1, p0);
        vec3 e2 = vec3_sub(p2, p0);
        
        vec3 tangent = vec3_mul(vec3_add(vec3_mul(e1, dy2), vec3_mul(e2, -dx2)), inverse_det);
        vec3 binormal = vec3_mul(vec3_add(vec3_mul(e1, -dy1), vec3_mul(e2, dx1)), inverse_det);
        tangents[a]  = vec3_add(tangents[a],  tangent);
        binormals[a] = vec3_add(binormals[a], binormal);
        tangents[b]  = vec3_add(tangents[b],  tangent);
        binormals[b] = vec3_add(binormals[b], binormal);
        tangents[c]  = vec3_add(tangents[c],  tangent);
        binormals[c] = vec3_add(binormals[c], binormal);
    }
    for (int i = 0; i < g->mesh_data->num_vertices; i++) {
        // Gram-Schmidt orthonormalization
        tangents[i] = vec3_sub(tangents[i], vec3_mul(normals[i], vec3_dot(tangents[i], normals[i])));
        tangents[i] = vec3_normalize(tangents[i]);
        binormals[i] = vec3_sub(binormals[i], vec3_mul(normals[i], vec3_dot(binormals[i], normals[i])));
        binormals[i] = vec3_sub(binormals[i], vec3_mul(tangents[i], vec3_dot(binormals[i], tangents[i])));
        binormals[i] = vec3_normalize(binormals[i]);
    }

    for (int i = 0; i < g->mesh_data->num_vertices; i++) {
        vec3 pos = vec3_add(model_pos, vec3_mul(mat4x4_vec3(&normal_matrix, positions[i]), b->scale));
        float length = Body_radius(b) * 0.035;
        vec3 normal = vec3_mul(mat4x4_vec3(&normal_matrix, normals[i]), length);
        vec3 tangent = vec3_mul(mat4x4_vec3(&normal_matrix, tangents[i]), length);
        vec3 binormal = vec3_mul(mat4x4_vec3(&normal_matrix, binormals[i]), length);
        paint_line_cv(Canvas3D, pos, vec3_add(pos, normal), "b", 1);
        paint_line_cv(Canvas3D, pos, vec3_add(pos, tangent), "g", 1);
        paint_line_cv(Canvas3D, pos, vec3_add(pos, binormal), "r", 1);
    }


    VertexFormat vertex_format;
    uint32_t num_vertices;
    void *attribute_data[NUM_ATTRIBUTE_TYPES];
    size_t attribute_data_sizes[NUM_ATTRIBUTE_TYPES];
    uint32_t num_triangles;
    uint32_t *triangles;
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

    int bunny_square_root = 1;
    for (int i = 0; i < bunny_square_root; i++) {
        for (int j = 0; j < bunny_square_root; j++) {
            EntityID e = new_entity(4);
            Transform_set(add_aspect(e, Transform), 200+i*70,-10,200+j*70, 0,0,0);
            Body *body = add_aspect(e, Body);
            //body->scale = 400;
            body->scale = 1870;
            body->geometry = new_resource_handle(Geometry, "Models/stanford_bunny -a");
            body->material = Material_create("Materials/textured_phong_shadows");
            material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/marble_tile");
            Logic_init(add_aspect(e, Logic), bunny_update);
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
