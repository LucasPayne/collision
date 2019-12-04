/*================================================================================
PROJECT_LIBS:
    + glad
    + helper_gl
    + helper_input
    + entity
    + iterator
    + mesh
    + mesh_gen
================================================================================*/
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
#include "entity.h"
#include "iterator.h"
#include "mesh.h"
#include "mesh_gen.h"

// Globals for testing -----------------------------------------------------------
static double ASPECT_RATIO;
static Renderer g_renderer;

static Matrix4x4f g_view_matrix;
static Matrix4x4f g_projection_matrix;

static Matrix4x4f g_mvp_matrix;
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
}

static UniformData get_uniform_value_aspect_ratio(void)
{
    UniformData data;
    data.float_value = ASPECT_RATIO;
    return data;
}
static UniformData get_uniform_value_mvp_matrix(void)
{
    UniformData data;
    memcpy(data.mat4x4_value, &g_mvp_matrix.vals, 4 * 4 * sizeof(float));
    return data;
}


void init_program(void)
{
    init_vertex_formats();
    init_entity_model();

    new_renderer_vertex_fragment(&g_renderer, SHADERS_LOCATION "shader.vert", SHADERS_LOCATION "shader.frag");
    renderer_add_uniform(&g_renderer, "aspect_ratio", get_uniform_value_aspect_ratio, UNIFORM_FLOAT);
    renderer_add_uniform(&g_renderer, "mvp_matrix", get_uniform_value_mvp_matrix, UNIFORM_MAT4X4);

    /* printf */

    Mesh mesh;
    create_cube_mesh(&mesh, 1.0);
    MeshHandle mesh_handle;
    upload_mesh(&mesh_handle, &mesh);
    /* print_mesh(&mesh); */
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

    ASPECT_RATIO = SCREEN_ASPECT_RATIO;
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop, GL_COLOR_BUFFER_BIT);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
