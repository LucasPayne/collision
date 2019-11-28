/*--------------------------------------------------------------------------------
PROJECT_LIBS:
    + text_processing
    + data/ply
--------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "helper_definitions.h"
#include "text_processing.h"
#include "data/ply.h"

void read_error(char *msg)
{
    fprintf(stderr, "READ ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    char *filename;
    if (argc != 2) {
        fprintf(stderr, "give good args\n");
        exit(EXIT_FAILURE);
    }
    filename = argv[1];

    printf("Reading file %s ...\n", filename);

    PLYStats stats;
    if (!ply_stat(filename, &stats)) {
        printf("misunderstood ply header\n");
    } else {
        printf("%d\n", stats.format);
    }


}
