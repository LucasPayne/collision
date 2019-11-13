/*================================================================================
   Schematic for graphical projects using the entity model
PROJECT_LIBS:
    - glad
    - helper_gl
    - helper_input
    - entity
    - iterator
================================================================================*/

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

// Got this just from checking, --- get the actual ratio
static double ASPECT_RATIO;

static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);
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
    update_entity_model();
}
void close_program(void)
{
    close_entity_model();
}

void reshape(GLFWwindow* window, int width, int height)
{
    force_aspect_ratio(window, width, height, ASPECT_RATIO);
}

int main(int argc, char *argv[])
{
    GLFWwindow *window = init_glfw_create_context("Shapes", 512, 512);

    ASPECT_RATIO = SCREEN_ASPECT_RATIO;

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop);
    close_program();

    exit(EXIT_SUCCESS);
}
