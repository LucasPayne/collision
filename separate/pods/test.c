
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

static uint32_t g_resource_path_length = 0;
static char *g_resource_path = NULL;


/* The resource path variable is formatted as
 * <Drive name>:<Path>:<Drive name>:<Path>:...
 *
 */

void resource_path_add(char *drive_name, char *path)
{
    size_t len = strlen(path) + 1 + strlen(drive_name);
    if (g_resource_path == NULL) {
        g_resource_path_length = len + 1;
        g_resource_path = (char *) malloc(g_resource_path_length * sizeof(char));
        /* mem_check(g_resource_path); */
        strcpy(g_resource_path, drive_name);
        g_resource_path[strlen(drive_name)] = ':';
        strcpy(g_resource_path + strlen(drive_name) + 1, path);
        return;
    }
    uint32_t old_length = g_resource_path_length;
    g_resource_path_length += len + 1;
    g_resource_path = (char *) realloc(g_resource_path, g_resource_path_length * sizeof(char));
    /* mem_check(g_resource_path); */
    g_resource_path[old_length - 1] = ':';
    strcpy(g_resource_path + old_length, drive_name);
    g_resource_path[old_length + strlen(drive_name)] = ':';
    strcpy(g_resource_path + old_length + strlen(drive_name) + 1, path);
}


FILE *resource_file_open(char *path, char *suffix, char *flags)
{
    char path_buffer[512];
    char drive_buffer[64];

    char *drive = path;
    path = strchr(path, '/');
    if (path == NULL) {
        fprintf(stderr, "Bad path given.\n");
        exit(EXIT_FAILURE);
    }
    int drive_length = path - drive;

    char *prefix = g_resource_path;
    if (prefix == NULL) return NULL;

    do {
        int i;
        for (i = 0; prefix[i] != ':'; i++) {
            if (prefix[i] == '\0') {
                fprintf(stderr, "Bad resource path variable.\n");
                exit(EXIT_FAILURE);
            }
            drive_buffer[i] = prefix[i];
        }
        drive_buffer[i] = '\0';
        /* printf("Read drive %s\n", drive_buffer); */

        i ++;
        int j = 0;
        for (; prefix[i] != '\0' && prefix[i] != ':'; i++, j++) {
            path_buffer[j] = prefix[i];
        }

        path_buffer[j] = '/';
        strcpy(path_buffer + j + 1, path);
        /* printf("Checking %s\n", path_buffer); */

        printf("%s = ", drive_buffer);
        for (int i = 0; i < drive_length; i++) putchar(drive[i]);
        printf(" ? \n");
        if (strlen(drive_buffer) == drive_length && strncmp(drive_buffer, drive, drive_length) == 0) {
            // Checking lengths since a strncmp can have equal prefixes yet the drive buffer stores a longer drive name.
            FILE *file = fopen(path_buffer, flags);
            if (file != NULL) return file;
        }

        prefix = strchr(prefix, ':');
        prefix = strchr(prefix + 1, ':');
        if (prefix != NULL) prefix ++;

    } while (prefix != NULL);
    return NULL;
}

void write_resource_image(void *image, size_t size, char *path)
{
    FILE *file = resource_image_open(path, "wb+");
    if (file == NULL) goto write_error;

    int num_written = fwrite(image, size, 1, file);
    /* printf("num_written: %d\n", num_written); */
    if (num_written != 1) {
        goto write_error;
    }
    return;
write_error:
    fprintf(stderr, "Failed to write resource image.\n");
    exit(EXIT_FAILURE);
}

void *load_resource_image(char *path, int *out_file_size)
{
    FILE *file = resource_image_open(path, "rb");
    if (file == NULL) goto load_error;

    uint32_t file_size = 0;
    const uint32_t block_size = 4096;
    size_t space = block_size;
    void *image = malloc(space);
    /* mem_check(image); */
    void *next_block = image;
    
    while (1) {
        uint32_t num_read = fread(next_block, 1, block_size, file);
        printf("reading %u ...\n", num_read);
        if (num_read != block_size) {
            printf("finishing\n");
            if (ferror(file)) goto load_error;
            if (feof(file)) {
                file_size += num_read;
                *out_file_size = file_size;
                return image;
            }
            goto load_error;
        }
        image = realloc(image, space + block_size);
        /* mem_check(image); */
        next_block = image + space;
        space += block_size;
        file_size += block_size;
    }
load_error:
    *out_file_size = 0;
    return NULL;
}

int main(void)
{
    printf("going\n");

    resource_path_add("Hello", "/home/jungus/bungelmungus");
    puts(g_resource_path);
    resource_path_add("Jungle", "/fungus/jungus");
    puts(g_resource_path);
    resource_path_add("Textures", "/home/lucas/code/collision/data/textures");
    puts(g_resource_path);

    if (resource_image_open("Textures/dirt.png", "rb") == NULL) {
        printf("not found.\n");
    } else {
        printf("found!\n");
    }

    int file_size;
    void *image = load_resource_image("Textures/dirt.png", &file_size);
    printf("%p, %d\n", image, file_size);

    write_resource_image(image, file_size, "Textures/dort.png");
    free(image);
    printf("written!\n");
}
