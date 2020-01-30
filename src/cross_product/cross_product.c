/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

vec3 p;
vec3 u;
vec3 v;
float size = 50;
int mode = 0;

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_M) mode = 0;
        if (key == GLFW_KEY_R) mode = 1;
        if (key == GLFW_KEY_G) mode = 2;
    }
}
extern void cursor_move_event(double x, double y)
{
}

void (*g_camera_input)(int, int, int);
void camera_input(int key, int action, int mods)
{
    if (mode == 0) g_camera_input(key, action, mods);
}
void (*g_camera_logic)(Logic *);
void camera_logic(Logic *logic)
{
    if (mode == 0) g_camera_logic(logic);
}

extern void init_program(void)
{
    EntityID camera_man = create_key_camera_man(0,0,0,  0,0,0);
    // Input *input = get_aspect_type(camera_man, Input);
    // g_camera_input = input->callback.key;
    // input->callback.key = camera_input;
    Logic *logic = get_aspect_type(camera_man, Logic);
    g_camera_logic = logic->update;
    logic->update = camera_logic;

    p = new_vec3(0,0,-100);
    u = new_vec3(1,0,0);
    v = new_vec3(0,1,0);

    test_directional_light_auto();
    open_scene(g_scenes, "floor");
}
extern void loop_program(void)
{
    
    if (mode > 0) {
        vec3 move = vec3_zero();
        int speed = 3;
        if (arrow_key_down(Left)) move.vals[0] -= speed;
        if (arrow_key_down(Right)) move.vals[0] += speed;
        if (arrow_key_down(Down)) move.vals[1] -= speed;
        if (arrow_key_down(Up)) move.vals[1] += speed;
        if (alt_arrow_key_down(Down)) move.vals[2] -= speed;
        if (alt_arrow_key_down(Up)) move.vals[2] += speed;
        if (mode == 1) u = vec3_add(u, vec3_mul(move, dt));
        if (mode == 2) v = vec3_add(v, vec3_mul(move, dt));
    }

    vec3 u_pos = vec3_add(p, vec3_mul(u, size));
    vec3 v_pos = vec3_add(p, vec3_mul(v, size));
    vec3 uplusv_pos = vec3_add(p, vec3_mul(vec3_add(u, v), size));
    paint_line_cv(p, u_pos, "r");
    paint_line_cv(p, v_pos, "g");
    paint_quad_v(p, u_pos, uplusv_pos, v_pos, new_vec4(0,1,1,1));
    vec3 cross = vec3_normalize(vec3_cross(u, v));
    paint_line_cv(p, vec3_add(p, vec3_mul(cross, size)), "b");
}
extern void close_program(void)
{
}
