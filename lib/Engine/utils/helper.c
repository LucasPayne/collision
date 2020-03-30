/*--------------------------------------------------------------------------------
    These are helper functions for a possibly more specific "base", which could
    be separated as an extension of this base. Mouse-controllable camera, etc.
--------------------------------------------------------------------------------*/
#include "Engine.h"

static void camera_controls(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    float speed = 10;
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
    int jump_height = 8;
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE) {
            Transform *t = get_sibling_aspect(input, Transform);
            t->y += jump_height;
        }
        if (key == GLFW_KEY_LEFT_SHIFT) {
            Transform *t = get_sibling_aspect(input, Transform);
            t->y -= jump_height;
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

// Create a camera man with mouse controls.
EntityID create_camera_man(float x, float y, float z, float lookat_x, float lookat_y, float lookat_z)
{
    EntityID camera_man = new_entity(4);
    Transform_set(entity_add_aspect(camera_man, Transform), x,y,z,  0,0,0);//--do lookat
    Camera *camera = entity_add_aspect(camera_man, Camera);
    Camera_init(camera, ASPECT_RATIO, 1, 0.9, 2500);
    Logic_init(entity_add_aspect(camera_man, Logic), camera_controls);
    Input_init(entity_add_aspect(camera_man, Input), INPUT_MOUSE_MOVE, camera_mouse_move, true);
    Input_init(entity_add_aspect(camera_man, Input), INPUT_KEY, camera_key_input, true);
    return camera_man;
}

static void camera_key_controls(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    float speed = 150;
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

// Create a camera man controlled by keys.
EntityID create_key_camera_man(float x, float y, float z, float lookat_x, float lookat_y, float lookat_z)
{
    EntityID camera_man = new_entity(4);
    Transform *t = entity_add_aspect(camera_man, Transform);
    Transform_set(t, x,y,z,  0,0,0);//--do lookat
    t->euler_controlled = true;
    Camera *camera = entity_add_aspect(camera_man, Camera);
// void Camera_init(Camera *camera, float aspect_ratio, float near_half_width, float near, float far)
    //Camera_init(camera, ASPECT_RATIO, 1, 0.9, 1200);
    Camera_init(camera, ASPECT_RATIO, 50, 50, 1200);
    Logic_init(entity_add_aspect(camera_man, Logic), camera_key_controls);
    Input_init(entity_add_aspect(camera_man, Input), INPUT_KEY, camera_key_input, true);
    return camera_man;
}
