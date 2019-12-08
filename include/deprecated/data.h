/*================================================================================
   Data module specifically for data needed in this project.
   Reading, saving, serializing, and test data.
================================================================================*/
#ifndef HEADER_DEFINED_DATA
#define HEADER_DEFINED_DATA
#include "shapes.h"

//================================================================================
//   Debug and utilities
//================================================================================
void print_data_directory(void);

//================================================================================
//   Reading and writing data formats/types
//================================================================================
void read_polygon(char *filename, Polygon *poly);
void write_polygon(Polygon *poly, char *filename);

//================================================================================
//   Test data
//================================================================================
// Reads from the ascii polygons directory
void ascii_polygon(char *name, Polygon *polygon);

#endif // HEADER_DEFINED_DATA
