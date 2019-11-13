/*--------------------------------------------------------------------------------
   Definitions for the shapes module.
   See the header for details.
---------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "shapes.h"
#include "iterator.h"

void point2f_print(Point2f point)
{
    printf("(%.2lf %.2lf)", point.x, point.y);
}

// Readable description of the polygon
void polygon_print(Polygon *polygon)
{
    printf(".num_vertices: %d\n", polygon->num_vertices);
    printf(".vertices:\n");
    for (int i = 0; i < polygon->num_vertices; i++) {
        putchar('\t');
        point2f_print(polygon->vertices[i]);
        putchar('\n');
    }
}

Vec2f vec_to(Point2f a, Point2f b)
{
    Vec2f vec;
    vec.x = b.x - a.x;
    vec.y = b.y - a.y;
    return vec;
}

double dot(Vec2f a, Vec2f b)
{
    return a.x * b.x + a.y * b.y;
}
