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

#include "helper_definitions.h"
#include "helper_gl.h"
#include "helper_input.h"
#include "data.h"
#include "shapes.h"
#include "entity.h"

// Component types
#define RendererShape_TYPE_ID 2
typedef struct RendererShape_component_type_s {
    Component component;
    Polygon poly;
} RendererShape;

#define ObjectLogic_TYPE_ID 3
typedef struct ObjectLogic_component_type_s {
    Component component;
    void (*init) (Entity *);
    void (*update) (Entity *);
    void (*close) (Entity *);
} ObjectLogic;

#define Transform_TYPE_ID 4
typedef struct Transform_component_type_s {
    Component component;
    double x;
    double y;
    double theta;
} Transform;

// Systems
void renderer_update(System *self)
{
    Iterator iterator;
    iterator_components_of_type(RendererShape, &iterator);
    while (1)
    {
        step(&iterator);
        if (iterator.val == NULL) {
            break;
        }
        RendererShape *shape = (RendererShape *) iterator.val;
        Transform *transform = get_entity_component(Entity *entity, ComponentType component_type);

        glBegin(GL_POLYGON);
            for (int i = 0; i < shape->poly.num_vertices; i++) {
                glVertex2f(shape->poly.vertices[i].x * 0.05, shape->poly.vertices[i].y * 0.05);
            }
        glEnd();
    } 
}
void object_logic_update(System *self)
{
    // for ObjectLogic entity entity ...
        /* if (entity->update != NULL) { */
        /*     entity->update(entity); */
        /* } */
}

static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_P) {
            print_entity_tree();
        }
    }
}

void init_program(void)
{
    init_entity_model();

    add_system("renderer", NULL, renderer_update, NULL);

    EntityID polygon = create_entity(UNIVERSE_ID, "polygon");
    Polygon poly;
    ascii_polygon("2.poly", &poly);
    RendererShape *renderer = entity_add_component_get(polygon, "renderer", RendererShape);
    renderer->poly = poly;
    Transform *transform = entity_add_component_get(polygon, "transform", Transform);
    ObjectLogic *logic = entity_add_component_get(polygon, "logic", ObjectLogic);

    /* logic->test_char = 's'; */
    /* printf("%s\n", logic->component.name); */
    /* printf("%c\n", logic->test_char); */
    /* printf("%s\n", renderer->component.name); */
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
    glViewport(0, 0, (GLsizei) width, (GLsizei) height);
}

int main(int argc, char *argv[])
{
    GLFWwindow *window = init_glfw_create_context("Shapes", 512, 512);
    glfwSetWindowAspectRatio(window, 1, 1);

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop);
    close_program();
    
    exit(EXIT_SUCCESS);
}
