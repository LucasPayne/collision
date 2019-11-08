/*
 *
 */

#ifndef HEADER_DEFINED_HELPER_GL
#define HEADER_DEFINED_HELPER_GL

#include <GLFW/glfw3.h>
#include <string>

double time(void);
double dt(void);
void loop_time(GLFWwindow *window, void (*inner_func)(GLFWwindow *));
GLFWwindow *init_glfw_create_context(std::string const &name, int horiz, int vert);

#endif
