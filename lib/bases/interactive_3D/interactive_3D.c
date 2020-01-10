/*--------------------------------------------------------------------------------
Application base.
Add this as a library in an application, and it will provide a main function.
The application must provide:
    conf.dd: A file containing application configuration, to be included as app_config. This is written to the ApplicationConfiguration type declared
             in this base's .dd file.

base_libs:
    + glad
    + helper_gl
    + helper_input
    + data_dictionary
    + matrix_mathematics
    + entity
    - aspect_library/gameobjects
    + iterator
    - resources
    + rendering
    - ply
--------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "helper_definitions.h"
#include "helper_gl.h"
#include "helper_input.h"
#include "data_dictionary.h"
/* #include "ply.h" */
#include "rendering.h"
#include "resources.h"
#include "entity.h"
#include "matrix_mathematics.h"
#include "aspect_library/gameobjects.h"

typedef Matrix4x4f mat4x4;
#include "shader_blocks/Standard3D.h"
#include "shader_blocks/StandardLoopWindow.h"
#include "shader_blocks/Lights.h"

#define BASE_DIRECTORY "/home/lucas/code/collision/lib/bases/interactive_3D/"

float time = 0;
float dt = 0;

extern void init_program(void);
extern void loop_program(void);
extern void close_program(void);

#define config_error(str)\
    { fprintf(stderr, ERROR_ALERT "Application configuration error: non-existent or malformed \"" str "\" entry.\n");\
      exit(EXIT_FAILURE); }

static void init_base(void)
{
    /* Non-window initialization. */
    // Create the basic shader blocks.
    add_shader_block(MaterialProperties); // The definition of this block is part of the rendering module.
    add_shader_block(StandardLoopWindow);
    add_shader_block(Standard3D);
    // add_shader_block(Lights);
    
    // Initialize the entity and aspect system, and load type data for aspects
    // forming the "GameObject" library.
    /* init_entity_model(); */
    /* init_aspects_gameobjects(); */
}

int main(void)
{
    DD *base_config = dd_fopen(BASE_DIRECTORY "interactive_3D.dd");
    DD *app_config = dd_open(base_config, "app_config");
    printf("Configuration\n");
    dd_print(app_config);

    if (!glfwInit()) {
        fprintf(stderr, "GLFW error: something went wrong initializing GLFW\n");
        exit(EXIT_FAILURE);
    }
    int gl_version[2];
    if (!dd_get(app_config, "gl_version", "ivec2", gl_version)) config_error("gl_version");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_version[0]);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_version[1]);
    // GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_COMPAT_PROFILE or GLFW_OPENGL_CORE_PROFILE
    bool core_profile;
    if (!dd_get(app_config, "core_profile", "bool", &core_profile)) config_error("core_profile");
    if (core_profile) glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    else              glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    char *name = "Window";
    GLFWwindow *window = glfwCreateWindow(1, 1, name, NULL, NULL);
    if (!window) {
        fprintf(stderr, ERROR_ALERT "GLFW error: failed to create a window properly.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);

    float clear_color[4];
    if (!dd_get(app_config, "clear_color", "vec4", clear_color)) config_error("clear_color");
    glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);

    init_base();

    init_program();
    double last_time = time;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        last_time = time;
        time = glfwGetTime();
        dt = time - last_time;

        /* Clearing: window clear to black, viewport clear to the clear colour.
         * (restore clear colour after window clear)
         */
        glClear(GL_COLOR_BUFFER_BIT);
        /* GLfloat clear_color[4]; */
        /* glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color); */
        /* glClearColor(0.0, 0.0, 0.0, 1.0); */
        /* glDisable(GL_SCISSOR_TEST); */
        /* glClear(GL_COLOR_BUFFER_BIT); // config: clear mask */

        /* GLint viewport[4]; */
        /* glGetIntegerv(GL_VIEWPORT, viewport); */
        /* glEnable(GL_SCISSOR_TEST); */
        /* glScissor(viewport[0], viewport[1], viewport[2], viewport[3]); */
        /* glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]); */
        /* glClear(GL_COLOR_BUFFER_BIT); // config: clear mask */

        loop_program();

        glFlush();
        glfwSwapBuffers(window);
    }
    // Cleanup
    close_program();
    glfwDestroyWindow(window);
    glfwTerminate();
}

#if 0
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

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop, GL_COLOR_BUFFER_BIT);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
#endif
