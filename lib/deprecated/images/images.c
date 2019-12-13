/*--------------------------------------------------------------------------------
    Definitions for the images module.
---------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "images.h"
#include "helper_definitions.h"

static uint32_t uint32_big_to_little_endian(uint32_t bytes)
{
#define LOW_MASK 0x000000FF
    uint32_t byte_1 = bytes & LOW_MASK;
    uint32_t byte_2 = (bytes >> 8) & LOW_MASK;
    uint32_t byte_3 = (bytes >> 16) & LOW_MASK;
    uint32_t byte_4 = (bytes >> 24) & LOW_MASK;

    return byte_4 | (byte_3 << 8) | (byte_2 << 16) | (byte_1 << 24);
#undef LOW_MASK
}

bool load_image_png(ImageData *image_data, FILE *png_file)
{
    if (png_file == NULL) {
        fprintf(stderr, ERROR_ALERT "Attempted to load a PNG image from a null file.\n");
        exit(EXIT_FAILURE);
    }
    char signature[8];
    if (fread(signature, 8, 1, png_file) != 1) return false;
    const char png_magic[8] = { 0x89, 'P','N','G', 0x0D, 0x0A, 0x1A, 0x0A };
    if (memcmp(signature, png_magic, 8) != 0) return false;

    struct IHDR_info_s {
        uint32_t length;
        uint8_t type[4];

        uint32_t width;
        uint32_t height;
        uint8_t bit_depth;
        uint8_t color_type;
        uint8_t compression_method;
        uint8_t filter_method;
        uint8_t interlace_method;

        uint32_t crc;
    } info;
    if (fread(&info, sizeof(struct IHDR_info_s), 1, png_file) != 1) return false;
    if (memcmp(info.type, "IHDR", 4) != 0) return false;
    info.length  = uint32_big_to_little_endian(info.length);
    info.width  = uint32_big_to_little_endian(info.width);
    info.height  = uint32_big_to_little_endian(info.height);

    printf("PNG info:\n");
    printf("width: %u\n", info.width);
    printf("height: %u\n", info.height);

    for (int i = 0; i < 32; i++) {
        int c;
        printf("%d: %c, %x\n", i, c = fgetc(png_file), c);
    }

    struct ChunkHeader_s {
        uint32_t length;
        char chunk_type[4];
    } chunk_header;
    fread(&chunk_header, sizeof(struct ChunkHeader_s), 1, png_file);
    chunk_header.length = uint32_big_to_little_endian(chunk_header.length);
    printf("Chunk length: %u\n", chunk_header.length);
    printf("Chunk type: \"%c%c%c%c\"\n", chunk_header.chunk_type[0], chunk_header.chunk_type[1], chunk_header.chunk_type[2], chunk_header.chunk_type[3]);

    printf("! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! got through\n");
    return false;
   
}
