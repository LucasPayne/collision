/*================================================================================
    make_sdf
    Create signed distance fields from font files.

https://www.freetype.org/freetype2/docs/tutorial/step1.html
================================================================================*/
#include <ft2build.h>
#include FT_FREETYPE_H
#include <png.h>
#include <setjmp.h>
//--------------------------------------------------------------------------------
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) < (Y) ? (Y) : (X))
#define ABS(X) ((X) < 0 ? -(X) : (X))

FT_Library ft_lib;
#define FT_ERROR(STR)\
{\
    fprintf(stderr, "FT_ERROR: " STR "\n");\
    exit(EXIT_FAILURE);\
}
#define mem_check(THING) if (( THING ) == NULL) { fprintf(stderr, "malloc failed\n"); exit(1); }

const int sdf_supersample = 8; // Scale-up N times when created monochrome bitmap for computing sdf for lower resolution grayscale.

void make_sdf(char *font_filename, int size_x, int size_y, int reach)
{
    FT_Face super_face; // At supersampling resolution, used for brute-force computation of signed-distance field for normal resolution.
    FT_Face sdf_face;   // Normal resolution rasterized font, determines the points at which the sdf is computed.
    if (FT_New_Face(ft_lib, font_filename, 0, &super_face) || FT_New_Face(ft_lib, font_filename, 0, &sdf_face)) {
        FT_ERROR("Failed loading font face.");
    }
    // Calculate in terms of pixel dimensions.
    // FT_Set_Pixel_Sizes(face, pixel_width, pixel_height);
    if (FT_Set_Pixel_Sizes(super_face, size_x * sdf_supersample, size_y * sdf_supersample)) {
        FT_ERROR("Failed to set character size information for supersampled font face.");
    }
    if (FT_Set_Pixel_Sizes(sdf_face, size_x, size_y)) {
        FT_ERROR("Failed to set character size information for sdf-sized font face.");
    }
    printf("Computing signed-distance field dimensions for each glyph ...\n");
    // Compute the glyph dimensions, in the sdf resolution.
    uint16_t glyph_dimensions[('z' - 'A' + 1) * 2] = { 0 };
    for (char c = 'A'; c <= 'z'; c++) {
        FT_UInt glyph_index = FT_Get_Char_Index(sdf_face, c);
        if (glyph_index == 0) {
            printf("Could not find glyph \'%c\'\n.", c);
            exit(1);
        }
        if (FT_Load_Glyph(sdf_face, glyph_index, FT_LOAD_DEFAULT)) {
            printf("Could not load glyph \'%c\'\n.", c);
            exit(1);
        }
        if (sdf_face->glyph->format != FT_GLYPH_FORMAT_BITMAP && FT_Render_Glyph(sdf_face->glyph, FT_RENDER_MODE_NORMAL)) {
            printf("Could not render glyph \'%c\'\n.", c);
            exit(1);
        }
        FT_Bitmap bitmap = sdf_face->glyph->bitmap;
        glyph_dimensions[2*('z' - 'A' + c) + 0] = (uint16_t) bitmap.width;
        glyph_dimensions[2*('z' - 'A' + c) + 1] = (uint16_t) bitmap.rows;
        printf("%c: %u, %u\n", c, (uint16_t) bitmap.width, (uint16_t) bitmap.rows);
    }
    // Rasterize each glyph at the supersampling resolution.
    for (char c = 'A'; c <= 'z'; c++) {
        FT_UInt glyph_index = FT_Get_Char_Index(super_face, c);
        if (glyph_index == 0) {
            printf("Could not find glyph \'%c\'\n.", c);
            exit(1);
        }
        if (FT_Load_Glyph(super_face, glyph_index, FT_LOAD_DEFAULT)) {
            printf("Could not load glyph \'%c\'\n.", c);
            exit(1);
        }
        if (super_face->glyph->format != FT_GLYPH_FORMAT_BITMAP && FT_Render_Glyph(super_face->glyph, FT_RENDER_MODE_NORMAL)) {
            printf("Could not render glyph \'%c\'\n.", c);
            exit(1);
        }
        int super_width = super_face->glyph->bitmap.width;
        int super_height = super_face->glyph->bitmap.rows;
        uint8_t *super_image = malloc(sizeof(uint8_t) * super_width * super_height);
        mem_check(super_image);
        for (int i = 0; i < super_height; i++) {
            for (int j = 0; j < super_width; j++) {
                super_image[i * super_width + j] = super_face->glyph->bitmap.buffer[i * super_width + j] > 0 ? 1 : 0;
            }
        }
        int sdf_width = glyph_dimensions[2*('z' - 'A' + c) + 0];
        int sdf_height = glyph_dimensions[2*('z' - 'A' + c) + 1];
        printf("Computing sdf for glyph \'%c\' ...\n", c);
        printf("sdf_width: %d\nsdf_height: %d\nsuper_width: %d\nsuper_height: %d\n", sdf_width, sdf_height, super_width, super_height);
    
        // Prepare the PNG image to be written.
        // Prepare the memory to store the sdf image, in a form which can be passed to libpng for the creation of a PNG file.
        const int buf_size = 4096;
        char buf[buf_size];
        snprintf(buf, buf_size, "char_%c.png", c);
        FILE *glyph_png = fopen(buf, "wb");
        if (glyph_png == NULL) {
            printf("Failed to create PNG file for glyph \'%c\'\n.", c);
            exit(1);
        }
        // https://www.lemoda.net/c/write-png/
#define libpng_error(STR) { fprintf(stderr, "libpng error: " STR "\n"); exit(1); }
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png_ptr == NULL) libpng_error("Failed to create a PNG struct.");
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) libpng_error("Failed to create PNG info struct.");
        // <setjmp.h>
        // $man setjmp
        if (setjmp(png_jmpbuf(png_ptr))) libpng_error("setjmp failed.");
        png_set_IHDR(png_ptr, info_ptr, sdf_width, sdf_height, 8, // depth
                     PNG_COLOR_TYPE_GRAY,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT,
                     PNG_FILTER_TYPE_DEFAULT);
        png_byte **sdf_png_rows = png_malloc(png_ptr, sizeof(png_byte *) * sdf_height);
        mem_check(sdf_png_rows);
        for (int i = 0; i < sdf_height; i++) {
            png_byte *sdf_png_row = png_malloc(png_ptr, sizeof(png_byte) * sdf_width);
            mem_check(sdf_png_row);
            sdf_png_rows[i] = sdf_png_row;
        }

        for (int sdf_i = 0; sdf_i < sdf_width; sdf_i++) {
            for (int sdf_j = 0; sdf_j < sdf_height; sdf_j++) {
                int super_i = sdf_i * sdf_supersample;
                int super_j = sdf_j * sdf_supersample;
                // check against a square determined by the value of reach.
                unsigned int min_square_dist = reach * reach;
                int tlx, tly, brx, bry;
                tlx = MAX(0, super_i - reach);
                tly = MAX(0, super_j - reach);
                brx = MIN(super_width - 1, super_i + reach);
                bry = MIN(super_height - 1, super_j + reach);
                bool inside = super_image[super_i * super_width + super_j] != 0;
                // To test for the distance to the border without rasterizing the glyph outline,
                // just find the closest == 0 if the super-image is > 0, and > 0 if the super-image is == 0. 
                for (int square_i = tlx; square_i <= brx; square_i++) {
                    for (int square_j = tly; square_j <= bry; square_j++) {
                        bool square_point_inside = super_image[square_i * super_width + square_j] != 0;
                        if (inside == square_point_inside) continue; // looking for min distance to 
                        unsigned int square_dist = (square_i - super_i) * (square_i - super_i) + (square_j - super_j) * (square_j - super_j);
                        if (square_dist < min_square_dist) min_square_dist = square_dist;
                    }
                }
                float min_dist = sqrt(min_square_dist);
                if (!inside) min_dist = -min_dist;

                // Map this floating point distance value, ranging from [-reach, reach], to [0, 255].
                uint8_t mapped_val = (int) (((min_dist + reach) / (2 * reach)) * 256);
                sdf_png_rows[sdf_i][sdf_j] = mapped_val;
                // printf("%.2f\n", min_dist);
                // getchar();
            }
        }
        // Save the PNG to the file.
        png_init_io(png_ptr, glyph_png);
        png_set_rows(png_ptr, info_ptr, sdf_png_rows);
        png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
        // Free resources.
        free(super_image);
        // Free libpng resources.
        for (int i = 0; i < sdf_height; i++) {
            png_free(png_ptr, sdf_png_rows[i]);
        }
        png_free(png_ptr, sdf_png_rows);
    }
    

#if 0
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
            if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
                printf("Could not render glyph \'%c\'\n.", c);
                exit(1);
            }
        }
        int super_width = face->glyph->bitmap.width;
        int super_height = face->glyph->bitmap.rows;
        uint8_t *super_image = malloc(sizeof(uint8_t) * super_width * super_height);
        for (int i = 0; i < super_height; i++) {
            for (int j = 0; j < super_width; j++) {
                super_image[i * super_width + j] = face->glyph->bitmap.buffer[i * super_width + j] > 0 ? 1 : 0;
            }
        }
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
        printf("Computing signed-distance field for glyph \'%c\' ...\n", c);
        printf("--------------------------------------------------------------------------------\n");
        printf("width: %u\nrows: %u\n", bitmap.width, bitmap.rows);
        printf("--------------------------------------------------------------------------------\n");
        glyph_dimensions[2*('z' - 'A' + c) + 0] = (uint16_t) bitmap.width;
        glyph_dimensions[2*('z' - 'A' + c) + 1] = (uint16_t) bitmap.rows;

        const int buf_size = 4096;
        char buf[buf_size];
        snprintf(buf, buf_size, "char_%c.png", c);
        FILE *glyph_png = fopen(buf, "wb");
        if (glyph_png == NULL) {
            printf("Failed to create PNG file for glyph \'%c\'\n.", c);
            exit(1);
        }
        // https://www.lemoda.net/c/write-png/
#define libpng_error(STR) { fprintf(stderr, "libpng error: " STR "\n"); exit(1); }
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png_ptr == NULL) libpng_error("Failed to create a PNG struct.");
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) libpng_error("Failed to create PNG info struct.");
        // <setjmp.h>
        // $man setjmp
        if (setjmp(png_jmpbuf(png_ptr))) libpng_error("setjmp failed.");
        png_set_IHDR(png_ptr, info_ptr, bitmap.width, bitmap.rows, 8, // depth
                     PNG_COLOR_TYPE_GRAY,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT,
                     PNG_FILTER_TYPE_DEFAULT);
        png_byte **row_pointers = png_malloc(png_ptr, bitmap.rows * sizeof(png_byte *));
        if (row_pointers == NULL) libpng_error("Failed to allocate for row_pointers.");
        for (int i = 0; i < bitmap.rows; i++) {
            png_byte *row = png_malloc(png_ptr, sizeof(png_byte) * bitmap.width * 1);
            row_pointers[i] = row;
            if (row == NULL) libpng_error("Failed to allocate for row.");
            for (int j = 0; j < bitmap.width; j++) {
                row[j] = bitmap.buffer[i * bitmap.width + j] > 0 ? (png_byte) 255 : (png_byte) 0; // monochromify the grayscale image given by freetype.
            }
        }
        png_init_io(png_ptr, glyph_png);
        png_set_rows(png_ptr, info_ptr, row_pointers);
        png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
        for (int i = 0; i < bitmap.rows; i++) {
            png_free(png_ptr, row_pointers[i]);
        }
        png_free(png_ptr, row_pointers);
    }
#endif
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

