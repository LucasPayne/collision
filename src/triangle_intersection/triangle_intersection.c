/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

vec3 a;
vec3 b;
vec3 c;
vec3 origin;
vec3 direction;

void new_triangle(void)
{
    a = new_vec3(frand() * 200, frand() * 200, frand() * 200);
    b = new_vec3(frand() * 200, frand() * 200, frand() * 200);
    c = new_vec3(frand() * 200, frand() * 200, frand() * 200);
}
void new_ray(void)
{
    origin = new_vec3(frand() * 100 - 50, frand() * 100 - 50, frand() * 100 - 50);
    direction = vec3_normalize(new_vec3(frand() * 50, frand() * 50, frand() * 50));
}


vec3 barycentric_triangle(vec3 a, vec3 b, vec3 c, float wa, float wb, float wc)
{
    //wa+wb+wc = 1
    return vec3_add(vec3_mul(a, wa), vec3_add(vec3_mul(b, wb), vec3_mul(c, wc)));
}
bool barycentric_triangle_convex(float wa, float wb, float wc)
{
    // tests whether the weights are a convex combination of the triangle points.
    return 0 <= wa && wa <= 1 && 0 <= wb && wb <= 1 && 0 <= wc && wc <= 1;
}

bool ray_triangle_intersection(vec3 origin, vec3 direction, vec3 a, vec3 b, vec3 c, vec3 *intersection)
{
    float wa = vec3_dot(direction, vec3_cross(vec3_sub(b, origin), vec3_sub(c, origin)));
    float wb = vec3_dot(direction, vec3_cross(vec3_sub(c, origin), vec3_sub(a, origin)));
    float wc = vec3_dot(direction, vec3_cross(vec3_sub(a, origin), vec3_sub(b, origin)));
    float winv = 1.0 / (wa + wb + wc);
    wa *= winv;
    wb *= winv;
    wc *= winv;
    if (barycentric_triangle_convex(wa,wb,wc)) {
        vec3 p = barycentric_triangle(a,b,c, wa,wb,wc);
        if (vec3_dot(vec3_sub(p, origin), direction) >= 0) {
            *intersection = p;
            return true;
        }
    }
    return false;
}

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_T) {
            new_triangle();
        }
        if (key == GLFW_KEY_R) {
            new_ray();
        }
        const float move = 10.0;
        if (key == GLFW_KEY_I) {
            a.vals[1] += move;
            b.vals[1] += move;
            c.vals[1] += move;
        }
        if (key == GLFW_KEY_O) {
            a.vals[1] -= move;
            b.vals[1] -= move;
            c.vals[1] -= move;
        }
        const float move2 = 30.0;
        if (key == GLFW_KEY_M) {
            a = vec3_add(a, vec3_mul(direction, move2));
            b = vec3_add(b, vec3_mul(direction, move2));
            c = vec3_add(c, vec3_mul(direction, move2));
        }
        if (key == GLFW_KEY_N) {
            a = vec3_add(a, vec3_mul(direction, -move2));
            b = vec3_add(b, vec3_mul(direction, -move2));
            c = vec3_add(c, vec3_mul(direction, -move2));
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
    new_triangle();
    new_ray();
    create_key_camera_man(0,0,0,  0,0,0);
}
extern void loop_program(void)
{
    paint_line_cv(Canvas3D, origin, vec3_add(origin, vec3_mul(direction, 1000)), "k", 3.0);
    vec3 in;
    if (ray_triangle_intersection(origin,direction,a,b,c, &in)) {
        paint_line_cv(Canvas3D, in, vec3_add(in, vec3_mul(direction, 1000)), "b", 10.0);
    }
    paint_line_cv(Canvas3D, a, b, "r", 4.0);
    paint_line_cv(Canvas3D, b, c, "r", 4.0);
    paint_line_cv(Canvas3D, c, a, "r", 4.0);
    paint_triangle_cv(Canvas3D, a, b, c, "tg");
}
extern void close_program(void)
{
}
