/*
 * Quick schematic (may be out of date later)
 */

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
#include "shapes.h"

static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);
}

void game_init(void)
{
    init_entity_model();
}
void loop(GLFWwindow *window)
{
    update_entity_model();
}
void game_cleanup(void)
{
    close_entity_model();
}

void reshape(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, (GLsizei) width, (GLsizei) height);
}

int main(int argc, char *argv[])
{
    GLFWwindow *window = init_glfw_create_context("Shapes", 512, 512);
    glfwSetWindowAspectRatio(window, 1, 1);

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    game_init();
    loop_time(window, loop);
    game_cleanup();
    
    exit(EXIT_SUCCESS);
}
