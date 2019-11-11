/*
 * Started as testing for the entity system. It works relatively well enough now to start on a collision system.
 * ------ important note: I think this all segfaults if the order of system additions is changed. Really fix this.
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

//================================================================================
// Game and game objects logic
//================================================================================
void polygon_update(Entity *self)
{
    Transform *transform = get_entity_component_of_type(self->id, Transform);
    if (alt_arrow_key_down(Up)) {
        transform->y += 1.0 * dt();
    }
    if (alt_arrow_key_down(Down)) {
        transform->y -= 1.0 * dt();
    }
    if (alt_arrow_key_down(Right)) {
        transform->x += 1.0 * dt();
    }
    if (alt_arrow_key_down(Left)) {
        transform->x -= 1.0 * dt();
    }
    transform->theta += 5.0 * transform->x * dt();
}

/* hacky thing to test the entity tree system
int count = UNIVERSE_ID;
...
    double r = frand();
    EntityID parent_id;
    for (parent_id = UNIVERSE_ID; parent_id <= count; parent_id++) {
        r -= 1.0 / count;
        if (r <= 0) {
            break;
        }
    }
    EntityID polygon = create_entity(parent_id, ascii_name);
...
    count++;
*/
void game_make_polygon(char *ascii_name, double x, double y, double theta)
{
    EntityID polygon = create_entity(UNIVERSE_ID, ascii_name);
    ObjectLogic *logic = entity_add_component_get(polygon, "logic", ObjectLogic);
    logic->update = polygon_update;
    RendererShape *renderer = entity_add_component_get(polygon, "renderer", RendererShape);
    ascii_polygon(ascii_name, &renderer->poly);
    Transform *transform = entity_add_component_get(polygon, "transform", Transform);
    transform->x = x;
    transform->y = y;
    transform->theta = theta;
}


// Systems
#define DEBUG 0
void renderer_update(System *self)
{
    Iterator iterator;
    iterator_components_of_type(RendererShape, &iterator);
#if DEBUG
    int count = 0;
    printf("ITERATING RENDERERS\n");
#endif
    while (1) {
        step(&iterator);
        if (iterator.val == NULL) {
            break;
        }
        RendererShape *shape = (RendererShape *) iterator.val;
        /* printf("%s\n", shape->component.name); */
        Transform *transform = get_entity_component_of_type(shape->component.entity_id, Transform);
        /* printf("(%.2lf %.2lf : %.2lf)\n", transform->x, transform->y, transform->theta); */

        glBegin(GL_POLYGON);
            for (int i = 0; i < shape->poly.num_vertices; i++) {
                double x, y;
                x =  transform->x
                     + cos(transform->theta) * shape->poly.vertices[i].x
                     + sin(transform->theta) * shape->poly.vertices[i].y;
                y = transform->y
                    - cos(transform->theta) * shape->poly.vertices[i].y
                    + sin(transform->theta) * shape->poly.vertices[i].x;

                glVertex2f(x, y);
            }
        glEnd();
#if DEBUG
        count ++;
#endif
    } 
#if DEBUG
    printf("rendered %d entities\n", count);
#endif
}
#undef DEBUG

#define DEBUG 0
void object_logic_update(System *self)
{
    Iterator iterator;
    iterator_components_of_type(ObjectLogic, &iterator);
#if DEBUG
    printf("ITERATING OBJECT LOGIC\n");
#endif
    while (1) {
        step(&iterator);
        if (iterator.val == NULL) {
            break;
        }
        ObjectLogic *logic = (ObjectLogic *) iterator.val;
        Entity *entity = get_entity(logic->component.entity_id);
#if DEBUG
        printf("logic update: %s\n", entity->name);
#endif
        if (logic->update != NULL) {
#if DEBUG
            printf("\tupdating ...\n");
#endif
            logic->update(entity);
        }
    }
}
#undef DEBUG

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
        else if (key == GLFW_KEY_SPACE) {
            char poly_name[50];
            double r = frand();
            int i = 1;
            for (; i <= 7; i++) {
                r -= 1.0/7.0;
                if (r <= 0) {
                    break;
                }
            }
            sprintf(poly_name, "%d", i);
            strncat(poly_name, ".poly", 50);
            game_make_polygon(poly_name, frand() * 0.5 - 0.25, frand() * 0.5 - 0.25, frand() * 2 * M_PI);
        }
    }
}

void init_program(void)
{
    init_entity_model();

    add_system("renderer", NULL, renderer_update, NULL);
    add_system("object logic", NULL, object_logic_update, NULL);
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
