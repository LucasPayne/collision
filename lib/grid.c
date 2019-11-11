/*
 *
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <math.h>
#include "helper_definitions.h"
#include "grid.h"

#define GRID_DEBUG 0

static unsigned int MAX_HORIZ;
static unsigned int MAX_VERT;
static unsigned int GRID_HORIZ;
static unsigned int GRID_VERT;
static bool grid_defined = false;
static bool *GRID;

int grid_horiz(void)
{
    return GRID_HORIZ;
}
int grid_vert(void)
{
    return GRID_VERT;
}

void resize_grid(int horiz, int vert)
{
    if (horiz > MAX_HORIZ || vert > MAX_VERT) {
        fprintf(stderr, "ERROR: tried to increase the grid size too much.\n");
        exit(EXIT_FAILURE);
    }
    if (horiz < 0 || vert < 0) {
        fprintf(stderr, "ERROR: tried to decrease the grid size below zero.\n");
        exit(EXIT_FAILURE);

    }
    GRID_HORIZ = horiz;
    GRID_VERT = vert;
}


double normalized_grid_pos_x(int i)
{
    return ((double) i) / GRID_HORIZ;
}
double normalized_grid_pos_y(int j)
{
    return ((double) j) / GRID_VERT;
}

void normalized_grid_pos(int i, int j, double *x, double *y)
{
    *x = ((double) i) / GRID_HORIZ;
    *y = ((double) j) / GRID_VERT;
}

void grid_init(int max_horiz, int max_vert)
{
    bool *mem_ptr = (bool *) malloc(sizeof(bool) * max_horiz * max_vert);
    if (!mem_ptr) {
        fprintf(stderr, "ERROR: failed to allocate memory for the grid.\n");
        exit(EXIT_FAILURE);
    }
    MAX_HORIZ = max_horiz;
    MAX_VERT = max_vert;
    GRID_HORIZ = MAX_HORIZ;
    GRID_VERT = MAX_VERT;
    GRID = mem_ptr;
    grid_defined = true;
}


bool col_in_grid_range(int i)
{
    return 0 <= i && i < GRID_HORIZ;
}
bool row_in_grid_range(int j)
{
    return 0 <= j && j < GRID_VERT;
}
bool in_grid_range(int i, int j)
{
    return 0 <= i && 0 <= j &&
        i < GRID_HORIZ && j < GRID_VERT;
}

void set_grid(int i, int j, bool val)
{
    GRID_DEFINED_CHECK();
#if GRID_DEBUG
    GRID_CHECK(i, j);
#endif
    GRID[i * GRID_VERT + j] = val;
}

bool grid_val(int i, int j)
{
    GRID_DEFINED_CHECK();
#if GRID_DEBUG
    GRID_CHECK(i, j);
#endif
    return GRID[i * GRID_VERT + j];
}

void grid_clear(void)
{
    FOR_GRID(i, j) {
        set_grid(i, j, false);
    }
}

void render_grid(GLFWwindow *window)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    double aspect_ratio = ((double) height) / width;
    double x_start = -1.0;
    double x_end = 1.0;
    double y_start = -1.0;
    double y_end = 1.0;

    if (aspect_ratio > 1.0) {
        y_start = -(((double) width)/height);
        y_end = ((double) width)/height;
    }
    else if (aspect_ratio < 1.0) {
        x_start = -(((double) height)/width);
        x_end = ((double) height)/width;
    }
    FOR_GRID(i, j) {
        // if (frand() > ((double) j)/grid_vert()) set_grid(i, j, true);
        if (grid_val(i, j)) {

            double xleft, ydown;
            normalized_grid_pos(i, j+1, &xleft, &ydown);
            double xright, yup;
            normalized_grid_pos(i+1, j, &xright, &yup);

            double screen_xleft, screen_ydown, screen_xright, screen_yup;
            screen_xleft = x_start + xleft * (x_end - x_start);
            screen_xright = x_start + xright * (x_end - x_start);
            screen_ydown = y_start + ydown * (y_end - y_start);
            screen_yup = y_start + yup * (y_end - y_start);

            glBegin(GL_POLYGON);
             glVertex2f(screen_xleft, screen_ydown);
             glVertex2f(screen_xright, screen_ydown);
             glVertex2f(screen_xright, screen_yup);
             glVertex2f(screen_xleft, screen_yup);
            glEnd();
        }
    }
}

//================================================================================
// Rasterization
//================================================================================

// ... somewhere else
double det(double ax, double ay, double bx, double by)
{
    return ax * by - ay * bx;
}

void rasterize_wireframe_polygon(double *xs, double *ys, int n)
{
    for (int i = 0; i < n; i++) {
        double x = xs[i];
        double y = ys[i];
        double xp = xs[(i + 1) % n];
        double yp = ys[(i + 1) % n];
        rasterize_line(x, y, xp, yp);
    }
}
void rasterize_circle(double center_x, double center_y, double radius)
{
    FOR_GRID(i, j) {
        double cell_x = normalized_grid_pos_x(i) - center_x;
        double cell_y = normalized_grid_pos_y(j) - center_y;
        
        double cell_sqdist = cell_x * cell_x + cell_y * cell_y;

        /* if (cell_sqdist <= radius * radius) { */
        /* /1* if (abs(sqrt(cell_sqdist) - radius) <= 0.03) { *1/ */
        /*     set_grid(i, j, true); */
        /* } */


        if (abs(sqrt(cell_sqdist) - radius) <= 0.03) {
            set_grid(i, j, true);
        }
        else if (cell_sqdist <= radius * radius) {
            if (frand() > 0.5) set_grid(i, j, true);
        }
    }
}

static int x_to_grid(double x)
{
    return (int) (GRID_HORIZ * x);
}
static int y_to_grid(double y)
{
    return (int) (GRID_VERT * y);
}
void rasterize_line(double ax, double ay, double bx, double by)
{
    int num_lerps = 20;
    for (int i = 0; i < num_lerps; i++) {
        double lerp_x = ax + (bx - ax) * (((double) i)/num_lerps);
        double lerp_y = ay + (by - ay) * (((double) i)/num_lerps);
        int lerp_i = x_to_grid(lerp_x);
        int lerp_j = y_to_grid(lerp_y);
        if (in_grid_range(lerp_i, lerp_j)) {
            set_grid(lerp_i, lerp_j, true);
        }
    }
}

#if 0
void rasterize_line(double ax, double ay, double bx, double by)
{
    double line_margin = 0.01;

    double vx = bx - ax;
    double vy = by - ay;
    double v_len = sqrt(vx * vx + vy * vy);
    double vx_n = vx / v_len;
    double vy_n = vy / v_len;

    FOR_GRID(i, j) {
        
        /* if (i == 3) { */
        /*     set_grid(i, j, true); */
        /* } */
        /* continue; */

        double cell_x = normalized_grid_pos_x(i) - ax;
        double cell_y = normalized_grid_pos_y(j) - ay;

        double cell_line_pos = cell_x * vx_n + cell_y * vy_n;
        if (cell_line_pos < 0 || cell_line_pos > v_len) {
            continue;
        }

        double cell_orth_line_pos = abs(cell_x * (-vy_n) + cell_y * vx_n);
        if (abs(cell_orth_line_pos) < line_margin) {
            set_grid(i, j, true);
        }
    }
}
#endif

void rasterize_convex_polygon(double *xs, double *ys, int n)
{
    FOR_GRID(col, row) {
        bool discard = false;
        double cell_x = normalized_grid_pos_x(col);
        double cell_y = normalized_grid_pos_y(row);
        for (int i = 0; i < n; i++) {
            double x = xs[i];
            double y = ys[i];
            double xp = xs[(i + 1) % n];
            double yp = ys[(i + 1) % n];

            if ((cell_x - x) * (-(yp - y)) + (cell_y - y) * (xp - x) < 0) {
                discard = true;
                break;
            }
        }
        if (discard) {
            continue;
        }
        /* if (sin(time() * 3 + cell_x * 50) < 0.5) set_grid(col, row, true); */
        if (frand() > ((double) row)/grid_vert()) set_grid(col, row, true);
    }
}

