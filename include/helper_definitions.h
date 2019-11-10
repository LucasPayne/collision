#ifndef HEADER_DEFINED_HELPER_DEFINITIONS
#define HEADER_DEFINED_HELPER_DEFINITIONS

#include <stdio.h>
#include <stdlib.h>

#define frand() ((double) rand() / (RAND_MAX+1.0))

#define opendir_handle(NAME, DIR_NAME)\
    DIR *NAME = opendir(( DIR_NAME ));\
    if (( NAME ) == NULL) {\
        fprintf(stderr, "directory not found.\n");\
        exit(EXIT_FAILURE);\
    }

#define fopen_handle(NAME, FILE_NAME, FLAGS)\
    FILE *NAME = fopen(( FILE_NAME ), ( FLAGS ));\
    if (( NAME ) == NULL) {\
	fprintf(stderr, "file failed to open.\n");\
        exit(EXIT_FAILURE);\
    }

#define mem_check(POINTER)\
    if (( POINTER ) == NULL) {\
        fprintf(stderr, "malloc failed.\n");\
        exit(EXIT_FAILURE);\
    }

#define argcheck(NUM)\
    if (argc != NUM) {\
        fprintf(stderr, "give good args");\
        exit(EXIT_FAILURE);\
    }

#endif

