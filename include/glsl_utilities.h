#ifndef HEADER_DEFINED_GLSL_UTILITIES
#define HEADER_DEFINED_GLSL_UTILITIES
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "helper_definitions.h"

/*--------------------------------------------------------------------------------
Shader bookkeeping stuff. Reading the source into application memory
(OpenGL requires this since it needs to send all the source to the driver to be compiled),
loading and compilation, and linking, with error handling and log printing.
--------------------------------------------------------------------------------*/
void read_shader_source(const char *name, char **lines_out[], size_t *num_lines);
bool load_and_compile_shader(GLuint shader_id, const char *shader_path);
bool link_shader_program(GLuint shader_program_id);

/*--------------------------------------------------------------------------------
glsl include-path. glsl header files are searched for in these paths, which must
be added to the path at runtime.
--------------------------------------------------------------------------------*/
FILE *glsl_include_path_open(char *name);
void glsl_include_path_add(char *directory);


#endif // HEADER_DEFINED_GLSL_UTILITIES
