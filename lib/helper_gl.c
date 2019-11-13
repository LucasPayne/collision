/*
 * OpenGL, GLFW, glad, etc. helper functions to be included in every build.
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "helper_gl.h"

static double TIME = 0.0;
static double DT = 0.0;
double time(void)
{
    return TIME;
}
double dt(void)
{
    return DT;
}

// Its loop time
void loop_time(GLFWwindow *window, void (*inner_func)(GLFWwindow *))
{
    double last_time = TIME;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        last_time = TIME;
        TIME = glfwGetTime();
        DT = TIME - last_time;

        glClear(GL_COLOR_BUFFER_BIT);

        inner_func(window);

        glFlush();
        glfwSwapBuffers(window);
    }
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
}


GLFWwindow *init_glfw_create_context(char *name, int horiz, int vert)
{
    /* Attempts to create a GLFW OpenGL context associated to a window.
     * Errors are handled in this function, can just assume returned
     * GLFWwindow pointer is good.
     */
    if (!glfwInit()) {
        fprintf(stderr, "GLFW error: something went wrong initializing GLFW\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow *window = glfwCreateWindow(horiz, vert, name, NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "GLFW error: failed to create a window properly\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glfwSwapInterval(1);

    return window;
}

void force_aspect_ratio(GLFWwindow *window, GLsizei width, GLsizei height, double wanted_aspect_ratio)
{
    double aspect_ratio = ((double) height) / width;
    if (aspect_ratio > wanted_aspect_ratio) {
        glViewport(0, (height - wanted_aspect_ratio * width)/2.0, width, wanted_aspect_ratio * width);
    }
    else {
        glViewport((width - height / wanted_aspect_ratio)/2.0, 0, height / wanted_aspect_ratio,  height);
    }
}

