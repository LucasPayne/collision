/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

void closest_points_on_lines(vec3 a, vec3 b, vec3 ap, vec3 bp, vec3 *p1, vec3 *p2)
{
#if 0
    vec3 n = vec3_normalize(vec3_cross(vec3_sub(b, a), vec3_sub(bp, ap)));
    vec3 n1 = vec3_normalize(vec3_cross(vec3_sub(b, a), n));
    vec3 n2 = vec3_normalize(vec3_cross(vec3_sub(bp, ap), n));
#else
    // Wikipedia, https://en.wikipedia.org/wiki/Skew_lines
    vec3 n = vec3_cross(vec3_sub(b, a), vec3_sub(bp, ap));
    vec3 n1 = vec3_cross(vec3_sub(b, a), n);
    vec3 n2 = vec3_cross(vec3_sub(bp, ap), n);
    *p1 = vec3_sub(a, vec3_mul(vec3_sub(b, a), vec3_dot(vec3_sub(a, ap), n2) / vec3_dot(vec3_sub(b, a), n2)));
    *p2 = vec3_sub(ap, vec3_mul(vec3_sub(bp, ap), vec3_dot(vec3_sub(ap, a), n1) / vec3_dot(vec3_sub(bp, ap), n1)));
#endif
}

vec3 a;
vec3 b;
vec3 ap;
vec3 bp;
void new_lines(void)
{
    float r = 50;
    a = new_vec3(frand()*r, frand()*r, frand()*r);
    b = new_vec3(frand()*r, frand()*r, frand()*r);
    ap = new_vec3(frand()*r, frand()*r, frand()*r);
    bp = new_vec3(frand()*r, frand()*r, frand()*r);
}

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_R) {
            new_lines();
        }
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
    new_lines();
    create_key_camera_man(0,0,0,  0,0,0);
}
extern void loop_program(void)
{
    vec3 p1, p2;
    closest_points_on_lines(a, b, ap, bp, &p1, &p2);

    vec3 int_a = vec3_sub(p1, vec3_mul(vec3_sub(p2, p1), 1000));
    vec3 int_b = vec3_add(p2, vec3_mul(vec3_sub(p2, p1), 1000));
    vec3 a_infinity = vec3_sub(a, vec3_mul(vec3_sub(b, a), 1000));
    vec3 b_infinity = vec3_add(b, vec3_mul(vec3_sub(b, a), 1000));
    vec3 ap_infinity = vec3_sub(ap, vec3_mul(vec3_sub(bp, ap), 1000));
    vec3 bp_infinity = vec3_add(bp, vec3_mul(vec3_sub(bp, ap), 1000));

    paint_grid_cv(Canvas3D, vec3_add(p1, vec3_add(vec3_sub(int_a, p1), vec3_sub(a_infinity, p1))),
                            vec3_add(p1, vec3_add(vec3_sub(int_a, p1), vec3_sub(b_infinity, p1))),
                            vec3_add(p1, vec3_add(vec3_sub(int_b, p1), vec3_sub(b_infinity, p1))),
                            vec3_add(p1, vec3_add(vec3_sub(int_b, p1), vec3_sub(a_infinity, p1))), "tr", 5000,280,2.0);
    paint_grid_cv(Canvas3D, vec3_add(p2, vec3_add(vec3_sub(int_a, p2), vec3_sub(ap_infinity, p2))),
                            vec3_add(p2, vec3_add(vec3_sub(int_a, p2), vec3_sub(bp_infinity, p2))),
                            vec3_add(p2, vec3_add(vec3_sub(int_b, p2), vec3_sub(bp_infinity, p2))),
                            vec3_add(p2, vec3_add(vec3_sub(int_b, p2), vec3_sub(ap_infinity, p2))), "tb", 5000,280,2.0);

    paint_line_cv(Canvas3D, int_a, int_b, "tk", 2.0);
    paint_line_cv(Canvas3D, p1,p2, "k", 20);

    paint_line_cv(Canvas3D, a_infinity, b, "g", 3);
    paint_line_cv(Canvas3D, b, b_infinity, "g", 3);
    paint_line_cv(Canvas3D, a, b, "r", 10);
    paint_line_cv(Canvas3D, ap_infinity, bp, "g", 3);
    paint_line_cv(Canvas3D, bp, bp_infinity, "g", 3);
    paint_line_cv(Canvas3D, ap, bp, "r", 10);
}
extern void close_program(void)
{
}
