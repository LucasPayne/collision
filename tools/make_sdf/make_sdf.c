/*================================================================================
    make_sdf
    Create signed distance fields from font files.

https://www.freetype.org/freetype2/docs/tutorial/step1.html
================================================================================*/
#include <ft2build.h>
#include FT_FREETYPE_H
#include <png.h>
//--------------------------------------------------------------------------------
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

FT_Library ft_lib;
#define FT_ERROR(STR)\
{\
    fprintf(stderr, "FT_ERROR: " STR "\n");\
    exit(EXIT_FAILURE);\
}

const int sdf_supersample = 8; // Scale-up N times when created monochrome bitmap for computing sdf for lower resolution grayscale.

void make_sdf(char *font_filename, int size_x, int size_y, int reach)
{
    FT_Face face;
    // FT_New_Face(library, filepathname, face_index, face);
    if (FT_New_Face(ft_lib, font_filename, 0, &face)) {
        FT_ERROR("Failed loading font face.");
    }
#if 0
    // Calculate in terms of DPI and point measurements.

    // Character width and height are in 1/64ths of points.
    // Device resolution is in DPI.
    // FT_Set_Char_Size(face, char_width, char_height, horizontal_device_resolution, vertical_device_resolution);
    int DPI_x = 72;
    int DPI_y = 72;
    if (FT_Set_Char_Size(face, size_x * 64, size_y * 64, DPI_x, DPI_y)) {
        FT_ERROR("Failed to set character size information.");
    }
#else
    // Calculate in terms of pixel dimensions.

    // FT_Set_Pixel_Sizes(face, pixel_width, pixel_height);
    if (FT_Set_Pixel_Sizes(face, size_x, size_y)) {
        FT_ERROR("Failed to set character size information.");
    }
#endif

    for (char c = 'A'; c <= 'z'; c++) {
        FT_UInt glyph_index = FT_Get_Char_Index(face, c);
        if (glyph_index == 0) {
            printf("Could not find glyph \'%c\'\n.", c);
            exit(1);
        }
        // FT_Load_Glyph(face, glyph_index, load_flags);
        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) {
            printf("Could not load glyph \'%c\'\n.", c);
            exit(1);
        }
        if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
            // FT_Render_Glyph(glyph, render_mode in {FT_RENDER_MODE_MONO,FT_RENDER_MODE_NORMAL});
            // FT_RENDER_MODE_NORMAL is an 8-bit grayscale anti-aliased (coverage-based) rasterization
            // of the glyph.
            if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO)) {
                printf("Could not render glyph \'%c\'\n.", c);
                exit(1);
            }
        }
        FT_Bitmap bitmap = face->glyph->bitmap;
        /*--------------------------------------------------------------------------------
            typedef struct  FT_Bitmap_
            {
              unsigned int    rows;
              unsigned int    width;
              int             pitch;
              unsigned char*  buffer;
              unsigned short  num_grays;
              unsigned char   pixel_mode;
              unsigned char   palette_mode;
              void*           palette;

            } FT_Bitmap;
        --------------------------------------------------------------------------------*/
        printf("Computing sdf for glyph \'%c\' ...\n", c);
        printf("--------------------------------------------------------------------------------\n");
        printf("rows: %u\nwidth: %u\n", bitmap.rows, bitmap.width);
        printf("--------------------------------------------------------------------------------\n");

        const int buf_size = 4096;
        char buf[buf_size];
        snprintf(buf, buf_size, "char_%c.png", c);
        FILE *glyph_png = fopen(buf, "wb");
        if (glyph_png == NULL) {
            printf("Failed to create PNG file for glyph \'%c\'\n.", c);
            exit(1);
        }
    
    }
}

int main(int argc, char *argv[])
{
    if (FT_Init_FreeType(&ft_lib)) {
        FT_ERROR("Failed to initialize library.");
    }
    char *font_filename = NULL;
    int size_x = -1;
    int size_y = -1;
    int reach = -1;
    int opt;
    while ((opt = getopt(argc, argv, "fsr")) != -1) {
        switch (opt) {
            case 'f':
                if (optind >= argc) continue;
                font_filename = argv[optind];
                break;
            case 's':
                if (optind >= argc) continue;
                char *size_str = argv[optind];
                char *x;
                if ((x = strchr(size_str, 'x')) == NULL) continue;
                if (sscanf(size_str, "%d", &size_x) == EOF) continue;
                if (sscanf(x + 1, "%d", &size_y) == EOF) continue;
                break;
            case 'r':
                if (optind >= argc) continue;
                if (sscanf(argv[optind], "%d", &reach) == EOF) continue;
                break;
        }
    }
    if (font_filename == NULL) {
        printf("Give a font file using flag -f {filename}.\n");
        exit(0);
    }
    if (size_x == -1 || size_y == -1) {
        printf("Give a pixel size using flag -s {X}x{Y}. If either is 0, it is set equal to the other value.\n");
        exit(0);
    }
    if (reach == -1) {
        printf("Give a reach using flag -r {reach}.\n");
        exit(0);
    }
    printf("Creating signed-distance field for font %s with size %dx%d and reach %d ...\n", font_filename, size_x, size_y, reach);
    make_sdf(font_filename, size_x, size_y, reach);
}

