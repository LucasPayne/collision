/*
 *
 */

#ifndef HEADER_DEFINED_SHAPES
#define HEADER_DEFINED_SHAPES

typedef struct Point2f_s {
    double x;
    double y;
} Point2f;

typedef struct Vec2f_s {
    double x;
    double y;
} Vec2f;

typedef struct Polygon_s {
    int num_vertices;
    Point2f *vertices;
} Polygon;

void point2f_print(Point2f point);
void polygon_print(Polygon *polygon);
void polygon_draw(Polygon *polygon);

Vec2f vec_to(Point2f a, Point2f b);
double dot(Vec2f a, Vec2f b);

#endif
