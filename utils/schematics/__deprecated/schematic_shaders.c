/* PROJECT_LIBS:
 *      + glad
 *      + helper_gl
 *      + helper_input
 * 
 * CREATED FROM SHADERS SCHEMATIC
 */
#define SHADERS_LOCATION "/home/lucas/code/collision/src/shader_test/"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "helper_gl.h"
#include "helper_input.h"

// Globals for testing -----------------------------------------------------------
DynamicShaderProgram dynamic_shader_program;
static double ASPECT_RATIO;
//--------------------------------------------------------------------------------

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
    if (action == GLFW_PRESS && key == GLFW_KEY_C) {
        recompile_shader_program(&dynamic_shader_program);
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{

}

void init_program(void)
{
}
void loop(GLFWwindow *window)
{
}
void close_program(void)
{
}

void reshape(GLFWwindow* window, int width, int height)
{
    force_aspect_ratio(window, width, height, ASPECT_RATIO);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    printf("x:%d, y: %d, width: %d, height: %d\n", viewport[0], viewport[1], viewport[2], viewport[3]);
}

int main(int argc, char *argv[])
{
    GLFWwindow *window;
    int horiz = 512;
    int vert = 512;
    char *name = "Shader testing";
    //--------------------------------------------------------------------------------
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
    gladLoadGL();
    glfwSwapInterval(1);

    // Shaders
    //--------------------------------------------------------------------------------
    char *vertex_shader_path = SHADERS_LOCATION "shader.vert";
    char *fragment_shader_path = SHADERS_LOCATION "shader.frag";
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    load_and_compile_shader(vertex_shader, vertex_shader_path);
    load_and_compile_shader(fragment_shader, fragment_shader_path);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    link_shader_program(shader_program);
    glUseProgram(shader_program);

    // Initialize dynamic shader program for run-time recompilation
    // global DynamicShaderProgram dynamic_shader_program;
    dynamic_shader_program.id = shader_program;
    dynamic_shader_program.vertex_shader_id = vertex_shader;
    dynamic_shader_program.fragment_shader_id = fragment_shader;
    if (strlen(vertex_shader_path) > MAX_DYNAMIC_SHADER_PROGRAM_PATH_LENGTH ||
            strlen(fragment_shader_path) > MAX_DYNAMIC_SHADER_PROGRAM_PATH_LENGTH) {
        fprintf(stderr, "ERROR: path name for shader too long.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(dynamic_shader_program.vertex_shader_path, vertex_shader_path);
    strcpy(dynamic_shader_program.fragment_shader_path, fragment_shader_path);
    print_dynamic_shader_program(&dynamic_shader_program);

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
