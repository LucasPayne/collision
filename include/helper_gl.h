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
void loop_time(GLFWwindow *window, void (*inner_func)(GLFWwindow *), GLbitfield color_mask);

//================================================================================
// Shaders
//================================================================================
#define NO_SHADER 0
#define MAX_DYNAMIC_SHADER_PROGRAM_PATH_LENGTH 500
typedef struct DynamicShaderProgram_s {
    GLuint id;
    GLuint vertex_shader_id;
    GLuint fragment_shader_id;
    GLuint geometry_shader_id;
    char vertex_shader_path[MAX_DYNAMIC_SHADER_PROGRAM_PATH_LENGTH];
    char fragment_shader_path[MAX_DYNAMIC_SHADER_PROGRAM_PATH_LENGTH];
    char geometry_shader_path[MAX_DYNAMIC_SHADER_PROGRAM_PATH_LENGTH];
} DynamicShaderProgram;

void link_shader_program(GLuint shader_program);
void load_and_compile_shader(GLuint shader, const char *shader_path);
void read_shader_source(const char *name, char **lines_out[], size_t *num_lines);

void recompile_shader_program(DynamicShaderProgram *dynamic_shader_program);
void print_dynamic_shader_program(DynamicShaderProgram *dynamic_shader_program);

//================================================================================
// GL types, definitions, values, ...
//================================================================================
void gl_type_size(GLenum gl_type);

#endif // HEADER_DEFINED_HELPER_GL
