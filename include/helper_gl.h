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
void force_aspect_ratio(GLFWwindow *window, GLsizei width, GLsizei height, double wanted_aspect_ratio);
GLFWwindow *gl_core_standard_window(char *name, void (*init_function)(void), void (*loop_function)(void), void (*close_function)(void));

//================================================================================
// Time and loop
//================================================================================
double time(void);
double dt(void);
void loop_time(GLFWwindow *window, void (*inner_func)(void), GLbitfield clear_mask);

//================================================================================
// GL types, definitions, values, ...
//================================================================================
size_t gl_type_size(GLenum gl_type);

#endif // HEADER_DEFINED_HELPER_GL
