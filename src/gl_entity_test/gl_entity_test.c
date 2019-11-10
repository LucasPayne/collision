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
#include "helper_definitions.h"
#include "entity.h"
#include "shapes.h"
#include "data.h"

//==== ENTITIES ==================================================================

void circling_triangle_entity_init(Entity2D *self)
{
}
void circling_triangle_entity_update(Entity2D *self)
{
    self->transform.rotation += 1.0 * dt();
    Point2f triangle_location;
    triangle_location.x = 1.0;
    triangle_location.y = 0.0;
    Point2f location = point2f_transform_to_entity(triangle_location, self);
    glBegin(GL_TRIANGLES);
        glVertex2f(location.x, location.y);
        glVertex2f(location.x + 0.3, location.y + 0.3);
        glVertex2f(location.x - 0.3, location.y + 0.3);
    glEnd();
}

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
    if (poly != NULL) {
        glBegin(GL_POLYGON);
        for (int i = 0; i < poly->num_vertices; i++) {
            Point2f point = point2f_transform_to_entity(poly->vertices[i], self);
            glVertex2f(point.x, point.y);
        }
        glEnd();
    }
}

struct solid_polygon_2_properties_s {
    Polygon polygon;
    char test_char;
};
void solid_polygon_2_entity_init(Entity2D *self)
{
    init_entity_data(self, solid_polygon);
    get_entity_data(data, self, solid_polygon);
    Polygon poly;
    ascii_polygon("3.poly", &poly);
    for (int i = 0; i < poly.num_vertices; i++) {
        poly.vertices[i].x *= 0.05;
        poly.vertices[i].y *= 0.05;
    }
    data->polygon = poly;
    data->test_char = 'b';
}
void solid_polygon_2_entity_update(Entity2D *self)
{
    get_entity_data(data, self, solid_polygon);
    Polygon *poly = &data->polygon;

    if (alt_arrow_key_down(Up)) {
        self->transform.position.y += 0.8 * dt();
    }
    if (alt_arrow_key_down(Down)) {
        self->transform.position.y -= 0.8 * dt();
    }
    if (alt_arrow_key_down(Right)) {
        self->transform.position.x += 0.8 * dt();
    }
    if (alt_arrow_key_down(Left)) {
        self->transform.position.x -= 0.8 * dt();
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

    Entity2D *poly = create_entity(NULL, "poly", solid_polygon, -0.8, 0, 0);
    poly->init(poly);
    for (int i = 0; i < 10; i++) {
        poly = create_entity(poly, "poly", solid_polygon, frand() * 0.1 - 0.05, frand() * 0.1 - 0.05, 0);
        poly->init(poly);
        create_entity(poly, "triangle", circling_triangle, 0, 0, 0);
    }
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
