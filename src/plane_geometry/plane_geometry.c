/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

bool lines_intersection(float a1, float a2, float b1, float b2, float ap1, float ap2, float bp1, float bp2, float *outa, float *outb)
{
    vec3 c1 = vec3_cross(new_vec3(a1, a2, 1), new_vec3(b1, b2, 1));
    vec3 c2 = vec3_cross(new_vec3(ap1, ap2, 1), new_vec3(bp1, bp2, 1));
    vec3 c = vec3_cross(c1, c2);
    if (c.vals[2] == 0) return false;
    *outa = c.vals[0] / c.vals[2];
    *outb = c.vals[1] / c.vals[2];
    return true;
} 
int num_points = 0;
float points[8];
void add_point(float p1, float p2)
{
    if (num_points == 4) num_points = 0;
    points[2*num_points] = p1;
    points[2*num_points+1] = p2;
    num_points += 1;
}

extern void input_event(int key, int action, int mods)
{
}
extern void mouse_button_event(int button, int action, int mods)
{
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        vec2 pos = pixel_to_rect(mouse_x,mouse_y,  0,0,  1,1);
        add_point(pos.vals[0], pos.vals[1]);
    }
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
    const float size = 0.01;
    for (int i = 0; i < num_points; i++) {
        paint2d_rect_c(Canvas2D, points[2*i], points[2*i+1], size, size, "r", 0);
    }
    if (num_points >= 4) {
        paint_line_c(Canvas2D, points[4],points[5],0,  points[6],points[7],0,  "g",  10);
        float in1, in2;
        if (lines_intersection(points[0],points[1],points[2],points[3],points[4],points[5],points[6],points[7], &in1, &in2)) {
            paint2d_rect_c(Canvas2D, in1, in2, size, size, "k", 0);
        }
    }
    if (num_points >= 2) paint_line_c(Canvas2D, points[0],points[1],0,  points[2],points[3],0,  "g",  10);
}
extern void close_program(void)
{
}
