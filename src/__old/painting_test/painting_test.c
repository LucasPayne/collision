/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
    + painting
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

static void camera_controls(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    float rotate_speed = 3;
    if (alt_arrow_key_down(Right)) t->theta_y += rotate_speed * dt;
    if (alt_arrow_key_down(Left)) t->theta_y -= rotate_speed * dt;
    if (alt_arrow_key_down(Up)) t->theta_x -= rotate_speed * dt;
    if (alt_arrow_key_down(Down)) t->theta_x += rotate_speed * dt;
    float speed = 10;
    if (arrow_key_down(Right)) t->x -= speed * dt;
    if (arrow_key_down(Left)) t->x += speed * dt;
    if (arrow_key_down(Up)) t->z += speed * dt;
    if (arrow_key_down(Down)) t->z -= speed * dt;
}

extern void input_event(int key, int action, int mods)
{
}
extern void init_program(void)
{
    painting_init();

    EntityID camera_man = new_entity(4);
    Transform_set(entity_add_aspect(camera_man, Transform), 0,0,0,0,0,0);
    Camera *camera = entity_add_aspect(camera_man, Camera);
    Camera_init(camera, ASPECT_RATIO, 1, 0.9, 10);
    Logic *logic = entity_add_aspect(camera_man, Logic);
    logic->update = camera_controls;
}
extern void loop_program(void)
{
    for_aspect(Camera, camera)
        Transform *camera_transform = get_sibling_aspect(camera, Transform);
        mat4x4 view_matrix = Transform_matrix(camera_transform);
        mat4x4 vp_matrix = camera->projection_matrix;
        right_multiply_mat4x4(&vp_matrix, &view_matrix);
        set_uniform_mat4x4(Standard3D, mvp_matrix.vals, vp_matrix.vals);
        paint_line(0,0,0,  5,0,0,  1,0,0,1);
        paint_line(5,0,0,  5,5,0,  1,0,0,1);
        paint_line(5,5,0,  0,5,0,  1,0,0,1);
        paint_line(0,5,0,  0,0,0,  1,0,0,1);
    end_for_aspect()
}
extern void close_program(void)
{
}
