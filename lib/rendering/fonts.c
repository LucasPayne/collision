/*--------------------------------------------------------------------------------
    Fonts module
----------------
    Monospace, ASCII, raster bitmap fonts. This module defines the Font resource.
--------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "helper_definitions.h"
#include "resources.h"
#include "rendering.h"


void Font_load(void *resource, char *path)
{
    Font font;
    #define load_error(STRING) { fprintf(stderr, ERROR_ALERT "Error loading font: %s\n", ( STRING )); exit(EXIT_FAILURE); }
    #define manifest_error(str) load_error("Font manifest file has missing or malformed " str " entry.\n")
    DataDictionary *dd = dd_open(g_resource_dictionary, path);
    if (dd == NULL) load_error("Could not open dictionary for font.");
    char *sdf_glyph_map_path;
    if (!dd_get(dd, "sdf_glyph_map", "string", &sdf_glyph_map_path)) manifest_error("sdf_glyph_map");
    FILE *sdf_glyph_map = resource_file_open(sdf_glyph_map_path, "", "rb");
    if (sdf_glyph_map == NULL) load_error("Could not load signed-distance-field glyph map.");
    free(sdf_glyph_map_path);
    char *sdf_metadata_path;
    if (!dd_get(dd, "sdf_metadata", "string", &sdf_metadata_path)) manifest_error("sdf_metadata");
    FILE *sdf_metadata = resource_file_open(sdf_metadata_path, "", "r");
    if (sdf_metadata == NULL) load_error("Could not load glyph map metadata text file.");
    free(sdf_metadata_path);
#define metadata_error() load_error("malformed metadata\n")

    font.num_glyphs = 'z' - 'A' + 1; //---preparing for just enough glyphs, since that is what is currently in the sdf data.
    Glyph glyphs[font.num_glyphs];
/*--------------------------------------------------------------------------------
 Format
 ------
 A:
     width: 10
     height: 8
     bearing_x: 0
     bearing_y: 8
     advance: 640
     uvs: 0.00,0.00,0.10,0.09
     glyph_uvs: 0.03,0.03,0.07,0.06
--------------------------------------------------------------------------------*/
    int line_num = 0;
    int index = -1;
    const int buf_size = 4096;
    char buf[buf_size];
    while (fgets(buf, buf_size, sdf_metadata) != NULL) {
        switch (line_num % 8) {
            case 0:
                index ++;
                if (sscanf(buf, "%c:", &glyphs[index].character) == EOF) metadata_error();
                break;
            case 1:
                if (sscanf(buf, "    width: %d", &glyphs[index].width) == EOF) metadata_error();
                break;
            case 2:
                if (sscanf(buf, "    height: %d", &glyphs[index].height) == EOF) metadata_error();
                break;
            case 3:
                if (sscanf(buf, "    bearing_x: %d", &glyphs[index].bearing_x) == EOF) metadata_error();
                break;
            case 4:
                if (sscanf(buf, "    bearing_y: %d", &glyphs[index].bearing_y) == EOF) metadata_error();
                break;
            case 5:
                if (sscanf(buf, "    advance: %d", &glyphs[index].advance) == EOF) metadata_error();
                break;
            case 6:
                if (sscanf(buf, "    uvs: %f,%f,%f,%f", &glyphs[index].uvs[0],&glyphs[index].uvs[1],&glyphs[index].uvs[2],&glyphs[index].uvs[3]) == EOF) metadata_error();
                break;
            case 7:
                if (sscanf(buf, "    glyph_uvs: %f,%f,%f,%f", &glyphs[index].glyph_uvs[0],&glyphs[index].glyph_uvs[1],&glyphs[index].glyph_uvs[2],&glyphs[index].glyph_uvs[3]) == EOF) metadata_error();
                break;
        }
        line_num ++;
    }
#if 0
    // Test if it is reading the text file correctly.
    for (int i = 0; i < font.num_glyphs; i++) {
        Glyph *cur_glyph = &glyphs[i];
        printf("%c:\n", cur_glyph->character);
        printf("    width: %d\n", cur_glyph->width);
        printf("    height: %d\n", cur_glyph->height);
        printf("    bearing_x: %d\n", cur_glyph->bearing_x);
        printf("    bearing_y: %d\n", cur_glyph->bearing_y);
        printf("    advance: %d\n", cur_glyph->advance);
        printf("    uvs: %.6f,%.6f,%.6f,%.6f\n", cur_glyph->uvs[0], cur_glyph->uvs[1],cur_glyph->uvs[2],cur_glyph->uvs[3]);
        printf("    glyph_uvs: %.6f,%.6f,%.6f,%.6f\n", cur_glyph->glyph_uvs[0], cur_glyph->glyph_uvs[1],cur_glyph->glyph_uvs[2],cur_glyph->glyph_uvs[3]);
    }
#endif
    
    ImageData image_data;
    if (!load_image_png(&image_data, sdf_glyph_map)) load_error("Failed to decode signed distance field png.");

    ResourceHandle tex_handle = new_resource_handle(Texture, "Fonts/computer_modern_sdf");
    Texture *tex = resource_data(Texture, tex_handle);
    font.sdf_glyph_map = tex->texture_id;
    glBindTexture(GL_TEXTURE_2D, tex->texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

#if 0
    GLuint glyph_map_texture;
    glGenTextures(1, &glyph_map_texture);
    glBindTexture(GL_TEXTURE_2D, glyph_map_texture);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGB, image_data.width, image_data.height);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,    // First mipmap level
                    0, 0, // x and y offset
                    image_data.width, image_data.height,
                    image_data.external_format, image_data.external_type,
                    image_data.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    free(image_data.data);
    font.sdf_glyph_map = glyph_map_texture;
#endif

    font.glyphs = (Glyph *) malloc(sizeof(Glyph) * font.num_glyphs);
    mem_check(font.glyphs);
    memcpy(font.glyphs, &glyphs, sizeof(Glyph) * font.num_glyphs);
    memcpy(resource, &font, sizeof(Font));
}


