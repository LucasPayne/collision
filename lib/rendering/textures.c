/*--------------------------------------------------------------------------------
    Textures component of the rendering module.
--------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "helper_definitions.h"
#include "resources.h"
#include "rendering.h"
/*--------------------------------------------------------------------------------
libpng
--------------------------------------------------------------------------------*/
#include <png.h>
#include <setjmp.h> // C standard library non-local jumps, https://en.wikipedia.org/wiki/Setjmp.h
                    // Used in exception handling in libpng.
                    // (wk)
                    // A typical use of setjmp/longjmp is implementation of an exception mechanism that exploits the ability
                    // of longjmp to reestablish program or thread state, even across multiple levels of function calls.
                    // A less common use of setjmp is to create syntax similar to coroutines.

ResourceType Texture_RTID;
/*--------------------------------------------------------------------------------
  Load a file-backed texture.
  The searched-for assets are:
    - .png
    
  =notes=
  Currently the only texture-resource support is for 2D textures with some options in RGB/RGBA/grayscale.
--------------------------------------------------------------------------------*/
void Texture_load(void *resource, char *path)
{
    #define load_error(str) { fprintf(stderr, "Texture load error: " str "\n"); exit(EXIT_FAILURE); }

    DD *dd = dd_open(g_resource_dictionary, path);
    if (dd == NULL) {
        fprintf(ERROR_ALERT "Could not find texture \"%s\".\n", path);
        exit(EXIT_FAILURE);
    }
    ImageData image_data;
    // Try for a PNG file.
    char *type;
    char *filename;
    if (!dd_get(dd, "type", "string", &type)) load_error("No type.");
    if (!dd_get(dd, "path", "string", &filename)) load_error("No file.");
    if (strcmp(type, "png") == 0) {
        FILE *file = resource_file_open(filename, "", "rb");
        if (file == NULL) load_error("Could not open png file.");
        if (!load_image_png(&image_data, file)) load_error("Failed to decode png.");
    }
    GLenum mag_filter = GL_LINEAR;
    GLenum min_filter = GL_LINEAR;
    GLenum texture_wrap_s = GL_REPEAT;
    GLenum texture_wrap_t = GL_REPEAT;
    bool cutout;
    if (!dd_get(dd, "cutout", "bool", &cutout)) load_error("No cutout.");
    if (cutout) {
        // Removes visual artifacts when repeating a texture combined with linear interpolation, which makes opaqueness wrap around where it shouldn't.
        texture_wrap_s = GL_CLAMP_TO_EDGE;
        texture_wrap_t = GL_CLAMP_TO_EDGE;
    }
    bool nearest;
    if (!dd_get(dd, "nearest", "bool", &nearest)) load_error("No nearest.");
    if (nearest) {
        printf("Making %s nearest\n", path);getchar();
        // For example, low-resolution minecraft-block textures should use this flag.
        mag_filter = GL_NEAREST;
        min_filter = GL_NEAREST;
    }

    // free(type);
    // free(filename);

    Texture *texture = (Texture *) resource;

    // All texture images are being stored in internal format rgba:8,8,8,8-bit unsigned integers.
    // A "Texture" resource really is intended to be used as a regular surface mapping texture.
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, image_data.width, image_data.height);
    // OpenGLPG8E p279
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,    // First mipmap level
                    0, 0, // x and y offset
                    image_data.width, image_data.height,
                    image_data.external_format, image_data.external_type,
                    image_data.data);
    // Destroy the image data.
    //-------------Organize actual destruction/tear-down functions. On-heap (destruction?) versus properties on heap (teardown, terminology?)
    free(image_data.data);

    // printf("GETTING DEPTH DATA\n");
    // getchar();getchar();getchar();
    // float depth_data[4 * image_data.width * image_data.height];
    // glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, depth_data);
    // printf("GOT DEPTH\n");

    // Set the texture defaults.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap_t);

    glBindTexture(GL_TEXTURE_2D, 0);

    texture->texture_id = texture_id;
}


bool load_image_png(ImageData *image_data, FILE *file)
{
    // Using libpng: Official reference implementation and tools for the PNG format.
    if (file == NULL) {
        fprintf(stderr, ERROR_ALERT "Attempted to load a PNG image from a null file.\n");
        exit(EXIT_FAILURE);
    }
    const int NUM_MAGIC_BYTES = 8;
    uint8_t header[NUM_MAGIC_BYTES];
    if (fread(header, 1, NUM_MAGIC_BYTES, file) != NUM_MAGIC_BYTES) return false;
    if (png_sig_cmp(header, 0, NUM_MAGIC_BYTES) != 0) return false;
    // Then, later png_set_sig_bytes to tell it some bytes have been skipped over.

    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;
    if ((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp) NULL, (png_voidp) NULL, (png_voidp) NULL)) == NULL) {
        return false;
    }
    if ((info_ptr = png_create_info_struct(png_ptr)) == NULL) {
        png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
        return false;
    }
    if ((end_info = png_create_info_struct(png_ptr)) == NULL) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        return false;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        // an exception was raised in a libpng routine, jump back to here.
        // manual:
        //      When libpng encounters an error, it expects to longjmp back to your routine. There-
        //      fore, you will need to call setjmp and pass your png_jmpbuf(png_ptr). If you
        //      read the file from different routines, you will need to update the jmpbuf field every time
        //      you enter a new routine that will call a png_*() function.
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return false;
    }
    // Default libpng I/O uses fread. Give it a function pointer for this, (should be) opened in binary mode.
    png_init_io(png_ptr, file);
    png_set_sig_bytes(png_ptr, NUM_MAGIC_BYTES);

    // Equivalent to
    // png_read_info
    //      transformations
    // png_read_end
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    // These just get the IHDR width/height data.
    uint32_t width = png_get_image_width(png_ptr, info_ptr);
    uint32_t height = png_get_image_height(png_ptr, info_ptr);

    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    // Get the pixel and channel sizes. This will be used later to reduce the number of explicit cases for
    // conversion.
    uint8_t pixel_size;
    uint8_t channel_size;
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth == 8) {
        pixel_size = 1; channel_size = 1;
    } else if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth == 16) {
        pixel_size = 2; channel_size = 2;
    } else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA && bit_depth == 8) {
        pixel_size = 1; channel_size = 1;
    } else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA && bit_depth == 16) {
        pixel_size = 2; channel_size = 2;
    } else if (color_type == PNG_COLOR_TYPE_RGB && bit_depth == 8) {
        pixel_size = 3; channel_size = 1;
    } else if (color_type == PNG_COLOR_TYPE_RGB && bit_depth == 16) {
        pixel_size = 6; channel_size = 2;
    } else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA && bit_depth == 8) {
        pixel_size = 4; channel_size = 1;
    } else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA && bit_depth == 16) {
        pixel_size = 8; channel_size = 2;
    } else {
        return false;
        /* // error, for now. should do error logging stuff. */
        /* fprintf(stderr, ERROR_ALERT "PNG error, type+bitdepth combination not supported.\n"); */
        /* exit(EXIT_FAILURE); */
    }
    
/*
color_type
        - holds the width of the image
        in pixels (up to 2ˆ31).
        - holds the height of the image
        in pixels (up to 2ˆ31).
        - holds the bit depth of one of the
        image channels. (valid values are
        1, 2, 4, 8, 16 and depend also on
        the color_type. See also
        significant bits (sBIT) below).
        - describes which color/alpha channels
        are present.
        PNG_COLOR_TYPE_GRAY
        (bit depths 1, 2, 4, 8, 16)
        PNG_COLOR_TYPE_GRAY_ALPHA
        (bit depths 8, 16)
        PNG_COLOR_TYPE_PALETTE
        (bit depths 1, 2, 4, 8)
        PNG_COLOR_TYPE_RGB
        (bit_depths 8, 16)
        PNG_COLOR_TYPE_RGB_ALPHA
        (bit_depths 8, 16)
*/
    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
#if 0
    for (int i = 0; i < height; i++) {
        // process each scanline
        for (int j = 0; j < width; j++) {
            // printing per-byte, not taking into account shorts etc.
            printf("(");
            for (int k = 0; k < pixel_size; k++) {
                printf("%u", row_pointers[i][pixel_size*j + k]);
                if (k != pixel_size - 1) printf(", ");
            }
            printf(")\n");
        }
    }
    printf("width: %u, height: %u\n", width, height);
    printf("Pixel size: %u\n", pixel_size);
    fprintf(stderr, "Function unfinished.\n");
    exit(EXIT_FAILURE);
#endif
    
    // Now that the PNG image data has been read into an RGBA byte buffer, fill the ImageData referenced.
    // Map the PNG color format to an OpenGL external format and type,
    // and create the data.
    GLenum external_format;
    GLenum external_type = GL_UNSIGNED_BYTE; // All conversions are to 0-255, 8-bit integer channels.
    void *data;
#define DOWNSAMPLE(SHORT)\
    ( channel_size == 2 ? (uint8_t) (val * (1 << 8)) / (1 << 16) : (uint8_t) val )
    if (color_type == PNG_COLOR_TYPE_GRAY) {
        /* printf("Doing grayscale.\n"); */
        external_format = GL_RGB;
        // Grayscale PNG images are turned into RGB images by simply N->[N,N,N].
        // The 16-bit depth channel is downscaled to 8 bits.
        data = calloc(1, 3 * width * height * sizeof(uint8_t));
        mem_check(data);
        uint8_t *pos = (uint8_t *) data;
        for (int i = 0; i < height; i++) {
            // i'th scanline
            for (int j = 0; j < width; j++) {
                uint16_t val;
                memcpy(&val, row_pointers[i] + pixel_size*j, channel_size);
                // Downsample (right terminology?)
                uint8_t down_val = DOWNSAMPLE(val);
                for (int k = 0; k < 3; k++, pos++) pos[0] = down_val;
            }
        }
    } else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        /* printf("gray alpha\n"); */
        external_format = GL_RGBA;
        // Grayscale+alpha PNG images are turned into RGBA images by [N,A]->[N,N,N,A].
        // 16-bit depth channels are downscaled to 8 bits.
        data = calloc(1, 4 * width * height * sizeof(uint8_t));
        mem_check(data);
        uint8_t *pos = (uint8_t *) data;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                uint16_t val;
                memcpy(&val, row_pointers[i] + pixel_size*j, channel_size);
                uint8_t down_val = DOWNSAMPLE(val);
                for (int k = 0; k < 3; k++, pos++) pos[0] = down_val;
                // Get the alpha value.
                memcpy(&val, row_pointers[i] + pixel_size*j + channel_size, channel_size);
                down_val = DOWNSAMPLE(val);
                *pos++ = down_val;
            }
        }
    } else if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGBA) {
        /* printf("rgb or rgba\n"); */
        // Handling both RGB and RGBA in the same case, but with these slight differences.
        int n = color_type == PNG_COLOR_TYPE_RGB ? 3 : 4;
        external_format = color_type == PNG_COLOR_TYPE_RGB ? GL_RGB : GL_RGBA;
        // RGB and RGBA PNG images are copied over directly with 16->8-bit downsampling.
        data = calloc(1, n * width * height * sizeof(uint8_t));
        mem_check(data);
        uint8_t *pos = (uint8_t *) data;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                uint16_t val;
                uint8_t down_val;
                for (int k = 0; k < n; k++) {
                    memcpy(&val, row_pointers[i] + pixel_size*j + channel_size*k, channel_size);
                    uint8_t down_val = DOWNSAMPLE(val);
                    *pos++ = down_val;
                }
            }
        }
        /* printf("%lu/%u\n", pos - (uint8_t *) data, n*width*height*sizeof(uint8_t)); */
        // ^ test to validate it has been filled correctly.
    } else {
        fprintf(stderr, ERROR_ALERT "Something went wrong when programming PNG case-handling stuff.\n");
        exit(EXIT_FAILURE); 
    }
    image_data->width = width;
    image_data->height = height;
    image_data->external_format = external_format;
    image_data->external_type = external_type;
    image_data->data = data;
    return true;
#undef DOWNSAMPLE
}

