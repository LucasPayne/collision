/*
 * Viewer for polygons.
 * Useful to: have hotkeys in file explorer, e.g. ranger, to view polygons in the data directory.
 *     for example
 *         map \p shell /path/to/poly_view %s
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "helper_definitions.h"
#include "helper_gl.h"
#include "helper_input.h"
#include "data.h"
#include "shapes.h"

Polygon poly;

static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);
}

void loop(GLFWwindow *window)
{
    polygon_draw(&poly);
}

void reshape(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, (GLsizei) width, (GLsizei) height);
}

int main(int argc, char *argv[])
{
    //================================================================================
    // Read the polygon
    //================================================================================
    if (argc == 3) {
        if (strcmp(argv[1], "-ascii") == 0) {
            char *ascii_poly_name = argv[2];

            ascii_polygon(ascii_poly_name, &poly);

            for (int i = 0; i < poly.num_vertices; i++) {
                poly.vertices[i].x *= 0.05;
                poly.vertices[i].y *= 0.05;
            }

            printf("Got poly:\n");
            polygon_print(&poly);
        }
    }
    else if (argc == 2) {
        char *polygon_filename = argv[1];

        read_polygon(polygon_filename, &poly);

        for (int i = 0; i < poly.num_vertices; i++) {
            poly.vertices[i].x *= 0.05;
            poly.vertices[i].y *= 0.05;
        }

        printf("Got poly:\n");
        polygon_print(&poly);
    } else {
        printf("give good args\n");
        exit(EXIT_SUCCESS);
    }
    //================================================================================
    // Set up for viewing
    //================================================================================
    
    

    GLFWwindow *window = init_glfw_create_context("Shapes", 512, 512);
    glfwSetWindowAspectRatio(window, 1, 1);

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    loop_time(window, loop);
    
    exit(EXIT_SUCCESS);
}
