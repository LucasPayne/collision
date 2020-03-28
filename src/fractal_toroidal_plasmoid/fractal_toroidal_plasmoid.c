/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

extern void input_event(int key, int action, int mods)
{
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
    int n = 4096;
    for (int i = 0; i < n; i++) {
        float r = 100;
        int coils = 1;
        vec3 pos = new_vec3(0,0,0);
        for (int j = 0; j < 4; j++) {
            vec3 add = new_vec3(r * cos(coils * i * 2*M_PI/n), 0, r * sin(coils * i * 2*M_PI/n));
            
            pos = vec3_add(pos, add);
            r /= 2;
            coils *= 8;
        }
    }
}
extern void close_program(void)
{
}
