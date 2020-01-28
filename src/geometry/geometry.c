/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_C) test_mass_objects(50);
    if (action == GLFW_PRESS && key == GLFW_KEY_R) test_spawn_stars(10);
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    painting_init();
    create_camera_man(0,0,0,  0,0,0);

    // test_floor("Textures/archimedes");
    // test_directional_light_controlled();
}
extern void loop_program(void)
{
    // paint_line_c(0,0,0,  50,50,50,  "r");

#if 1
    int cell_h = 6;
    int cell_v = 6;
    for (int i = 0; i < cell_h; i++) {
        for (int j = 0; j < cell_v; j++) {
            paint2d_rect(i/(cell_h*5.0),j/(cell_v*5.0),  1.0/5.0,1.0/5.0,   i*1.0/cell_h,frand(),1-j*1.0/cell_v,1);
        }
    }
#endif

    paint2d_line_c(0,0,  1,1,  "b");
}
extern void close_program(void)
{
}
