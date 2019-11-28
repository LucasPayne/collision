/*================================================================================
   PLY file format module - Also known as the Stanford triangle format.

Example of an ASCII PLY file:
----------------------------------------------------------------------------------
ply
format ascii 1.0
comment object: cube
element vertex 8
property float x
property float y
property float z
property float red
property float green
property float blue
element triangle 12
property int index_a
property int index_b
property int index_c
end_header
-0.100000 -0.100000 -0.100000 1.0 0.0 0.0
0.100000 0.100000 0.100000 0.0 1.0 0.0
-0.100000 -0.100000 -0.100000 0.0 0.0 1.0
0.100000 0.100000 0.100000 0.5 0.5 0.5
-0.100000 -0.100000 -0.100000 0.0 0.0 0.0
0.100000 0.100000 0.100000 1.0 1.0 1.0
-0.100000 -0.100000 -0.100000 1.0 1.0 0.0
0.100000 0.100000 0.100000 0.5 0.5 0.0
0 1 2
1 3 2
3 1 5
7 3 5
5 6 7
5 4 6
4 0 2
4 2 6
6 2 3
6 3 7
4 5 1
4 1 0
================================================================================*/
#ifndef HEADER_DEFINED_PLY
#define HEADER_DEFINED_PLY
#include <stdint.h>

typedef uint8_t PLYFormat;
enum PLY_FORMATS {
    PLY_FORMAT_ASCII_1,
    PLY_FORMAT_BINARY_1,
    NUM_PLY_FORMATS
};
typedef uint8_t PLYType;
enum PLY_TYPES {
    PLY_FLOAT,
    PLY_INT,
    NUM_PLY_TYPES
};


#define MAX_PLY_ELEMENTS 8
#define MAX_PLY_ELEMENT_NAME_LENGTH 8
#define MAX_PLY_PROPERTY_NAME_LENGTH 8
#define MAX_PLY_PROPERTIES 80

typedef struct PLYProperty_s {
    char name[MAX_PLY_PROPERTY_NAME_LENGTH];
    PLYType type;
} PLYProperty;

typedef struct PLYElement_s {
    char name[MAX_PLY_ELEMENT_NAME_LENGTH];
    unsigned int count;
    unsigned int num_properties;
    struct PLYProperty_s properties[MAX_PLY_PROPERTIES];
} PLYElement;

typedef struct PLYStats_s {
    PLYFormat format;
    unsigned int num_elements;
    struct PLYElement_s elements[MAX_PLY_ELEMENTS];
} PLYStats;

bool ply_stat(char *filename, PLYStats *ply_stats);
void print_ply_stats(PLYStats *ply_stats);

#endif // HEADER_DEFINED_PLY
