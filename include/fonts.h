/*================================================================================
    Fonts module
================================================================================*/
#include "resources.h"
#include "rendering.h"
#include "matrix_mathematics.h" // for vec4

// All font character sets are subsets of ASCII.
#define FONT_NUM_CHARACTERS 256

extern ResourceType Font_RTID;
typedef struct /* Resource */ Font_s {
    int character_width;
    int character_height;
    GLuint bitmap_texture;
    vec4 character_coords[FONT_NUM_CHARACTERS]; // (-1, *, *, *): null character coordinates, not part of the character set.
    Geometry character_quads[FONT_NUM_CHARACTERS];
} Font;

void Font_load(void *resource, char *path)
{
    Font font;

    #define load_error(STRING) { fprintf(stderr, ERROR_ALERT "Error loading font: %s\n", ( STRING )); exit(EXIT_FAILURE); }
    #define manifest_error(str) load_error("Font manifest file has missing or malformed " str " entry.\n")
    
    // Load the Font resource manifest.
    DataDictionary *dd = dd_open(g_resource_dictionary, path);
    if (dd == NULL) load_error("Could not open dictionary for font.");

    // Form the bitmap and character coords.

    // Upload the quads for each character. This is so that UV coordinates for each character are stored in vram, and rendering
    // a character does not require an upload of new geometry.
    for (char c = 0; c < FONT_NUM_CHARACTERS; c++) {
        vec4 coords = Font_character_coords(&font, c);
        if (coords.vals[0] == -1) continue;
        gm_triangles(VERTEX_FORMAT_3U);
        attribute_3f(Position, 0,0,0);
        attribute_3f(Position, 0,-1,0);
        attribute_3f(Position, 1,-1,0);
        attribute_3f(Position, 1,0,0);
        attribute_2f(TexCoord, coords.vals[0], coords.vals[1]);
        attribute_2f(TexCoord, coords.vals[0], coords.vals[3]);
        attribute_2f(TexCoord, coords.vals[2], coords.vals[3]);
        attribute_2f(TexCoord, coords.vals[2], coords.vals[1]);
        gm_index(0); gm_index(1); gm_index(2);
        gm_index(0); gm_index(2); gm_index(3);
        Geometry character_quad = gm_done();
        font->character_quads[c] = character_quad;
    }

    // Font is finished loading, copy it over.
    memcpy(resource, &font, sizeof(Font));
}


vec4 Font_character_coords(Font *font, char character)
{
    return font->character_coords[character];
}




