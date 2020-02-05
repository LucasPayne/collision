/*--------------------------------------------------------------------------------
    Text aspect
--------------------------------------------------------------------------------*/
#include "Engine.h"

AspectType Text_TYPE_ID;

void Text_render(mat4x4 matrix, Text *text)
{
    Font *font = resource_data(Font, text->font);
    // assuming Text2D
    ResourceHandle texture_handle;
    Texture *tex = oneoff_resource(Texture, texture_handle); 
    tex->texture_id = font->sdf_glyph_map;
    ResourceHandle material = Material_create("Materials/sdf_text");
    material_set_texture(resource_data(Material, material), "sdf_texture", texture_handle);

    set_uniform_mat4x4(Standard3D, mvp_matrix.vals, matrix.vals);

    gm_draw(text->geometry, resource_data(Material, material));
}
/*--------------------------------------------------------------------------------
typedef struct Glyph_s {
    char character;
    float uvs[4]; // uv coordinates on the sdf glyph map of the sdf cell. Top-left, then bottom-right.
    float glyph_uvs[4]; // uv coordinates of the subrectangle containing the actual glyph extents, with no added reach.
    int width;
    int height; // Width and height of the glyph's bounding box, in pixels.
    int bearing_x;
    int bearing_y;
    int advance; // in 1/64ths of pixels, as retrieved with freetype.
} Glyph;
void print_glyph(Glyph *glyph);
--------------------------------------------------------------------------------*/
void Text_bake(Text *text)
{
    gm_free(text->geometry);

    Font *font = resource_data(Font, text->font);
    int len = strlen(text->string);
    gm_triangles(VERTEX_FORMAT_3U);
    float cur_x = 0; // in pixels
    for (int i = 0; i < len; i++) {
        Glyph *glyph = &font->glyphs[text->string[i] - 'A'];
        // All offsets are in pixels.
        float glyph_x_offset = glyph->bearing_x;
        float glyph_y_offset = -(glyph->height - glyph->bearing_y); 
        float quad_x_offset = -glyph->width * (glyph->glyph_uvs[2] - glyph->uvs[0]) * 1.0 / (glyph->glyph_uvs[2] - glyph->glyph_uvs[0]);
        float quad_y_offset = glyph->height * (glyph->glyph_uvs[3] - glyph->uvs[1]) * 1.0 / (glyph->glyph_uvs[3] - glyph->glyph_uvs[1]);
        float x_offset = glyph_x_offset + quad_x_offset;
        float y_offset = glyph_y_offset + quad_y_offset;

        // Add this glyph's geometry.
        attribute_3f(Position, cur_x + x_offset, y_offset, 0);
        attribute_3f(Position, cur_x + x_offset, y_offset - glyph->height, 0);
        attribute_3f(Position, cur_x + x_offset + glyph->width, y_offset - glyph->height, 0);
        attribute_3f(Position, cur_x + x_offset + glyph->width, y_offset, 0);

        // The texture coordinates are just the UVs given in the font.
        // attribute_2f(TexCoord, glyph->uvs[0],glyph->uvs[1]);
        // attribute_2f(TexCoord, glyph->uvs[0],glyph->uvs[3]);
        // attribute_2f(TexCoord, glyph->uvs[2],glyph->uvs[3]);
        // attribute_2f(TexCoord, glyph->uvs[2],glyph->uvs[1]);
        attribute_2f(TexCoord, glyph->glyph_uvs[0],glyph->glyph_uvs[1]);
        attribute_2f(TexCoord, glyph->glyph_uvs[0],glyph->glyph_uvs[3]);
        attribute_2f(TexCoord, glyph->glyph_uvs[2],glyph->glyph_uvs[3]);
        attribute_2f(TexCoord, glyph->glyph_uvs[2],glyph->glyph_uvs[1]);
        gm_index(4*i+0); gm_index(4*i+1); gm_index(4*i+2);
        gm_index(4*i+0); gm_index(4*i+2); gm_index(4*i+3);
        // Advance.
        cur_x += glyph->advance >> 6;
    }
    text->geometry = gm_done();
}

void Text_init(Text *text, TextType type, char *font_path, char *string)
{
    text->type = type;
    text->font = new_resource_handle(Font, font_path);
    Text_set(text, string);
}

void Text_set(Text *text, char *string)
{
    text->string = (char *) malloc(sizeof(char) * (strlen(string) + 1));
    mem_check(text->string);
    strcpy(text->string, string);
    Text_bake(text);
}

