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

    test_directional_light_controlled();
    open_scene(g_scenes, "block_on_floor");

    EntityID e = new_entity(4);
    Transform_set(add_aspect(e, Transform), 0,0,0,  0,0,0);
    Body *body = add_aspect(e, Body);
    body->scale = 100;
    body->geometry = new_resource_handle(Geometry, "Models/quad");
    body->material = Material_create("Materials/cutout_texture");
    material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/grass");
}

extern void loop_program(void)
{
    for (int i = 0; i < 1000; i++) {
        // paint_line_v(Canvas3D,  new_vec3(0,0,0), new_vec3(sin(2*M_PI*i/4000.0) * 10000, cos(2*M_PI*i/4000.0) * 10000, 10000), new_vec4(sin(2*i/4000.0),sin(2*i/4000.0 + 2),0,1),  1);
        paint_line(Canvas3D,  0,0,0, sin(2*M_PI*i/4000.0) * 10000, cos(2*M_PI*i/4000.0) * 10000, 10000, sin(2*i/4000.0),sin(2*i/4000.0 + 2),0,1,  1);
    }
    paint_quad_v(Canvas3D, new_vec3(0,0,0), new_vec3(100,0,0), new_vec3(100,100,0), new_vec3(0,100,0), new_vec4(0,1,0.3,0.5));

}
extern void close_program(void)
{
}
