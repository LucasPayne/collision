#ifndef HEADER_DEFINED_DATA
#define HEADER_DEFINED_DATA

#include "shapes.h"

void print_data_directory(void);
void ascii_polygon(char *name, Polygon *polygon);
void read_polygon(char *filename, Polygon *poly);
void write_polygon(Polygon *poly, char *filename);

#endif
