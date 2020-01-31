/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

static void camera_update(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    float speed = 100;
    float move_x = 0, move_z = 0;
    if (alt_arrow_key_down(Right)) move_x += speed * dt;
    if (alt_arrow_key_down(Left)) move_x -= speed * dt;
    if (alt_arrow_key_down(Up)) move_z -= speed * dt;
    if (alt_arrow_key_down(Down)) move_z += speed * dt;
    Transform_move_relative(t, new_vec3(move_x, 0, move_z));

    float look_speed = 4;
    if (arrow_key_down(Left)) t->theta_y -= look_speed * dt;
    if (arrow_key_down(Right)) t->theta_y += look_speed * dt;
}
static void camera_key_input(Input *input, int key, int action, int mods)
{
    Transform *t = get_sibling_aspect(input, Transform);
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE) t->y += 5;
        if (key == GLFW_KEY_LEFT_SHIFT) t->y -= 5;
    }
}

extern void input_event(int key, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    open_scene(g_scenes, "block_on_floor");

    EntityID camera_man = new_entity(4);
    Transform_set(entity_add_aspect(camera_man, Transform), -70,50,70,  0,M_PI/4,0);
    Camera *camera = entity_add_aspect(camera_man, Camera);
    Camera_init(camera, ASPECT_RATIO, 1, 0.9, 10);
    Logic *logic = entity_add_aspect(camera_man, Logic);
    logic->update = camera_update;
    Input_init(entity_add_aspect(camera_man, Input), INPUT_KEY, camera_key_input, true);

    EntityID light = new_entity(4);
    Transform_set(entity_add_aspect(light, Transform), 0,100,20,  M_PI/2+0.4,0,0);
    DirectionalLight_init(entity_add_aspect(light, DirectionalLight),  1,1,1,1,  200,200,400);
}
extern void loop_program(void)
{
    
}
extern void close_program(void)
{
}
