/*================================================================================
   Image facilities.

Resources:
OpenGL Programming Guide, 8th Edition
    p283, specifying texture data
================================================================================*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "helper_definitions.h"

typedef struct ImageData_s {
    uint32_t width;
    uint32_t height;
    GLenum swizzle[4]; // Swizzle for RGBA
    GLenum external_format; // Channels, is-integer e.g. GL_RG
    GLenum external_type;   // Data type e.g. GL_UNSIGNED_SHORT, so the format in all is two unsigned 16-bit channels packed into data.
    void *data;
} ImageData;

bool load_image_png(ImageData *image_data, FILE *png_file);
