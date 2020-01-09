/*
 *
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "helper_gl.h"
#include "helper_input.h"
#include "shapes.h"

static double TIME = 0.0;
static double DT = 0.0;

#if 0 // Learn quicksort, comparison keys, C and C++ facilities for stuff like this
void convex_hull_2D_points(double *points, int num_points, Polygon &out_polygon)
{
    double *vals = (double *) malloc(sizeof(double) * num_points);
    qsort(vals, 
}
#endif

static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
}

int main(int argc, char *argv[])
{
    GLFWwindow *window = init_glfw_create_context("Convex collision", 512, 512);

    glfwSetKeyCallback(window, key_callback);

    loop_time(loop);

    exit(EXIT_SUCCESS);
}
