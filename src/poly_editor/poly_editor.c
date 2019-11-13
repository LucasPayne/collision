/* PROJECT_LIBS:
 *      - glad
 *      - helper_gl
 *      - helper_input
 *      - entity
 *      - iterator
 *      - components/Transform2D
 * A polygon editor
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
#include "components/Transform2D.h"
#undef Transform2D_TYPE_ID
#define Transform2D_TYPE_ID 1


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
    glBegin(GL_POLYGON);
    glVertex2f(-1, -1);
    glVertex2f(1, -1);
    glVertex2f(1, 1);
    glVertex2f(-1, 1);
    glEnd();
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

int main(int argc, char *argv[])
{
    GLFWwindow *window = init_glfw_create_context("Polygon editor", 512, 512);
    ASPECT_RATIO = SCREEN_ASPECT_RATIO;

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
