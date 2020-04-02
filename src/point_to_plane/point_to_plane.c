/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"


extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_T) create();
}
extern void mouse_button_event(int button, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}

vec3 ps[3];
vec3 p;
void create(void)
{
    for (int i = 0; i < 3; i++) ps[i] = rand_vec3(100);
    p = rand_vec3(150);
}
extern void init_program(void)
{
    create();
    create_key_camera_man(0,0,0,  0,0,0);
}
extern void loop_program(void)
{
    paint_triangle_cv(Canvas3D, ps[0],ps[1],ps[2], "tg");
    paint_points_c(Canvas3D, &p, 1, "k", 20);
    vec3 proj = point_to_triangle_plane(ps[0],ps[1],ps[2], p);
    paint_points_c(Canvas3D, &proj, 1, "r", 20);
}
extern void close_program(void)
{

}
