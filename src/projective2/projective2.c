/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

vec3 line_directions[10];

extern void input_event(int key, int action, int mods)
{
}
extern void mouse_button_event(MouseButton button, bool click, float x, float y)
{
}
extern void mouse_position_event(double x, double y)
{
}
extern void mouse_move_event(float x, float y, double dx, double dy)
{
}
extern void init_program(void)
{
    Player_create_default(0,0,0,  0,0);
    for (int i = 0; i < 10; i++) {
        line_directions[i] = vec3_normalize(new_vec3(frand()-0.5, frand()-0.5, frand()-0.5));
    }
}

extern void loop_program(void)
{
    vec3 p = vec3_zero();
    paint_points_c(Canvas3D, &p, 1, "k", 20);
    float radius = 25;
    for (int i = 0; i < 10; i++) {
        paint_line_v(Canvas3D, vec3_mul(line_directions[i], radius), vec3_neg(vec3_mul(line_directions[i], radius)), new_vec4(0,0,0,0.9), 2);
    }

    vec3 origin = new_vec3(50, 0,0);
    for (int i = 0; i < 10; i++) {
        vec3 p = vec3_add(origin, new_vec3(0, X(line_directions[i])*radius, Z(line_directions[i])*radius));
        paint_line_v(Canvas3D, p, vec3_add(p, new_vec3(60,0,0)), new_vec4(0,0,0,0.9), 2);
    }
}
extern void close_program(void)
{
}
