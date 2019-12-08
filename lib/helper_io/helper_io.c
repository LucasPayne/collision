/*-------------------------------------------------------------------------------
    Helper functions for I/O, file reading and writing, and file system usage.
-------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "helper_definitions.h"
#include "helper_io.h"

/* Load a file fully into application memory, and return a pointer to the allocated memory.
 * The out_file_size is passed by reference and gives the size in bytes that was read into application memory.
 */
void *load_file(FILE *file, int *out_file_size)
{
    if (file == NULL) goto load_error;

    uint32_t file_size = 0;
    const uint32_t block_size = 4096;
    size_t space = block_size;
    void *image = malloc(space);
    mem_check(image);
    void *next_block = image;
    
    while (1) {
        size_t num_read = fread(next_block, 1, block_size, file);
        /* printf("read %u bytes\n", num_read); */
        if (num_read != block_size) {
            if (ferror(file)) goto load_error;
            if (feof(file)) {
                file_size += num_read;
                *out_file_size = file_size;
                return image;
            }
            goto load_error;
        }
        image = realloc(image, space + block_size);
        mem_check(image);
        next_block = image + space;
        space += block_size;
        file_size += block_size;
    }
load_error:
    *out_file_size = 0;
    return NULL;
}
