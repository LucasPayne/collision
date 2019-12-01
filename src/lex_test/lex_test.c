/*
PROJECT_LIBS:
	+ data/ply
	+ text_processing
	+ text_processing/ply.yy
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "text_processing.h"
#include "data/ply.h"
#include "text_processing/ply.yy.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "give good args\n");
        exit(EXIT_FAILURE);
    }
    char *filename = argv[1];

    PLYStats stats;
    lex_ply_header(&stats, filename);

    print_ply_stats(&stats);
}
