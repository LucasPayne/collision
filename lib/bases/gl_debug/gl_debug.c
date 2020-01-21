/*--------------------------------------------------------------------------------
base_libs:
    + glad
    + helper_gl
    + helper_input
    + glsl_utilities
    + matrix_mathematics
    + data_dictionary
    - entity
    - aspect_library/gameobjects
    - iterator
    - resources
    - rendering
    - ply
--------------------------------------------------------------------------------*/
#define BASE_DIRECTORY "/home/lucas/collision/lib/bases/gl_debug/"
#include "bases/gl_debug.h"

float time = 0;
float dt = 0;

extern void init_program(void);
extern void loop_program(void);
extern void close_program(void);

static void init_base(void)
{
    printf("gl_debug: Initialized.\n");
}
static void loop_base(void)
{
    loop_program();
}

int main(void)
{
    if (!glfwInit()) {
        fprintf(stderr, "GLFW error: something went wrong initializing GLFW\n");
        exit(EXIT_FAILURE);
    }
    int gl_version[2] = { 4, 2 };
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_version[0]);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_version[1]);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    char *name = "gl_debug";
    GLFWwindow *window = glfwCreateWindow(1, 1, name, NULL, NULL);
    if (!window) {
        fprintf(stderr, ERROR_ALERT "GLFW error: failed to create a window properly.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);

    float clear_color[4] = { 1, 0, 1, 1 };
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

        glClear(GL_COLOR_BUFFER_BIT);
        loop_base();

        glFlush();
        glfwSwapBuffers(window);
    }
    // Cleanup
    close_program();
    glfwDestroyWindow(window);
    glfwTerminate();
}
