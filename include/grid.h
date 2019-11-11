/*
 *
 */

#ifndef HEADER_DEFINED_GRID
#define HEADER_DEFINED_GRID

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define FOR_GRID(ITER_I, ITER_J) \
    int ITER_I = 0;\
    int ITER_J = 0;\
    for (int _iter = 0; _iter < grid_horiz() * grid_vert(); _iter ++, ITER_I = _iter % grid_horiz(), ITER_J = _iter / grid_vert())

#define GRID_DEFINED_CHECK()\
    if (!grid_defined) {\
        fprintf(stderr, "ERROR: Must initialize the grid before usage.\n");\
        exit(EXIT_FAILURE);\
    }

#define GRID_CHECK(I, J) \
    if (!in_grid_range(I, J)) {\
        fprintf(stderr, "ERROR: Attempted to index out of range of the grid\n");\
        exit(EXIT_FAILURE);\
    }

double normalized_grid_pos_y(int j);
double normalized_grid_pos_x(int i);
int grid_vert(void);
int grid_horiz(void);
bool grid_val(int i, int j);
bool in_grid_range(int i, int j);
void grid_clear(void);
void grid_init(int horiz, int vert);
void normalized_grid_pos(int i, int j, double *x, double *y);
void set_grid(int i, int j, bool val);
bool col_in_grid_range(int i);
bool row_in_grid_range(int j);
void render_grid(GLFWwindow *window);

double det(double ax, double ay, double bx, double by);
void rasterize_wireframe_polygon(double *xs, double *ys, int n);
void rasterize_circle(double center_x, double center_y, double radius);
void rasterize_line(double ax, double ay, double bx, double by);
void rasterize_convex_polygon(double *xs, double *ys, int n);

void resize_grid(int horiz, int vert);

#endif
