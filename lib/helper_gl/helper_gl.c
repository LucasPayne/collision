/*-------------------------------------------------------------------------------
    OpenGL, GLFW, glad, etc. helper functions.
-------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper_definitions.h"
#include "helper_gl.h"

// static double TIME = 0.0;
// static double DT = 0.0;
// double time(void)
// {
//     return TIME;
// }
// double dt(void)
// {
//     return DT;
// }
// 
// // Its loop time
// void loop_time(GLFWwindow *window, void (*inner_func)(void), GLbitfield clear_mask)
// {
//     double last_time = TIME;
//     while (!glfwWindowShouldClose(window))
//     {
//         glfwPollEvents();
// 
//         last_time = TIME;
//         TIME = glfwGetTime();
//         DT = TIME - last_time;
// 
//         /* Clearing: window clear to black, viewport clear to the clear colour.
//          * (restore clear colour after window clear)
//          */
//         GLfloat clear_color[4];
//         glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);
//         glClearColor(0.0, 0.0, 0.0, 1.0);
//         glDisable(GL_SCISSOR_TEST);
//         glClear(clear_mask);
// 
//         GLint viewport[4];
//         glGetIntegerv(GL_VIEWPORT, viewport);
//         glEnable(GL_SCISSOR_TEST);
//         glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
//         glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
//         glClear(clear_mask);
// 
//         if (inner_func != NULL) {
//             inner_func();
//         }
// 
//         glFlush();
//         glfwSwapBuffers(window);
//     }
//     // Cleanup
//     glfwDestroyWindow(window);
//     glfwTerminate();
// }

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


GLFWwindow *gl_core_standard_window(char *name, void (*init_function)(void), void (*loop_function)(void), void (*close_function)(void))
{
    GLFWwindow *window;
    int horiz = 512;
    int vert = 512;
    if (!glfwInit()) {
        fprintf(stderr, "GLFW error: something went wrong initializing GLFW\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(horiz, vert, name, NULL, NULL);
    if (!window) {
        fprintf(stderr, "GLFW error: failed to create a window properly.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    return window;
}

