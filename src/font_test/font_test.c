/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

Font *font;
EntityID overlay_text;

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_T) {
            get_aspect_type(overlay_text, Text)->scale += 0.001;
        }
        if (key == GLFW_KEY_G) {
            get_aspect_type(overlay_text, Text)->scale -= 0.001;
        }
    }
}
extern void mouse_button_event(int button, int action, int mods)
{
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        vec2 pos = pixel_to_rect(mouse_x,mouse_y,  0,0,  1,1);
        Transform_set(get_aspect_type(overlay_text, Transform), pos.vals[0],pos.vals[1],0,0,0,0);
    }
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    ResourceHandle font_handle = new_resource_handle(Font, "Fonts/computer_modern");
    font = resource_data(Font, font_handle);

    {
        EntityID text_entity = new_entity(3);
        Transform_set(entity_add_aspect(text_entity, Transform),  0,0,-200,  0,0,0);
        Text_init(entity_add_aspect(text_entity, Text), TextOriented, "Fonts/computer_modern", "This is a block", 0.2);
        Body *body = entity_add_aspect(text_entity, Body);
        body->scale = 100;
        body->geometry = new_resource_handle(Geometry, "Models/block");
        body->material = Material_create("Materials/texture");
        material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/mario/sand_bricks");
    }
    {
        EntityID text_entity = new_entity(3);
        Transform_set(entity_add_aspect(text_entity, Transform),  0.3,0.3,0,  0,0,0);
        Text_init(entity_add_aspect(text_entity, Text), Text2D, "Fonts/arial_regular", "This is overlay text", 0.004);
        overlay_text = text_entity;
    }


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
