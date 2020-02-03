/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

#define max_points 1024
static int n = 0;
static float points[max_points * 2];

void add_point(float x, float y)
{
    printf("Adding point at (%.2f, %.2f)\n", x, y);
    points[2*n] = x;
    points[2*n+1] = y;
    n ++;
}

vec2 bezier_point(float t)
{
    float bezier_points[max_points * 2];
    memcpy(bezier_points, points, sizeof(float) * 2 * n);
    for (int i = n - 1; i >= 0; --i) {
        for (int j = 0; j < i; j++) {
            bezier_points[2*j] = bezier_points[2*j] + (bezier_points[2*(j + 1)] - bezier_points[2*j]) * t;
            bezier_points[2*j+1] = bezier_points[2*j+1] + (bezier_points[2*(j + 1)+1] - bezier_points[2*j+1]) * t;
        }
    }
    return new_vec2(bezier_points[0], bezier_points[1]);
}

void draw_bezier_curve(void)
{
    if (n >= 2) {
        const int num_points = 100;

        float curve[num_points * 2];
        for (int i = 0; i < num_points; i++) {
            vec2 p = bezier_point(i * 1.0 / num_points);
            ((vec2 *) curve)[i] = p;
        }
        paint2d_chain_c(curve, num_points, "g");
    }

    float quad_size = 0.02;
    for (int i = 0; i < n; i++) {
        paint2d_rect_c(points[2*i]-quad_size/2, points[2*i+1]-quad_size/2, quad_size,quad_size, "r");
    }
}

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_B) {
        n = 0;
    }
}
extern void cursor_move_event(double x, double y)
{
}
extern void mouse_button_event(int button, int action, int mods)
{
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        vec2 pos = pixel_to_rect(mouse_x,mouse_y,  0,0,  1,1);
        add_point(pos.vals[0], pos.vals[1]);
    }
}
extern void init_program(void)
{
    create_key_camera_man(0,0,0,  0,0,0);
}
extern void loop_program(void)
{
    draw_bezier_curve();
}
extern void close_program(void)
{
}
