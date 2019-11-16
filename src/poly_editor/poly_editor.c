/* PROJECT_LIBS:
 *      + glad
 *      + helper_gl
 *      + helper_input
 *      + entity
 *      + iterator
 *      - components/Transform2D
 * A polygon editor
 */

// How should this actually be done? Application-local files? Asset directory?
#define APPLICATION_LOCATION "/home/lucas/code/collision/src/poly_editor/"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "helper_gl.h"
#include "helper_input.h"
#include "entity.h"
#include "components/Transform2D.h"
#undef Transform2D_TYPE_ID
#define Transform2D_TYPE_ID 1

// Globals for testing -----------------------------------------------------------
GLuint triangle_vao;
//--------------------------------------------------------------------------------

static double ASPECT_RATIO;

static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);

    if (action == GLFW_PRESS && key == GLFW_KEY_SPACE) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        printf("%d %d\n", width, height);
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{

}

void init_program(void)
{
    init_entity_model();
}
void loop(GLFWwindow *window)
{
    glBindVertexArray(triangle_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    update_entity_model();
}
void close_program(void)
{
    close_entity_model();
}

void reshape(GLFWwindow* window, int width, int height)
{
    force_aspect_ratio(window, width, height, ASPECT_RATIO);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    printf("x:%d, y: %d, width: %d, height: %d\n", viewport[0], viewport[1], viewport[2], viewport[3]);
}

/* This really is just a generic file-to-lines-array reader.*/
void read_shader_source(char *name, char **lines_out[], size_t *num_lines)
{
#define TRACING 0
#define SHADER_SOURCE_LINE_MAX_LENGTH 500
#define MEM_SIZE_START 1024
#define LINES_MEM_SIZE_START 128
    FILE *fd = fopen(name, "r");
    if (fd == NULL) {
        fprintf(stderr, "ERROR: Shader %s does not exist.\n", name);
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
#if TRACING
        printf("reading LINE %ld: %s\n", lines_mem_used, line);
#endif
        if (len + mem_used + 1 >= mem_size) {
            while (len + mem_used + 1 >= mem_size) {
                mem_size *= 2;
            }
            shader_source = (char *) realloc(shader_source, mem_size * sizeof(char));
            if (shader_source == NULL) {
                fprintf(stderr, "ERROR: Could not allocate memory when reading shader source %s.\n", name);
                exit(EXIT_FAILURE);
            }
        }
        if (lines_mem_used + 1 >= lines_mem_size) {
            lines_mem_size *= 2;
            lines = (char **) realloc(lines, lines_mem_size * sizeof(char *));
            if (lines == NULL) {
                fprintf(stderr, "ERROR: Could not allocate memory when reading shader source %s.\n", name);
                exit(EXIT_FAILURE);
            }
        }
        strncpy(&shader_source[mem_used], line, SHADER_SOURCE_LINE_MAX_LENGTH);
#if TRACING
        printf("SOURCE: %s", shader_source);
#endif
        lines[lines_mem_used] = &shader_source[mem_used];

        mem_used += len;
        shader_source[mem_used] = '\0';
        mem_used += 1;
        lines_mem_used += 1;

#if TRACING
        for (int i = 0; i < lines_mem_used; i++) {
            printf("lines LINE %d: %s\n", i, lines[i]);
        }
#endif
    }

    if (num_lines == 0) {
        // ... only to make sure free(lines[0]) can free the source string on the callers side.
        fprintf(stderr, "ERROR: empty shader file %s\n", name);
        exit(EXIT_FAILURE);
    }

    // Write return values
    *lines_out = lines;
    *num_lines = lines_mem_used;
#undef TRACING
#undef SHADER_SOURCE_LINE_MAX_LENGTH
#undef MEM_SIZE_START
#undef LINES_MEM_SIZE_START
}


void load_and_compile_shader(GLuint shader, char *shader_path)
{
    //=========================================================================================
    // glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length)
    //=========================================================================================
    // Get the source and associate it to the shader ----------------------------------
    char **lines;
    size_t num_lines;
    read_shader_source(shader_path, &lines, &num_lines);
    glShaderSource(shader, num_lines, (const GLchar * const*) lines, NULL);
    free(lines);
    free(lines[0]);
    //---------------------------------------------------------------------------------
    // Compile the shader and print error logs if needed ------------------------------
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
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
    //---------------------------------------------------------------------------------
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


int main(int argc, char *argv[])
{
    GLFWwindow *window;
    int horiz = 512;
    int vert = 512;
    char *name = "Polygon editor";
    //--------------------------------------------------------------------------------
    if (!glfwInit()) {
        fprintf(stderr, "GLFW error: something went wrong initializing GLFW\n");
        exit(EXIT_FAILURE);
    }

    /* glmeta
     * Use a VBO to render a single, 2D flat white triangle to the screen. OpenGL with the compatibility profile
     * will render this without needing shaders or matrix operations. It will fallback to fixed function for the rendering
     * and provide you with a viewport with -1.0 to 1.0 for the x & y coordinates. Z coordinates will be ignored.
     * ---
     * No default pass-through white vertex and fragment shaders (""?) with the core profile.
     */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(horiz, vert, name, NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "GLFW error: failed to create a window properly\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glfwSwapInterval(1);

    // Vertex attributes -------------------------------------------------------------
    const int vPosition = 0;
    //--------------------------------------------------------------------------------
    // Shaders
    //--------------------------------------------------------------------------------
    char *vertex_shader_path = APPLICATION_LOCATION "shader.vert";
    char *fragment_shader_path = APPLICATION_LOCATION "shader.frag";
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    load_and_compile_shader(vertex_shader, vertex_shader_path);
    load_and_compile_shader(fragment_shader, fragment_shader_path);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
        // Attachments
        glBindAttribLocation(shader_program, vPosition, "vPosition");
    link_shader_program(shader_program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glUseProgram(shader_program);
    //--------------------------------------------------------------------------------
    // Vertex data and specification -------------------------------------------------

    float triangle_data[3 * 2] = {
        -0.5, 0.5,
        -0.5, -0.5,
        0.5, -0.5
    };
    // global GLuint triangle_vao;
    glGenVertexArrays(1, &triangle_vao);
    glBindVertexArray(triangle_vao);

    GLuint triangle_vbo;
    glGenBuffers(1, &triangle_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_data), triangle_data, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(vPosition);
    //--------------------------------------------------------------------------------
    ASPECT_RATIO = SCREEN_ASPECT_RATIO;

    glClearColor(0.0, 0.0, 0.0, 1.0);

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
