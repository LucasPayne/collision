/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

#define num_points 10
float gradients[num_points];
const float max_gradient = 3.0;

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

    for (int i = 0; i < num_points; i++) {
        gradients[i] = frand() * 3 - 1;
    }

}
extern void loop_program(void)
{
    for (int i = 0; i < num_points; i++) {
        const float border = 0.13;
        const float y = 0.13;
        const float width = 0.032;
        float x = border + (1 - 2 * border) * i * 1.0 / num_points;
        paint_line_c(Canvas2D, x-width/2, y-width * gradients[i]/max_gradient, 0, x+width/2, y+width*gradients[i]/max_gradient, 0, "g", 3);
    }
    for (int i = 0; i < num_points; i++) {
        for (int j = 0; j < num_points; j++) {
            
        }
    }
}
extern void close_program(void)
{
}
