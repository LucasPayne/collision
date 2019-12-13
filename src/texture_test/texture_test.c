/*================================================================================
PROJECT_LIBS:
    + glad
    + helper_gl
    + helper_input
    + dictionary
    + resources
    + rendering
    + ply
================================================================================*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "helper_definitions.h"
#include "helper_gl.h"
#include "helper_input.h"
#include "ply.h"
#include "rendering.h"
#include "resources.h"


// GL stuff and loop
//================================================================================

GLuint g_program_id;
GLuint g_vao_id;
GLuint g_texture_id;

ResourceHandle g_graphics_program;

void init_program(void)
{
    const int diffuse_map_binding_point = GL_TEXTURE0;

    init_resources_rendering();
    resource_path_add("Textures", "/home/lucas/code/collision/src/texture_test/textures");
    resource_path_add("Shaders", "/home/lucas/code/collision/src/texture_test/shaders");


#if 1
    g_graphics_program = new_resource_handle(GraphicsProgram, "Shaders/texturing");
    GraphicsProgram *prog = resource_data(GraphicsProgram, g_graphics_program);
#if 1
    GLint diffuse_map_location = glGetUniformLocation(prog->program_id, "diffuse_map");
    if (diffuse_map_location < 0) {
        fprintf(stderr, "!!! Something went wrong in texture test.\n");
        exit(EXIT_FAILURE);
    }
    glUseProgram(prog->program_id);
    glUniform1i(diffuse_map_location, 0); // or GL_TEXTURE0?
#endif

    /* ResourceHandle texture = new_resource_handle(Texture, "Textures/felix_klein"); */
    /* Texture *tex = resource_data(Texture, texture); */
    /* glActiveTexture(GL_TEXTURE0); */
    /* glBindTexture(GL_TEXTURE_2D, tex->texture_id); */
    /* printf("Texture ID: %u\n", tex->texture_id); */

    static const GLubyte checkerboard_data[] = {
        0xFF, 0x00, 0xFF,
        0x00, 0xFF, 0x00,
        0xFF, 0x00, 0xFF,
    };
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, 3, 3);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,    // First mipmap level
                    0, 0, // x and y offset
                    3, 3,
                    GL_RED, GL_UNSIGNED_BYTE,
                    checkerboard_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR works
    /* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); */

    float rectangle[2 * 4] = {
        -0.95, -0.95,
        0.95, -0.95,
        0.95, 0.95,
        -0.95, 0.95,
    };
    float rectangle_tex_coords[2 * 4] = {
        0, 0,
        1, 0,
        0, 1,
        1, 1,
    };
    GLuint vbos[2];
    glGenBuffers(2, vbos);
    printf("vbos: %u, %u\n", vbos[0], vbos[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2 * 4, (void *) rectangle, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2 * 4, (void *) rectangle_tex_coords, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    printf("vao: %u\n", vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glVertexAttribPointer(0/*position*/, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
    glVertexAttribPointer(3/*texture coordinates*/, 2/*vec2*/, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(3);

    g_program_id = prog->program_id;
    /* g_texture_id = tex->texture_id; */
    g_vao_id = vao;
#endif
}
void loop(void)
{
    // Make sure the resource handle is dereferenced each time, because stuff can change in the background.
    
    glUseProgram(resource_data(GraphicsProgram, g_graphics_program)->program_id);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glPointSize(20);
    glDrawArrays(GL_POINTS, 0, 4);
}
void close_program(void)
{
}
static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
#define CASE(ACTION,KEY)\
    if (action == ( GLFW_ ## ACTION ) && key == ( GLFW_KEY_ ## KEY ))
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);

    CASE(PRESS, C) {
        GraphicsProgram_reload(g_graphics_program);
    }
#undef CASE
}

static float ASPECT_RATIO;
void reshape(GLFWwindow* window, int width, int height)
{
    force_aspect_ratio(window, width, height, ASPECT_RATIO);
}
int main(int argc, char *argv[])
{
    GLFWwindow *window;
    char *name = "Texture testing";
    //--------------------------------------------------------------------------------
    if (!glfwInit()) {
        fprintf(stderr, "GLFW error: something went wrong initializing GLFW\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1, 1, name, NULL, NULL);
    if (!window) {
        fprintf(stderr, "GLFW error: failed to create a window properly.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);
    ASPECT_RATIO = SCREEN_ASPECT_RATIO;

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop, GL_COLOR_BUFFER_BIT);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
