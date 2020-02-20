/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

#define grid_width 5 // intersection points on grid
float control_point_heights[grid_width * grid_width];
#define FORBOTH(I,J) for (int I = 0; I < grid_width; I++) {\
                          for (int J = 0; J < grid_width; J++) {
#define END() }}
#define CPH(I,J) control_point_heights[5*( I ) + ( J )]
#define FOR(I) for (int I = 0; I < grid_width-1; I ++)

float U = 0.5;
float V = 0.5;

const float width = 20;
const float rect_x = 0.1;
const float rect_y = 0.1;
const float rect_w = 0.3;
const float rect_h = 0.3;

int num_bezier_iterations = 0;

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_K) {
            num_bezier_iterations ++;
            if (num_bezier_iterations > grid_width - 1) num_bezier_iterations = grid_width - 1;
        }
        if (key == GLFW_KEY_J) {
            num_bezier_iterations --;
            if (num_bezier_iterations < 0) num_bezier_iterations = 0;
        }
    }
}
extern void mouse_button_event(int button, int action, int mods)
{
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        vec2 pos = pixel_to_rect(mouse_x,mouse_y,  0,0,  1,1);
        if (pos.vals[0] > rect_x && pos.vals[0] < rect_x + rect_w
                && pos.vals[1] > rect_y && pos.vals[1] < rect_y + rect_h) {
            U = (pos.vals[0] - rect_x) / rect_w;
            V = (pos.vals[1] - rect_y) / rect_h;
            printf("(%.2f %.2f)\n", U, V);
        }
    }
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    create_key_camera_man(0,0,0,  0,0,0);
    FORBOTH(i,j)
        CPH(i,j) = 23 * frand();
    END()
}
extern void loop_program(void)
{
    FOR(i) {
        FOR (j) {
            float x = width * i;
            float y = width * j;
            float xp = width * (i + 1);
            float yp = width * (j + 1);
            vec3 a =  new_vec3(x,y,CPH(i,j));
            vec3 b =  new_vec3(xp,y,CPH(i+1,j));
            vec3 c =  new_vec3(xp,yp,CPH(i+1,j+1));
            vec3 d =  new_vec3(x,yp,CPH(i,j+1));
            paint_quad_cv(Canvas3D, a,b,c,d, "tk");
            paint_line_c(Canvas3D, x,y,CPH(i,j), x,yp,CPH(i,j+1), "r", 3);
            paint_line_c(Canvas3D, x,y,CPH(i,j), xp,y,CPH(i+1,j), "r", 3);
            if (i == grid_width - 1)
                paint_line_c(Canvas3D, xp,y,CPH(i+1,j), xp,yp,CPH(i+1,j+1), "r", 6);
            if (j == grid_width - 1)
                paint_line_c(Canvas3D, x,yp,CPH(i,j+1), xp,yp,CPH(i+1,j+1), "r", 6);
        }
    }

    paint2d_rect_bordered(Canvas2D, rect_x,rect_y, rect_w,rect_h,  new_vec4(1,0.86,0.86,1), 6, new_vec4(0,0,0,1),  0);
    float px = rect_x + rect_w * U;
    float py = rect_y + rect_h * V;
    float point_size = 0.01;
    paint2d_rect(Canvas2D, px,py, point_size,point_size, new_vec4(1,0,0,1), 1);

    float heights[grid_width * grid_width] = { 0 };
    for (int pass = 0; pass < grid_width - 1; pass ++) {
        for (int i  = 0; i < pass; i++) {
            for (int j = 0; j < pass; j++) {
                float ha = heights[grid_width*i + j];
                float hb = heights[grid_width*(i + 1) + j];
                float hc = heights[grid_width*i + j + 1];
                float hd = heights[grid_width*(i + 1) + j + 1];

                float he = ha + U * (hb - ha);
                float hf = hc + U * (hd - hc);
                float h_final = he + V * (hf - he);
                heights[grid_width*i + j] = h_final;
            }
        }
    }
    paint_line_c(Canvas3D, width*grid_width*U, width*grid_width*V, heights[0], 0,0,0, "k", 10);

}
extern void close_program(void)
{
}
