/*
 *
 */

#ifndef HEADER_DEFINED_SHAPES
#define HEADER_DEFINED_SHAPES

#define MAX_POLYGON_VERTICES 512

typedef struct Point2f_s {
    double x;
    double y;
} Point2f;

typedef struct Polygon_s {
    int num_vertices;
    Point2f *vertices;
} Polygon;

void point2f_print(Point2f point);
void polygon_print(Polygon *polygon);
void polygon_draw(Polygon *polygon);

#endif
