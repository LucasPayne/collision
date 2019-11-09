
/*
 * Simple grid display backend with OpenGL and GLFW, to make a toy rasterizer.
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "grid.h"
#include "helper_gl.h"
#include "helper_input.h"

#define WINDOW_HORIZ 512
#define WINDOW_VERT 512

#define frand() ((double) rand() / (RAND_MAX+1.0))

int DRAWX = 0;
int DRAWY = 0;
double SIZE = 0.1;

static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);
}

double det(double ax, double ay, double bx, double by)
{
    return ax * by - ay * bx;
}

void rasterize_line(double ax, double ay, double bx, double by)
{
    double line_margin = 0.01;

    double vx = bx - ax;
    double vy = by - ay;
    double v_len = sqrt(vx * vx + vy * vy);
    double vx_n = vx / v_len;
    double vy_n = vy / v_len;

    FOR_GRID(i, j) {
        double cell_x, cell_y;
        normalized_grid_pos(i, j, &cell_x, &cell_y);
        cell_x -= ax;
        cell_y -= ay;

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

void rasterize_wireframe_convex_polygon(double *xs, double *ys, int n)
{
    for (int i = 0; i < n; i++) {
        double x = xs[i];
        double y = ys[i];
        double xp = xs[(i + 1) % n];
        double yp = ys[(i + 1) % n];
        rasterize_line(x, y, xp, yp);
    }
}


void loop(GLFWwindow *window)
{

    grid_clear();
 #if 1   // point moving
    int x_move = 0, y_move = 0;
    if (arrow_key_down(Down)) {
        y_move --;
    }
    if (arrow_key_down(Up)) {
        y_move ++;
    }
    if (arrow_key_down(Left)) {
        x_move --;
    }
    if (arrow_key_down(Right)) {
        x_move ++;
    }
    /* if (col_in_grid_range(DRAWX + x_move)) { */
        DRAWX += x_move;
    /* } */
    /* if (row_in_grid_range(DRAWY + y_move)) { */
        DRAWY += y_move;
    /* } */


    double grow_speed = 2;
    if (alt_arrow_key_down(Up)) {
        SIZE += dt() * SIZE * grow_speed;
    }
    if (alt_arrow_key_down(Down)) {
        if (SIZE > 0.005) {
            SIZE -= dt() * SIZE * grow_speed;
            if (SIZE < 0.005) SIZE = 0.005;
        }
    }

    /* double line_end_x, line_end_y; */
    /* normalized_grid_pos(DRAWX, DRAWY, &line_end_x, &line_end_y); */
    /* rasterize_line(0.5, 0.5, line_end_x, line_end_y); */
    rasterize_circle(normalized_grid_pos_x(DRAWX), normalized_grid_pos_y(DRAWY), SIZE);
    /* set_grid(DRAWX, DRAWY, true); */
#endif
 
#if 0   // Random lines
    for (int i = 0; i < 10; i++) {
        double ax = frand();
        double ay = frand();
        double bx = frand();
        double by = frand();
        rasterize_line(ax, ay, bx, by);
    }
#endif

#if 1 // Draw a convex polygon
    double xs[7] = {
                        0.9144900833140153,
                        0.9395865834424388,
                        0.6183841248097242,
                        0.4358857530188046,
                        0.11684677333606941,
                        0.30680055868869904,
                        0.4345728956923307
                   };
    double ys[7] = {
                        0.22926545393698983,
                        0.4007402157449801,
                        0.949795683127074,
                        0.8703148777876525,
                        0.483570074792403,
                        0.2921001999317757,
                        0.173552196767099
                    };

    rasterize_convex_polygon(xs, ys, 7);
    rasterize_wireframe_convex_polygon(xs, ys, 7);

#endif

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


void reshape(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, (GLsizei) width, (GLsizei) height);
}


int main(int argc, char *argv[])
{
    printf("Starting grid ...\n");

    GLFWwindow *window = init_glfw_create_context("Grid", WINDOW_HORIZ, WINDOW_VERT);
    glfwSetWindowAspectRatio(window, 1, 1);

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    grid_init(50, 50);

    loop_time(window, loop);
    
    exit(EXIT_SUCCESS);
}
