/* PROJECT_LIBS:
 *      + glad
 *      + helper_gl
 *      + helper_input
 *      + matrix_mathematics
 *      + mesh
 */
#define SHADERS_LOCATION "/home/lucas/code/collision/src/sphere/shaders/"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "helper_definitions.h"
#include "helper_gl.h"
#include "helper_input.h"
#include "matrix_mathematics.h"
#include "mesh.h"

// Globals for testing -----------------------------------------------------------

static Renderer basic_renderer;

static double ASPECT_RATIO;
GLuint sphere_vao;

GLuint shader_program;

Matrix4x4f sphere_matrix;
GLuint uniform_location_model_matrix;
GLuint uniform_location_aspect_ratio;
GLuint uniform_location_time;

MeshHandle sphere_mesh_handle;

float sphere_radius = 0.0;
int sphere_tesselation = 1; 

//--------------------------------------------------------------------------------

#include "include/sphere.h"
// file separation
#include "input/key_callback.c"
#include "input/mouse_button_callback.c"
#include "window/reshape.c"
#include "init_program.c"
#include "close_program.c"
#include "loop.c"

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

    ASPECT_RATIO = SCREEN_ASPECT_RATIO;
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop, GL_COLOR_BUFFER_BIT);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
