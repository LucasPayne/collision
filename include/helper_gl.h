/*================================================================================
   Helper module for working with a real-time OpenGL application.
================================================================================*/
#ifndef HEADER_DEFINED_HELPER_GL
#define HEADER_DEFINED_HELPER_GL
#include <GLFW/glfw3.h>

// Got this just from checking, --- get the actual ratio
#define SCREEN_ASPECT_RATIO 0.5615835777126099

//================================================================================
// GLFW, windowing, and context creation
//================================================================================
GLFWwindow *init_glfw_create_context(char *name, int horiz, int vert);
void force_aspect_ratio(GLFWwindow *window, GLsizei width, GLsizei height, double wanted_aspect_ratio);

//================================================================================
// Time and loop
//================================================================================
double time(void);
double dt(void);
void loop_time(GLFWwindow *window, void (*inner_func)(GLFWwindow *));

#endif // HEADER_DEFINED_HELPER_GL
