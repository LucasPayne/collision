/*
 *
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include "shapes.h"

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

void polygon_draw(Polygon *polygon)
{
    glBegin(GL_POLYGON);
    for (int i = 0; i < polygon->num_vertices; i++) {
        glVertex2f(polygon->vertices[i].x, polygon->vertices[i].y);
    }
    glEnd();
}
