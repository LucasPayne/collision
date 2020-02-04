/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

ResourceHandle texture_handle;

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
    ResourceHandle font_handle = new_resource_handle(Font, "Fonts/computer_modern");
    Font *font = resource_data(Font, font_handle);

    Texture *tex = oneoff_resource(Texture, texture_handle); 
    tex->texture_id = font->sdf_glyph_map;

    create_key_camera_man(0,0,0,  0,0,0);
}
extern void loop_program(void)
{
    paint2d_sprite(0,0,  0.5,0.5,  texture_handle);
}
extern void close_program(void)
{
}
