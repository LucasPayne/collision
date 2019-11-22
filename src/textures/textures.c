/* PROJECT_LIBS:
 *      + glad
 *      + helper_gl
 *      + helper_input
 * 
 * CREATED FROM SHADERS SCHEMATIC
 */
#define SHADERS_LOCATION "/home/lucas/code/collision/src/textures/"

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

static GLint uniform_location_time;

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
    float texture_data[4*4 * 4] = {
        0, 0, 1, 1,   0, 1, 0, 1,   0, 0, 0, 1,   1, 1, 0, 1,
        1, 0, 1, 1,   0, 1, 0, 1,   0, 1, 0, 1,   1, 1, 0, 1,
        0, 0, 1, 1,   0, 0, 0, 1,   0, 0, 0, 1,   0, 1, 0, 1,
        0, 0, 1, 1,   0, 1, 1, 1,   0, 1, 0, 1,   1, 1, 0, 1,
    };
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexStorage2D(GL_TEXTURE_2D, // target
                               1, // mipmap levels
                               GL_RGB16, // internal format
                               4, 4); // width and height

    glTexSubImage2D(GL_TEXTURE_2D, // target
                                0, // first mipmap level
                                0, 0, // x and y offsets
                                4, 4, // width and height
                                GL_RGB, // external format
                                GL_FLOAT, // type used in external format
                                texture_data); // application data pointer
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Can use default sampler object associated (in) texture.
    /* GLuint sampler; */
    /* glGenSamplers(1, &sampler); */


    GLuint vertex_data_buffer;
    glGenBuffers(1, &vertex_data_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_data_buffer);
    float vertex_data[4*2 * 2] = {
        // Positions
        -0.5, -0.5,
        0.5, -0.5,
        0.5, 0.5,
        -0.5, 0.5,
        // Texture coordinates
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_DYNAMIC_DRAW);

    GLuint vao;
    const int location_vPosition = 0;
    const int location_texture_coord = 1;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(location_vPosition, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(location_vPosition);
    glVertexAttribPointer(location_texture_coord, 2, GL_FLOAT, GL_FALSE, 0, (void *) (sizeof(float) * 8));
    glEnableVertexAttribArray(location_texture_coord);
}

void loop(GLFWwindow *window)
{
    glUniform1f(uniform_location_time, time());
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
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
    char *vertex_shader_path = SHADERS_LOCATION "texture.vert";
    char *fragment_shader_path = SHADERS_LOCATION "texture.frag";
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    load_and_compile_shader(vertex_shader, vertex_shader_path);
    load_and_compile_shader(fragment_shader, fragment_shader_path);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    link_shader_program(shader_program);
    glUseProgram(shader_program);

    // Get uniforms
    //--------------------------------------------------------------------------------
    uniform_location_time = glGetUniformLocation(shader_program, "time");

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
