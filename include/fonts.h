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
    #define load_error(STRING) { fprintf(stderr, ERROR_ALERT "Error loading font: %s\n", ( STRING )); exit(EXIT_FAILURE); }
    #define manifest_error(str) load_error("Font manifest file has missing or malformed " str " entry.\n")
    Font *font = (Font *) resource;
    //---read manifest, get character coords and bitmap...

    // Upload the quads for each character. This is so that UV coordinates for each character are stored in vram, and rendering
    // a character does not require an upload of new geometry.
    for (char c = 0; c < FONT_NUM_CHARACTERS; c++) {
        vec4 coords = Font_character_coords(font, c);
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

    DataDictionary *dd = dd_open(g_resource_dictionary, path);
    if (dd == NULL) load_error("Could not open dictionary for geometry.");
    char *vertex_format_string;
    if (!dd_get(dd, "vertex_format", "string", &vertex_format_string)) manifest_error("vertex_format_string");
    VertexFormat vertex_format = string_to_VertexFormat(vertex_format_string);
    if (vertex_format == VERTEX_FORMAT_NONE) load_error("Invalid vertex format given.");
    char *type;
    if (!dd_get(dd, "type", "string", &type)) manifest_error("type"); //- if not doing a fatal error, remember to free the queried strings.
    
    // With this option, normals are calculated from the mesh and override whatever normals were loaded (defaults to false).
    // Normals (just a boolean flag, no variation, always calculated as the average of the normals of adjacent triangles.)
    bool calculate_normals;
    if (!dd_get(dd, "calculate_normals", "bool", &calculate_normals)) manifest_error("calculate_normals");
    // UV, texture coordinates (options for different types of projections, and parameters for those options.)
    char *calculate_uv_string;
    if (!dd_get(dd, "calculate_uv", "string", &calculate_uv_string)) manifest_error("calculate_uv");
    int calculate_uv_type;
    bool calculate_uv = true;
    if (strcmp(calculate_uv_string, "none") == 0) {
        calculate_uv = false; // don't calculate any uv coordinates. This is the default.
        calculate_uv_type = UVNone;
    } else if (strcmp(calculate_uv_string, "orthographic") == 0) {
        calculate_uv_type = UVOrthographic;
    } else {
        fprintf(stderr, ERROR_ALERT "Invalid option for uv coordinate calculation given in geometry resource definition.\n");
        exit(EXIT_FAILURE);
    }
}


vec4 Font_character_coords(Font *font, char character)
{
    return font->character_coords[character];
}




