/*================================================================================
   A grid system. One can be active at a time.
   The grid is initialized with a maximum size, and the size is variable below
   this for the lifetime of the grid.
  
   This is for variable-resolution pixelating effects, and learning rasterization
   techniques. This should be easily swappable in as behind a renderer system in
   the entity model.
                      x:0   ->    1
   x : [0 -> 1]       1[...HORIZ...]
   y : [0 -> 1]   -->  [     .     ]
                      ^[     .     ]
                      |[   VERT    ]
                       [     .     ]
                    y:0[...........]
  
   --- add colour
 ================================================================================*/

#ifndef HEADER_DEFINED_GRID
#define HEADER_DEFINED_GRID

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//================================================================================
// Utility and error checking macros
//================================================================================
#define FOR_GRID(ITER_I, ITER_J) \
    int ITER_I = 0;\
    int ITER_J = 0;\
    for (int _iter = 0; _iter < grid_horiz() * grid_vert(); _iter ++,\
           ITER_I = _iter % grid_horiz(), ITER_J = _iter / grid_vert())
#define GRID_ACTIVE_CHECK()\
    if (!GRID_ACTIVE) {\
        fprintf(stderr, "ERROR: Must initialize the grid before usage.\n");\
        exit(EXIT_FAILURE);\
    }
#define GRID_CHECK(I, J) \
    if (!in_grid_range(I, J)) {\
        fprintf(stderr, "ERROR: Attempted to index out of range of the grid\n");\
        exit(EXIT_FAILURE);\
    }

//================================================================================
// Initialization, basic usage, and closing
//================================================================================
void grid_init(int horiz, int vert);
void render_grid(GLFWwindow *window);
void grid_clear(void);
void set_grid(int i, int j, bool val);
void resize_grid(int horiz, int vert);
void grid_close(void);

//================================================================================
// Querying information about the grid, and checking ranges
//================================================================================
int grid_vert(void);
int grid_horiz(void);
bool grid_val(int i, int j);
bool in_grid_range(int i, int j);
bool col_in_grid_range(int i);
bool row_in_grid_range(int j);

//================================================================================
// Coordinate and grid position transformation
//================================================================================
double normalized_grid_pos_x(int i);
double normalized_grid_pos_y(int j);

//================================================================================
// Rasterization routines
//================================================================================
void rasterize_wireframe_polygon(double *xs, double *ys, int n);
void rasterize_circle(double center_x, double center_y, double radius);
void rasterize_line(double ax, double ay, double bx, double by);
void rasterize_convex_polygon(double *xs, double *ys, int n);

#endif // HEADER_DEFINED_GRID
