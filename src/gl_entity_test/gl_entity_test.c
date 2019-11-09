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
#include "data.h"

//==== ENTITIES ==================================================================


struct solid_polygon_properties_s {
    Polygon polygon;
    char test_char;
};
void solid_polygon_entity_init(Entity2D *self)
{
    init_entity_data(self, solid_polygon);
    get_entity_data(data, self, solid_polygon);

    
    Polygon poly;
    ascii_polygon("2.poly", &poly);
    for (int i = 0; i < poly.num_vertices; i++) {
        poly.vertices[i].x *= 0.05;
        poly.vertices[i].y *= 0.05;
    }
    data->polygon = poly;

    data->test_char = 'b';
}
void solid_polygon_entity_update(Entity2D *self)
{
    get_entity_data(data, self, solid_polygon);
    Polygon *poly = &data->polygon;

    if (arrow_key_down(Up)) {
        self->transform.position.y += 0.8 * dt();
    }
    if (arrow_key_down(Down)) {
        self->transform.position.y -= 0.8 * dt();
    }
    if (arrow_key_down(Right)) {
        self->transform.position.x += 0.8 * dt();
    }
    if (arrow_key_down(Left)) {
        self->transform.position.x -= 0.8 * dt();
    }
    if (alt_arrow_key_down(Left)) {
        self->transform.rotation -= 1.5 * dt();
    }
    if (alt_arrow_key_down(Right)) {
        self->transform.rotation += 1.5 * dt();
    }

    if (poly != NULL) {
        glBegin(GL_POLYGON);
        for (int i = 0; i < poly->num_vertices; i++) {
            Point2f point = point2f_transform_to_entity(poly->vertices[i], self);
            glVertex2f(point.x, point.y);
        }
        glEnd();
    }
}

//================================================================================

static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_P) {
            print_entity_tree(NULL);
        }
    }
}

void game_init(void)
{
    init_entity_model();
    Entity2D *poly1 = create_entity(NULL, "poly1", solid_polygon, 0, 0, 0);
    poly1->init(poly1);

    Entity2D *poly2 = create_entity(NULL, "poly2", solid_polygon, 0, 0, 0);
    poly2->init(poly2);
    poly2->transform.position.x = -0.3;
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
