/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

Font *font;

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
    font = resource_data(Font, font_handle);

    EntityID text_entity = new_entity(2);
    Transform_set(entity_add_aspect(text_entity, Transform),  0,0,0,  0,0,0);
    Text_init(entity_add_aspect(text_entity, Text), Text2D, "Fonts/computer_modern", "HELLOWORLD");

    create_key_camera_man(0,0,0,  0,0,0);
}
extern void loop_program(void)
{
#if 0
    ResourceHandle texture_handle;
    Texture *tex = oneoff_resource(Texture, texture_handle); 
    tex->texture_id = font->sdf_glyph_map;
    ResourceHandle material = Material_create("Materials/sdf_text");
    material_set_texture(resource_data(Material, material), "sdf_texture", texture_handle);

    paint2d_sprite_m(0,0,  0.5 + time * 0.2,0.5 + time * 0.2,  material);
#endif
}
extern void close_program(void)
{
}
