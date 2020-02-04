/*================================================================================
    make_sdf
    Create signed distance fields from font files.

https://www.freetype.org/freetype2/docs/tutorial/step1.html
https://learnopengl.com/In-Practice/Text-Rendering
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

const int sdf_supersample = 8; // Scale-up N times when created monochrome bitmap for computing sdf for lower resolution grayscale.

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

void write_png(FILE *file, uint8_t **rows, int width, int height)
{
        // Write the grayscale image as a PNG to a file.
        // https://www.lemoda.net/c/write-png/
#define libpng_error(STR) { fprintf(stderr, "libpng error: " STR "\n"); exit(1); }
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png_ptr == NULL) libpng_error("Failed to create a PNG struct.");
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) libpng_error("Failed to create PNG info struct.");
        // <setjmp.h>
        // $man setjmp
        if (setjmp(png_jmpbuf(png_ptr))) libpng_error("setjmp failed.");
        png_set_IHDR(png_ptr, info_ptr, width, height, 8, // depth
                     PNG_COLOR_TYPE_GRAY,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT,
                     PNG_FILTER_TYPE_DEFAULT);
        // Save the PNG to the file.
        png_init_io(png_ptr, file);
        png_set_rows(png_ptr, info_ptr, rows);
        png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
}

void make_sdf(char *font_filename, int size_x, int size_y, int reach, bool separate_images)
{
    const int num_glyphs = 'z' - 'A' + 1;

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
    // Prepare the structs of glyph information. These will be filled.
    Glyph glyphs[num_glyphs]; //----This may become too large to put on the stack.
    int max_sdf_width = 0;
    int max_sdf_height = 0;
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
        if (bitmap.width > max_sdf_width) max_sdf_width = bitmap.width;
        if (bitmap.rows > max_sdf_height) max_sdf_height = bitmap.rows;
        glyphs['z' - 'A' + c].width = (uint16_t) bitmap.width;
        glyphs['z' - 'A' + c].height = (uint16_t) bitmap.rows;
        glyphs['z' - 'A' + c].bearing_x = sdf_face->glyph->bitmap_left;
        glyphs['z' - 'A' + c].bearing_y = sdf_face->glyph->bitmap_top;
        glyphs['z' - 'A' + c].advance = sdf_face->glyph->advance.x;
        // The Glyph information is not yet done. The UV coordinates are needed, computed later.
        printf("%c: width: %u, height: %u, bearing_x: %d, bearing_y: %d, advance: %d\n", c, (uint16_t) bitmap.width, (uint16_t) bitmap.rows, sdf_face->glyph->bitmap_left, sdf_face->glyph->bitmap_top, sdf_face->glyph->advance.x);
    }
    // Account for the reach of the sdf.
    max_sdf_width += 2 * reach;
    max_sdf_height += 2 * reach;
    // Prepare memory for the total collated image. No packing is done. The textures are simply laid out in a grid, anchored to the top left of each cell.
    int cells_horiz = 1;
    while (cells_horiz * cells_horiz < num_glyphs) cells_horiz += 1;
    int cells_vert = 1;
    while (cells_vert * cells_horiz < num_glyphs) cells_vert += 1;
    uint8_t **total_sdf_rows = malloc(sizeof(uint8_t *) * max_sdf_height * cells_vert);
    mem_check(total_sdf_rows);
    for (int i = 0; i < max_sdf_height * cells_vert; i++) {
        // Initialize this image to zeros since not all of it is written over.
        total_sdf_rows[i] = calloc(1, sizeof(uint8_t) * max_sdf_width * cells_horiz);
        mem_check(total_sdf_rows[i]);
    }

    // Write the glyph map meta-data text file, which contains UV coordinates for the sdr rectangle and the actual glyph bounding box,
    // and glyph information such as bearings.
    FILE *metadata_file = fopen("glyph_map_metadata.txt", "w+");
    if (metadata_file == NULL) {
        fprintf(stderr, "Failed to create/open glyph map metadata file.\n");
        exit(EXIT_FAILURE);
    }

    int cur_cell_i = 0;
    int cur_cell_j = 0;
    for (char c = 'A'; c <= 'z'; c++) {
        Glyph *cur_glyph = &glyphs['z'-'A'+c];
        cur_glyph->character = c;
        // Rasterize each glyph at the supersampling resolution.
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
        int sdf_width = glyphs['z' - 'A' + c].width + reach * 2;
        int sdf_height = glyphs['z' - 'A' + c].height + reach * 2;
        printf("Computing sdf for glyph \'%c\' ...\n", c);
        printf("sdf_width: %d\nsdf_height: %d\nsuper_width: %d\nsuper_height: %d\n", sdf_width, sdf_height, super_width, super_height);

        // Prepare memory for the rows. For convenience, this is a form which can be passed to libpng, so no conversion has to be done if
        // the separate-images option is enabled. Otherwise, these rows are then placed onto the larger image buffer.
        uint8_t **sdf_rows = malloc(sizeof(uint8_t *) * sdf_height);
        mem_check(sdf_rows);
        for (int i = 0; i < sdf_height; i++) {
            uint8_t *sdf_row = calloc(1, sizeof(uint8_t) * sdf_width);
            mem_check(sdf_row);
            sdf_rows[i] = sdf_row;
        }
        // Calculate the signed-distance function using the supersampled bitmap.
        for (int sdf_i = 0; sdf_i < sdf_width; sdf_i++) {
            for (int sdf_j = 0; sdf_j < sdf_height; sdf_j++) {
                // super_i and super_j can be outside of the supersampled bitmap. This is checked for.
                int super_i = (sdf_i - reach) * sdf_supersample;
                int super_j = (sdf_j - reach) * sdf_supersample;
                // check against a square determined by the value of reach.
                unsigned int min_square_dist = reach * reach;
                int tlx, tly, brx, bry;
                tlx = super_i - reach;
                tly = super_j - reach;
                brx = super_i + reach;
                bry = super_j + reach;
                
                bool inside;
                if (super_i < 0 || super_i >= super_width || super_j < 0 || super_j >= super_height) inside = false; // Outside of the bitmap, can't be inside the glyph. Avoid out of bounds check.
                else inside = super_image[super_j * super_width + super_i] > 0;
                // To test for the distance to the border without rasterizing the glyph outline,
                // just find the closest == 0 if the super-image is > 0, and > 0 if the super-image is == 0. 
                for (int square_i = tlx; square_i <= brx; square_i++) {
                    for (int square_j = tly; square_j <= bry; square_j++) {
                        if (square_i < 0 || square_i >= super_width || square_j < 0 || square_j >= super_height) continue; // don't check out of bounds.
                        bool square_point_inside = super_image[square_j * super_width + square_i] > 0;
                        if (inside == square_point_inside) continue; // looking for min distance to other monochrome value.
                        unsigned int square_dist = (square_i - super_i) * (square_i - super_i) + (square_j - super_j) * (square_j - super_j);
                        if (square_dist < min_square_dist) min_square_dist = square_dist;
                    }
                }
                float min_dist = sqrt(min_square_dist);
                if (!inside) min_dist = -min_dist;

                // Map this floating point distance value, ranging from [-reach, reach], to [0, 255].
                uint8_t mapped_val = (int) (((min_dist + reach) / (2 * reach)) * 256);
                sdf_rows[sdf_j][sdf_i] = mapped_val;
                // printf("%.2f -> %u\n", min_dist, mapped_val);
                // getchar();
            }
        }
        if (separate_images) {
            // Create a separate PNG image for this glyph.
            const int buf_size = 4096;
            char buf[buf_size];
            snprintf(buf, buf_size, "char_%c.png", c);
            FILE *glyph_png = fopen(buf, "wb");
            if (glyph_png == NULL) {
                printf("Failed to create PNG file for glyph \'%c\'\n.", c);
                exit(1);
            }
            write_png(glyph_png, sdf_rows, sdf_width, sdf_height);
        }
        // Add this image to the sdf glyph map.
        int tlx = cur_cell_i * max_sdf_width;
        int tly = cur_cell_j * max_sdf_height;
        for (int cell_i = 0; cell_i < max_sdf_width; cell_i++) {
            for (int cell_j = 0; cell_j < max_sdf_height; cell_j++) {
                total_sdf_rows[tly + cell_j][tlx + cell_i] = cell_i * cell_j; // Create a background pattern. This part shouldn't be used anyway.
                if (cell_i >= sdf_width || cell_j >= sdf_height) continue;
                total_sdf_rows[tly + cell_j][tlx + cell_i] = sdf_rows[cell_j][cell_i];
            }
        }
        // Calculate the UV coordinates, where (0,0) is the top left and (1,1) the bottom right, of the cell, and the subrectangle UVs of the actual glyph bounding box.
        cur_glyph->uvs[0] = cur_cell_i * 1.0 / cells_horiz;
        cur_glyph->uvs[1] = cur_cell_j * 1.0 / cells_vert;
        cur_glyph->uvs[2] = (cur_cell_i + sdf_width * 1.0 / max_sdf_width) * 1.0 / cells_horiz;
        cur_glyph->uvs[3] = (cur_cell_j + sdf_height * 1.0 / max_sdf_height) * 1.0 / cells_vert;
        cur_glyph->glyph_uvs[0] = (tlx + reach) * 1.0 / (cells_horiz * max_sdf_width);
        cur_glyph->glyph_uvs[1] = (tly + reach) * 1.0 / (cells_vert * max_sdf_height);
        cur_glyph->glyph_uvs[2] = (tlx + reach + cur_glyph->width) * 1.0 / (cells_horiz * max_sdf_width);
        cur_glyph->glyph_uvs[3] = (tly + reach + cur_glyph->height) * 1.0 / (cells_vert * max_sdf_height);
        // printf("%.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", cur_glyph->uvs[0], cur_glyph->uvs[1], cur_glyph->uvs[2], cur_glyph->uvs[3], cur_glyph->glyph_uvs[0], cur_glyph->glyph_uvs[1], cur_glyph->glyph_uvs[2], cur_glyph->glyph_uvs[3]);
        // getchar();
        fprintf(metadata_file, "%c:\n", cur_glyph->character);
        fprintf(metadata_file, "    width: %d\n", cur_glyph->width);
        fprintf(metadata_file, "    height: %d\n", cur_glyph->height);
        fprintf(metadata_file, "    bearing_x: %d\n", cur_glyph->bearing_x);
        fprintf(metadata_file, "    bearing_y: %d\n", cur_glyph->bearing_y);
        fprintf(metadata_file, "    advance: %d\n", cur_glyph->advance);
        fprintf(metadata_file, "    uvs: %.6f,%.6f,%.6f,%.6f\n", cur_glyph->uvs[0], cur_glyph->uvs[1],cur_glyph->uvs[2],cur_glyph->uvs[3]);
        fprintf(metadata_file, "    glyph_uvs: %.6f,%.6f,%.6f,%.6f\n", cur_glyph->glyph_uvs[0], cur_glyph->glyph_uvs[1],cur_glyph->glyph_uvs[2],cur_glyph->glyph_uvs[3]);


        // Go to the next cell position.
        cur_cell_i ++;
        if (cur_cell_i == cells_horiz) {
            cur_cell_i = 0;
            cur_cell_j += 1;
        }
        // Free resources.
        free(super_image);
        for (int i = 0; i < sdf_height; i++) {
            free(sdf_rows[i]);
        }
        free(sdf_rows);
    } // end rasterization of this glyph.

    // Create the total sdf image file and open it.
    FILE *total_sdf_file = fopen("sdf_glyph_map.png", "wb");
    if (total_sdf_file == NULL) {
        fprintf(stderr, "Failed to create/open glyph map file.\n");
        exit(EXIT_FAILURE);
    }
    // Write the PNG sdf glyph map image.
    write_png(total_sdf_file, total_sdf_rows, cells_horiz * max_sdf_width, cells_vert * max_sdf_height);
    // Free resources.
    for (int i = 0; i < cells_vert * max_sdf_height; i++) free(total_sdf_rows[i]);
    free(total_sdf_rows);

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
    bool separate_images = false;
    while ((opt = getopt(argc, argv, "fsri")) != -1) {
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
            case 'i':
                separate_images = true;
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
    make_sdf(font_filename, size_x, size_y, reach, separate_images);
}

