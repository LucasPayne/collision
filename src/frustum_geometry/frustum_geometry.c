/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

void draw_frustum(Camera *camera)
{
    float n, f, b, t, l, r;
    n = camera->plane_n;
    f = camera->plane_f;
    b = camera->plane_b;
    t = camera->plane_t;
    l = camera->plane_l;
    r = camera->plane_r;

    Transform *transform = get_sibling_aspect(camera, Transform);
    vec3 near_p = vec3_mul(Transform_forward(transform), n);
    vec3 far_p = vec3_mul(Transform_forward(transform), f);

    vec3 near_quad[] = {
        vec3_add(near_p, Transform_relative_direction(transform, new_vec3(-l, t, 0))),
        vec3_add(near_p, Transform_relative_direction(transform, new_vec3(-l, -b, 0))),
        vec3_add(near_p, Transform_relative_direction(transform, new_vec3(r, -b, 0))),
        vec3_add(near_p, Transform_relative_direction(transform, new_vec3(r, t, 0))),
    };
    vec3 far_quad[] = {
        vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(-l, t, 0),  f/n))),
        vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(-l, -b, 0), f/n))),
        vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, -b, 0),  f/n))),
        vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, t, 0),   f/n))),
    };
    for (int i = 0; i < 4; i++) {
        paint_line_cv(near_quad[i], near_quad[(i+1)%4], "r");
    }
    for (int i = 0; i < 4; i++) {
        paint_line_cv(far_quad[i], far_quad[(i+1)%4], "r");
    }
    for (int i = 0; i < 4; i++) {
        paint_line_cv(near_quad[i], far_quad[i], "r");
    }
}
void swap_camera(void)
{
    static int camera_num = 0;
    int i = 0;
    for_aspect(Camera, camera)
        Input *input = get_sibling_aspect(camera, Input);
        Logic *logic = get_sibling_aspect(camera, Logic);
        if (i == camera_num) {
            input->listening = true;
            logic->updating = true;
        }
        else {
            input->listening = false;
            logic->updating = false;
        }
        i ++;
    end_for_aspect()
    camera_num = (camera_num + 1) % i;
}
extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_N) {
        swap_camera();
    }
}
extern void mouse_button_event(int button, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    Camera *camera_1 = get_aspect_type(create_key_camera_man(0,0,0,  0,0,0), Camera);
    camera_1->override_bg_color = true;
    camera_1->bg_color = new_vec4(0.8,1,0.8,1);
    camera_1->bly = 0.25;
    camera_1->try = 0.75;
    camera_1->trx = 0.5;
    Camera *camera_2 = get_aspect_type(create_key_camera_man(0,0,200,  0,0,0), Camera);
    camera_2->override_bg_color = true;
    camera_2->bg_color = new_vec4(1,0.8,1,1);
    camera_2->blx = 0.5;
    camera_2->bly = 0.25;
    camera_2->try = 0.75;

    EntityID e = new_entity(4);
    Transform_set(entity_add_aspect(e, Transform), 0,0,-200, 0,0,0);
    Body *body = entity_add_aspect(e, Body);
    body->scale = 100;
    body->geometry = new_resource_handle(Geometry, "Models/block");
    body->material = Material_create("Materials/texture");
    material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/mario/sand_bricks");
    
    swap_camera();
}
extern void loop_program(void)
{
    for_aspect(Camera, camera)
        draw_frustum(camera);
    end_for_aspect()
}
extern void close_program(void)
{
}
