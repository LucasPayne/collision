/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

extern void input_event(int key, int action, int mods)
{
}
extern void mouse_button_event(MouseButton button, bool click, float x, float y)
{
}
extern void mouse_position_event(double x, double y)
{
}
extern void mouse_move_event(double dx, double dy)
{
}
extern void init_program(void)
{
    create_key_camera_man(0,0,0,  0,0,0);
}
extern void loop_program(void)
{
    float r = 0.3 * time;
    int n = 200;
    for (int i = 0; i < n; i++) {
        float theta = 2*M_PI * i * 1.0 / 200;
        paint_line_cv(Canvas2D, new_vec3(0.5,0.5,0), new_vec3(0.5 + r*cos(theta), 0.5 + r*sin(theta), 0), "k", time);
    }
}
extern void close_program(void)
{
}
