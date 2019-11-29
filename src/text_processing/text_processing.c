/*--------------------------------------------------------------------------------
PROJECT_LIBS:
    + glad
    + helper_gl
    + text_processing
    + data/ply
    + mesh
--------------------------------------------------------------------------------*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "helper_gl.h"
#include "helper_definitions.h"
#include "text_processing.h"
#include "data/ply.h"
#include "mesh.h"

void read_error(char *msg)
{
    fprintf(stderr, "READ ERROR: %s\n", msg);
    exit(EXIT_FAILURE); }

int main(int argc, char *argv[])
{
    char *filename;
    if (argc != 2) {
        fprintf(stderr, "give good args\n");
        exit(EXIT_FAILURE);
    }
    filename = argv[1];

    printf("Reading file %s ...\n", filename);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "couldn't find file ...\n");
        exit(EXIT_FAILURE);
    }

    PLYStats stats;
    if (!ply_stat(file, &stats)) {
        printf("misunderstood ply header\n");
        return 0;
    }

    /* print_ply_stats(&stats); */

    Mesh mesh;
    load_mesh_ply(&mesh, VERTEX_FORMAT_3C, filename);
}
