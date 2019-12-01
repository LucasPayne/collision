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
#include <stdbool.h>

typedef uint8_t PLYFormat;
enum PLY_FORMATS {
    PLY_FORMAT_NONE,
    PLY_FORMAT_ASCII_1,
    PLY_FORMAT_BINARY_LITTLE_ENDIAN_1,
    PLY_FORMAT_BINARY_BIG_ENDIAN_1,
    NUM_PLY_FORMATS
};
typedef int8_t PLYType;
#define NULL_PLY_TYPE -1
enum PLY_TYPES { // do not shuffle these!
    PLY_CHAR,
    PLY_UCHAR,
    PLY_SHORT,
    PLY_USHORT,
    PLY_INT,
    PLY_UINT,
    PLY_FLOAT,
    PLY_DOUBLE,
    NUM_PLY_TYPES
};

typedef struct PLY_s {
    char *filename;
    PLYFormat format;
    int num_elements;
    PLYElement *first_element;
} PLY;
typedef struct PLYElement_s {
    char *name;
    int count;
    int num_properties;
    struct PLYElement_s *next_element;
    PLYProperty *first_property;
} PLYElement;
typedef struct PLYProperty_s {
    char *name;
    PLYType type;
    bool is_list;
    PLYType list_count_type;
    struct PLYProperty_s *next_property;
} PLYProperty;

void init_ply(PLY *ply);
void init_ply_element(PLYElement *ply_element);
void init_ply_property(PLYProperty *ply_property);
PLY *read_ply(char *filename);
void print_ply(PLY *ply);

#endif // HEADER_DEFINED_PLY
