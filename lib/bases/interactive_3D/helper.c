/*--------------------------------------------------------------------------------
    These are helper functions for a possibly more specific "base", which could
    be separated as an extension of this base. Mouse-controllable camera, etc.
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

static void camera_controls(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    float speed = 20;
    float move_x = 0;
    float move_y = 0;
    float move_z = 0;
    if (alt_arrow_key_down(Right)) move_x += speed * dt;
    if (alt_arrow_key_down(Left)) move_x -= speed * dt;
    if (alt_arrow_key_down(Up)) move_z -= speed * dt;
    if (alt_arrow_key_down(Down)) move_z += speed * dt;
    Transform_move_relative(t, new_vec3(move_x, move_y, move_z));
}
static void camera_key_input(Input *input, int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE) {
            Transform *t = get_sibling_aspect(input, Transform);
            t->y += 5;
        }
        if (key == GLFW_KEY_LEFT_SHIFT) {
            Transform *t = get_sibling_aspect(input, Transform);
            t->y -= 5;
        }
    }
}
static void camera_mouse_move(Input *input, double dx, double dy)
{
    Transform *t = get_sibling_aspect(input, Transform);
    t->theta_y += dx * 0.002;
    //---what is pitch yaw and roll?
    vec3 x_rel = Transform_relative_direction(t, new_vec3(1,0,0));
    // printf("%.2f %.2f %.2f\n", x_rel.vals[0], x_rel.vals[1], x_rel.vals[2]);
    float dpitch = dy * -0.002;

    // t->theta_x += dpitch * x_rel.vals[0];
    // t->theta_z += dpitch * x_rel.vals[2];
    t->theta_z += dpitch;

    // const float max_theta_x = 0.8 * M_PI/2;
    // const float min_theta_x = -0.8 * M_PI/2;
    // if (t->theta_x > max_theta_x) t->theta_x = max_theta_x;
    // else if (t->theta_x < min_theta_x) t->theta_x = min_theta_x;
}
void create_camera_man(float x, float y, float z, float lookat_x, float lookat_y, float lookat_z)
{
    EntityID camera_man = new_entity(4);
    Transform_set(entity_add_aspect(camera_man, Transform), x,y,z,  0,0,0);//--do lookat
    Camera *camera = entity_add_aspect(camera_man, Camera);
    Camera_init(camera, ASPECT_RATIO, 1, 0.9, 10);
    Logic *logic = entity_add_aspect(camera_man, Logic);
    logic->update = camera_controls;
    Input_init(entity_add_aspect(camera_man, Input), INPUT_MOUSE_MOVE, camera_mouse_move, true);
    Input_init(entity_add_aspect(camera_man, Input), INPUT_KEY, camera_key_input, true);
}




