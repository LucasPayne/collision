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


// Static function declarations --------------------------------------------------
static void recompile_shader(GLuint shader_program, GLuint shader, char *path);
//--------------------------------------------------------------------------------

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
void loop_time(GLFWwindow *window, void (*inner_func)(GLFWwindow *), GLbitfield clear_mask)
{
    double last_time = TIME;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        last_time = TIME;
        TIME = glfwGetTime();
        DT = TIME - last_time;

        /* Clearing: window clear to black, viewport clear to the clear colour.
         * (restore clear colour after window clear)
         */
        GLfloat clear_color[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glDisable(GL_SCISSOR_TEST);
        glClear(clear_mask);

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glEnable(GL_SCISSOR_TEST);
        glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
        glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
        glClear(clear_mask);

        if (inner_func != NULL) {
            inner_func(window);
        }

        glFlush();
        glfwSwapBuffers(window);
    }
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
}


GLFWwindow *init_glfw_create_context(char *name, int horiz, int vert)
{
    /* --- remove this, just do it "manually", have schematics anyway.
     * --- Recreate helper functions when know more opengl.
     * Attempts to create a GLFW OpenGL context associated to a window.
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

/* This really is just a generic file-to-lines-array reader.*/
void read_shader_source(const char *name, char **lines_out[], size_t *num_lines)
{
    /* BUGS:
     *  Lines array must be just indices until the source string is fixed. Otherwise, reallocs of the source
     *  string may break away from the location the lines array points to.
     * strncpy was corrupting memory, since the malloc'd array was only guaranteed to be enough for the new line, not
     * all the random stuff at the end of the uninitialized line-buffer. Needed to set n in strnpy to length of added line.
     */
#define TRACING 1
#define SHADER_SOURCE_LINE_MAX_LENGTH 500
#define MEM_SIZE_START 1024
#define LINES_MEM_SIZE_START 128
    FILE *fd = fopen(name, "r");
    if (fd == NULL) {
        fprintf(stderr, ERROR_ALERT "Shader %s does not exist.\n", name);
        exit(EXIT_FAILURE);
    }
    char line[SHADER_SOURCE_LINE_MAX_LENGTH];
    size_t mem_size = MEM_SIZE_START;
    size_t mem_used = 0;
    size_t lines_mem_size = LINES_MEM_SIZE_START;
    size_t lines_mem_used = 0;

    char *shader_source = (char *) malloc(mem_size * sizeof(char));
    char **lines = (char **) malloc(lines_mem_size * sizeof(char *));
    if (shader_source == NULL || lines == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory to start reading shader source %s.\n", name);
        exit(EXIT_FAILURE);
    }
    while (fgets(line, SHADER_SOURCE_LINE_MAX_LENGTH, fd) != NULL) {
        size_t len = strlen(line);
        if (len + mem_used + 1 >= mem_size) {
#if TRACING
            printf("Reallocating memory for the source text ...\n");
#endif
            while (len + mem_used + 1 >= mem_size) {
                mem_size *= 2;
            }
            char *prev_location = shader_source;
            shader_source = (char *) realloc(shader_source, mem_size * sizeof(char));
            // Update position that the lines array points to
            for (int i = 0; i < lines_mem_used; i++) {
                lines[i] = shader_source + (lines[i] - prev_location);
            }
            if (shader_source == NULL) {
                fprintf(stderr, "ERROR: Could not allocate memory when reading shader source %s.\n", name);
                exit(EXIT_FAILURE);
            }
        }
        if (lines_mem_used + 1 >= lines_mem_size) {
#if TRACING
            printf("Reallocating memory for the source lines ...\n");
#endif
            lines_mem_size *= 2;
            lines = (char **) realloc(lines, lines_mem_size * sizeof(char *));
            if (lines == NULL) {
                fprintf(stderr, "ERROR: Could not allocate memory when reading shader source %s.\n", name);
                exit(EXIT_FAILURE);
            }
        }

        strncpy(shader_source + mem_used, line, len);
        lines[lines_mem_used] = shader_source + mem_used;

        mem_used += len;
        shader_source[mem_used] = '\0';
        mem_used += 1;
        lines_mem_used += 1;
    }

    if (num_lines == 0) {
        // ... only to make sure free(lines[0]) can free the source string on the callers side.
        fprintf(stderr, "ERROR: empty shader file %s\n", name);
        exit(EXIT_FAILURE);
    }

    // Write return values
    *lines_out = lines;
    *num_lines = lines_mem_used;
    fclose(fd);
#undef TRACING
#undef SHADER_SOURCE_LINE_MAX_LENGTH
#undef MEM_SIZE_START
#undef LINES_MEM_SIZE_START
}

void load_and_compile_shader(GLuint shader, const char *shader_path)
{
#define TRACING 1
#if TRACING
    /* printf("Loading and compiling shader %d from path %s ...\n", shader, shader_path); */
#endif
    //=========================================================================================
    // glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length)
    //=========================================================================================
    // Get the source and associate it to the shader ----------------------------------
    char **lines = NULL;
    size_t num_lines = 0;
    read_shader_source(shader_path, &lines, &num_lines);
#if TRACING
    printf("Successfully read the source.\n");
    for (int i = 0; i < num_lines; i++) {
        printf("%s", lines[i]);
    }
#endif
    glShaderSource(shader, num_lines, (const GLchar * const*) lines, NULL);
    free(lines[0]);
    free(lines);
#if TRACING
    printf("Successfully associated source to shader.\n");
#endif
    //---------------------------------------------------------------------------------
    // Compile the shader and print error logs if needed ------------------------------
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
#if TRACING
    printf("Successfully attempted compilation, checking errors ...\n");
#endif
    if (compiled == GL_FALSE) {
        fprintf(stderr, "ERROR: shader %s failed to compile.\n", shader_path);
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        char *log = (char *) malloc(log_length * sizeof(char));
        if (log == NULL) {
            fprintf(stderr, "ERROR: Oh no, could not assign memory to store shader compilation error log.\n");
            exit(EXIT_FAILURE);
        }
        glGetShaderInfoLog(shader, log_length, NULL, log);
        fprintf(stderr, "Failed shader compilation error log:\n");
        fprintf(stderr, "%s", log);
        free(log);
        exit(EXIT_FAILURE);
    }
#if TRACING
    printf("Successfully compiled shader.\n");
#endif
    /* //--------------------------------------------------------------------------------- */
}

void link_shader_program(GLuint shader_program)
{
    /* Links the shader program and handles errors and error logs. */
    glLinkProgram(shader_program);
    GLint link_status;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &link_status);
    if (link_status == GL_FALSE) {
        fprintf(stderr, "ERROR: failed to link shader program.\n");
        GLint log_length; 
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &log_length);
        char *log = (char *) malloc(log_length * sizeof(char));
        if (log == NULL) {
            fprintf(stderr, "ERROR: Oh no, failed to allocate memory for error log for failed link of shader program.\n");
            exit(EXIT_FAILURE);
        }
        glGetProgramInfoLog(shader_program, log_length, NULL, log);
        fprintf(stderr, "Failed shader program link error log:\n");
        fprintf(stderr, "%s", log);
        free(log);
        exit(EXIT_FAILURE);
    }
}

//--- deprecate this
void recompile_shader_program(DynamicShaderProgram *dynamic_shader_program)
{
    if (dynamic_shader_program->vertex_shader_id != 0) {
        recompile_shader(dynamic_shader_program->id, dynamic_shader_program->vertex_shader_id, dynamic_shader_program->vertex_shader_path);
    }
    if (dynamic_shader_program->fragment_shader_id != 0) {
        recompile_shader(dynamic_shader_program->id, dynamic_shader_program->fragment_shader_id, dynamic_shader_program->fragment_shader_path);
    }
    if (dynamic_shader_program->geometry_shader_id != 0) {
        recompile_shader(dynamic_shader_program->id, dynamic_shader_program->geometry_shader_id, dynamic_shader_program->geometry_shader_path);
    }
    link_shader_program(dynamic_shader_program->id);
}

static void recompile_shader(GLuint shader_program, GLuint shader, char *path)
{
    if (!glIsProgram(shader_program)
            || !glIsShader(shader)) {
        fprintf(stderr, "ERROR: Dynamic shader program has bad handles.\n");
        exit(EXIT_FAILURE);
    }
    glDetachShader(shader_program, shader);
    load_and_compile_shader(shader, path);
    glAttachShader(shader_program, shader);
}

void print_dynamic_shader_program(DynamicShaderProgram *dynamic_shader_program)
{
    printf("Dynamic shader program:\n");
    printf("\tShader program id: %d\n", dynamic_shader_program->id);
    printf("\tVertex shader id: %d\n", dynamic_shader_program->vertex_shader_id);
    printf("\tFragment shader id: %d\n", dynamic_shader_program->fragment_shader_id);
    printf("\tGeometry shader id: %d\n", dynamic_shader_program->geometry_shader_id);
    printf("\tVertex shader path: %s", dynamic_shader_program->vertex_shader_path);
    putchar('\n');
    printf("\tFragment shader path: %s", dynamic_shader_program->fragment_shader_path);
    putchar('\n');
    printf("\tGeometry shader path: %s", dynamic_shader_program->geometry_shader_path);
    putchar('\n');
}

//--- Flesh this out and make sure it is correct.
size_t gl_type_size(GLenum gl_type)
{
    switch(gl_type) {
        case GL_FLOAT: return sizeof(float);
    }
    fprintf(stderr, ERROR_ALERT "Either invalid GL type %d was size checked, or gl_type_size does not yet map this type to its size.\n", gl_type);
    exit(EXIT_FAILURE);
}
