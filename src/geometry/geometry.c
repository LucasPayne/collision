/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

void star_update(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    t->x += 4 * sin(time) * dt;
    t->y += 4 * cos(time) * dt;
    t->theta_x -= 1.1 * dt;
    t->theta_y += 1.5 * dt;
    t->theta_z -= 1.3 * dt;

    const int n = 5;
    vec4 colors[] = {
        new_vec4(1,0,0,1),
        new_vec4(1,1,0,1),
        new_vec4(1,1,1,1),
        new_vec4(1,0,1,1),
        new_vec4(0.3,0.5,1,1),
    };
    float speed = 5;
    float a = speed*time - (int) (speed*time);
    
    for (int i = 0; i < n; i++) {

        float mul = (a + i*1.0/n);
        mul -= (int) mul;

        float size = 2;
        float vertices[3 * 10];
        for (int i = 0; i < 10; i++) {
            float outward = i % 2 == 0 ? size : size/2;
            float x = cos(i * 2*M_PI/10);
            float y = sin(i * 2*M_PI/10);
            *((vec3 *) &vertices[3*i]) = vec3_add(Transform_position(t), Transform_relative_direction(t, new_vec3(x*outward*mul, y*outward*mul, 0)));
        }
        paint_loop_v(vertices, 10, colors[i]);
    }
}
void spawn_star(float x, float y, float z, float theta_x, float theta_y, float theta_z)
{
    EntityID entity = new_entity(4);
    Transform_set(entity_add_aspect(entity, Transform), x,y,z,  theta_x,theta_y,theta_z);
    Logic *logic = entity_add_aspect(entity, Logic);
    logic->update = star_update;
}
void spawn_stars(int how_many)
{
    for (int i = 0; i < how_many; i++) spawn_star(frand()*50-25, frand()*50-25,frand()*50-25,  2*M_PI*frand(),2*M_PI*frand(),2*M_PI*frand());
}

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_C) test_mass_objects(50);
    if (action == GLFW_PRESS && key == GLFW_KEY_R) spawn_stars(10);
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    painting_init();
    create_camera_man(0,0,0,  0,0,0);

    test_floor("Textures/archimedes");
    test_directional_light_controlled();
}
extern void loop_program(void)
{
    paint_line_c(0,0,0,   50,50,50,   "r");
}
extern void close_program(void)
{
}
