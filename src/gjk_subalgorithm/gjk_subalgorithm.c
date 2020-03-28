/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

vec3 closest_point_on_line_to_point(vec3 a, vec3 b, vec3 p)
{
    // This is an unlimited line.
    vec3 ab = vec3_sub(b, a);
    vec3 ap = vec3_sub(p, a);
    return vec3_add(a, vec3_mul(ab, vec3_dot(ap, ab) / vec3_dot(ab, ab)));
}

vec3 barycentric_triangle(vec3 a, vec3 b, vec3 c, float wa, float wb, float wc)
{
    return vec3_mul(vec3_add(vec3_mul(a, wa), vec3_add(vec3_mul(b, wb), vec3_mul(c, wc))), 1.0/(wa + wb + wc));
}
bool barycentric_triangle_convex(float wa, float wb, float wc)
{
    // tests whether the weights wa+wb+wc = 1 are a convex combination of the triangle points.
    // Weights _must_ have wa+wb+wc for this to work.
    return 0 <= wa && wa <= 1 && 0 <= wb && wb <= 1 && 0 <= wc && wc <= 1;
}
vec3 closest_point_on_triangle_to_point(vec3 a, vec3 b, vec3 c, vec3 p)
{
    vec3 n = vec3_cross(vec3_sub(b, a), vec3_sub(c, a));
    vec3 ap = vec3_sub(p, a);
    vec3 bp = vec3_sub(p, b);
    vec3 cp = vec3_sub(p, c);
    float wa = vec3_dot(vec3_cross(bp, cp), n);
    float wb = vec3_dot(vec3_cross(cp, ap), n);
    float wc = vec3_dot(vec3_cross(ap, bp), n);
    float winv = 1.0 / (wa + wb + wc);
    wa *= winv; wb *= winv; wc *= winv;
    if (wa < 0) {
       if (vec3_dot(vec3_sub(p, b), vec3_sub(c, b)) < 0) return b;
       if (vec3_dot(vec3_sub(p, c), vec3_sub(b, c)) < 0) return c;
       return closest_point_on_line_to_point(b, c, p);
    }
    if (wb < 0) {
       if (vec3_dot(vec3_sub(p, c), vec3_sub(a, c)) < 0) return c;
       if (vec3_dot(vec3_sub(p, a), vec3_sub(c, a)) < 0) return a;
       return closest_point_on_line_to_point(c, a, p);
    }
    if (wc < 0) {
        if (vec3_dot(vec3_sub(p, a), vec3_sub(b, a)) < 0) return a;
        if (vec3_dot(vec3_sub(p, b), vec3_sub(a, b)) < 0) return b;
        return closest_point_on_line_to_point(a, b, p);
    }
    return barycentric_triangle(a,b,c, wa,wb,wc);
}

vec3 closest_point_on_simplex(int n, vec3 points[], vec3 p)
{
    if (n == 1) return points[0];
    if (n == 2) {
        if (vec3_dot(vec3_sub(p, points[0]), vec3_sub(points[1], points[0])) < 0) {
            return points[0];
        }
        if (vec3_dot(vec3_sub(p, points[1]), vec3_sub(points[0], points[1])) < 0) {
            return points[1];
        }
        return closest_point_on_line_to_point(points[0], points[1], p);
    }
    if (n == 3) {
        return closest_point_on_triangle_to_point(points[0], points[1], points[2], p);
    }
    if (n == 4) {
        vec3 a = points[0]; vec3 b = points[1]; vec3 c = points[2]; vec3 d = points[3];
        vec3 close_points[4];
        close_points[0] = closest_point_on_triangle_to_point(a,b,c, p);
        close_points[1] = closest_point_on_triangle_to_point(a,b,d, p);
        close_points[2] = closest_point_on_triangle_to_point(a,c,d, p);
        close_points[3] = closest_point_on_triangle_to_point(b,c,d, p);
        float mindis = -1;
        int min_index = 0;
        for (int i = 0; i < 4; i++) {
            float dis = vec3_dot(close_points[i], close_points[i]);
            if (mindis < 0 || dis < mindis) {
                mindis = dis; min_index = i;
            }
        }
        return close_points[min_index];
    }
    return vec3_zero(); //bad input
}

int n = 0;
vec3 points[4];
vec3 p;

float r = 50;
void new_point(void)
{
    p = new_vec3(r*frand(), r*frand(), r*frand());
}
void new_simplex(void)
{
    for (int i = 0; i < n; i++) points[i] = new_vec3(r*frand(), r*frand(), r*frand());
    if (n++ == 4) n = 1;
}

int mode = 0;
extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_M) mode = (mode + 1) % 3;
    if (action == GLFW_PRESS && key == GLFW_KEY_R) new_simplex();
    if (action == GLFW_PRESS && key == GLFW_KEY_P) new_point();
}
extern void mouse_button_event(int button, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    new_simplex();
    new_point();
    create_key_camera_man(0,0,0,  0,0,0);
}
extern void loop_program(void)
{
    if (mode == 0) {
        if (arrow_key_down(Down)) p.vals[0] -= 100 * dt;
        if (arrow_key_down(Up)) p.vals[0] += 100 * dt;
    } else if (mode == 1) {
        if (arrow_key_down(Down)) p.vals[1] -= 100 * dt;
        if (arrow_key_down(Up)) p.vals[1] += 100 * dt;
    } else if (mode == 2) {
        if (arrow_key_down(Down)) p.vals[2] -= 100 * dt;
        if (arrow_key_down(Up)) p.vals[2] += 100 * dt;
    }


    vec3 c = closest_point_on_simplex(n, points, p);
    paint_line_cv(Canvas3D, c, p, "r", 10.0);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            if (i == j) continue;
            paint_line_cv(Canvas3D, points[i], points[j], "b", 5.0);
        }
    }
    if (n >= 3) {
        for (int i = 0; i < n-1; i++) {
            paint_triangle_cv(Canvas3D, points[i], points[(i+1)%n], points[(i+2)%n], "tb");
        }
    }
}
extern void close_program(void)
{
}
