/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

vec3 origin;
vec3 direction;
void new_ray(void)
{
    origin = new_vec3(frand() * 100 - 50, frand() * 100 - 50, frand() * 100 - 50);
    direction = vec3_normalize(new_vec3(frand() * 50, frand() * 50, frand() * 50));
}

bool ray_sphere_intersection(vec3 origin, vec3 direction, vec3 center, float radius, vec3 *intersection)
{
    float d = vec3_dot(direction, direction);
    vec3 p = vec3_add(origin, vec3_mul(direction, vec3_dot(vec3_sub(center, origin), direction) / (d * d)));
    vec3 pp = vec3_sub(p, center);
    float r_squared = vec3_dot(pp, pp);
    float radius_squared = radius*radius;
    if (r_squared > radius_squared) return false;
    vec3 n = vec3_normalize(direction);
    float h = sqrt(radius_squared - r_squared);
    vec3 inter1 = vec3_sub(p, vec3_mul(n, h));
    if (vec3_dot(direction, vec3_sub(inter1, origin)) >= 0) {
        *intersection = inter1;
        return true;
    }
    vec3 inter2 = vec3_add(p, vec3_mul(n, h));
    if (vec3_dot(direction, vec3_sub(inter2, origin)) >= 0) {
        *intersection = inter2;
        return true;
    }
    return false;
}

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_R) {
            new_ray();
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
    create_key_camera_man(0,0,0,  0,0,0);
}
extern void loop_program(void)
{
    vec3 center = new_vec3(0,0,0);
    float radius = 50;
    
    paint_sphere_cv(Canvas3D, center, radius, "tr");
    vec3 p;
    if (ray_sphere_intersection(origin, direction, center, radius, &p)) {
        paint_points_c(Canvas3D, &p, 1, "k", 25);
    }

    paint_line_cv(Canvas3D, origin, vec3_add(origin, vec3_mul(direction, 1000)), "k", 3.0);
}
extern void close_program(void)
{
}
