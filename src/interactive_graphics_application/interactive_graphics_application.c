/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
    + scenes
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"
#include "scenes.h"
#include "painting.h"


extern void input_event(int key, int action, int mods)
{
#define CASE(ACTION,KEY)\
    if (action == ( GLFW_ ## ACTION ) && key == ( GLFW_KEY_ ## KEY ))
    CASE(PRESS, O) open_scene(g_data, "Scenes/scene2");
    CASE(PRESS, C) test_spawn_cubes(5);
#undef CASE
}
extern void cursor_move_event(double x, double y)
{
}

extern void init_program(void)
{
    painting_init();
    create_camera_man(0,0,0, 0,0,1);
    
    test_point_light_1();
    test_directional_light_controlled();
    test_floor("Textures/minecraft/dirt");

    open_scene(g_data, "Scenes/scene1");
}

extern void loop_program(void)
{
    // paint_line(new_vec3(0,0,0), new_vec3(50,50,50), "g");
    const float quad_size = 5;
    static vec4 grid[5*5] = { 0 };
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (frand() < 0.05) grid[5*i + j] = new_vec4(frand(),frand(),frand(),1);
            paint_quad_v(new_vec3(quad_size*i,quad_size*j,0),
                         new_vec3(quad_size*(i+1),quad_size*j,0),
                         new_vec3(quad_size*(i+1),quad_size*(j+1),0),
                         new_vec3(quad_size*i,quad_size*(j+1),0),
                         grid[5*i + j]);
        }
    }

    vec3 spiral[100];
    for (int i = 0; i < 100; i++) {
        float theta = 6 * i * 2*M_PI / 100;
        spiral[i] = new_vec3(sin(theta + time), 3 + cos(theta + time), i*0.1);
    }
    paint_chain_c((float *) spiral, 100, "g");

    float quad_loop[] = {
        0,0,0,  5,0,0,  5,5,0,  0,5,0
    };
    paint_loop(quad_loop, 4, 1,0,0,1);

    // Draw helpful coordinte axes.
    paint_line_c(-2,2,-2,   1,2,-2,   "r"); // x
    paint_line_c(-2,2,-2,   -2,5,-2,  "g"); // y
    paint_line_c(-2,2,-2,   -2,2,1,   "b"); // z
}

extern void close_program(void)
{
    printf("i'm out\n");
}
