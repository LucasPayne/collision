/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"


vec3 closest_point_on_line_to_point(vec3 a, vec3 b, vec3 p)
{
    //===If possible, always avoid square roots / normalizations.
    vec3 ab = vec3_sub(b, a);
    vec3 ap = vec3_sub(p, a);
    return vec3_add(a, vec3_mul(ab, vec3_dot(ap, ab) / vec3_dot(ab, ab)));
}

vec3 a;
vec3 b;
vec3 p;
void new_point_and_line(void)
{
    float r = 50;
    a = new_vec3(frand()*r, frand()*r, frand()*r);
    b = new_vec3(frand()*r, frand()*r, frand()*r);
    p = new_vec3(frand()*r, frand()*r, frand()*r);
}

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_R) new_point_and_line();
}
extern void mouse_button_event(int button, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    new_point_and_line();
    create_key_camera_man(0,0,0,  0,0,0);
}
extern void loop_program(void)
{
    vec3 c = closest_point_on_line_to_point(a, b, p);
    paint_line_cv(Canvas3D, vec3_sub(a, vec3_mul(vec3_sub(b, a), 1000)), vec3_add(b, vec3_mul(vec3_sub(b, a), 1000)), "g", 2.0);
    paint_line_cv(Canvas3D, a, vec3_add(a, vec3_mul(vec3_sub(b, a), 0.5)), "b", 5.0);
    paint_line_cv(Canvas3D, vec3_add(a, vec3_mul(vec3_sub(b, a), 0.5)), b, "r", 5.0);
    paint_line_cv(Canvas3D, p, c, "k", 3.0);
}
extern void close_program(void)
{
}
