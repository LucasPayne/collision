/*================================================================================
   Definitions for general error checking and argument handling, and utilities.
================================================================================*/
#ifndef HEADER_DEFINED_HELPER_DEFINITIONS
#define HEADER_DEFINED_HELPER_DEFINITIONS
#include <stdio.h>
#include <stdlib.h>

// Quick double-precision random number [0, 1). From K&R.
#define frand() ((double) rand() / (RAND_MAX+1.0))

//================================================================================
// File system, files and directories
//================================================================================
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

//================================================================================
// Memory management/usage
//================================================================================
#define mem_check(POINTER)\
    if (( POINTER ) == NULL) {\
        fprintf(stderr, "malloc failed.\n");\
        exit(EXIT_FAILURE);\
    }

//================================================================================
// Argument checking
//================================================================================
#define argcheck(NUM)\
    if (argc != NUM) {\
        fprintf(stderr, "give good args");\
        exit(EXIT_FAILURE);\
    }

#endif // HEADER_DEFINED_HELPER_DEFINITIONS
