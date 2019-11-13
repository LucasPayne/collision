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
#include "grid.h"

// Globals =======================================================================
static double ASPECT_RATIO;
static bool GRID_RENDERING = true;
static GLFWwindow *WINDOW;
static int MAX_GRID_SIZE;
static int GRID_SIZE;
//================================================================================

// Component types
#define RendererShape_TYPE_ID 2
typedef struct RendererShape_s {
    Component component;
    Polygon poly;
} RendererShape;

#define ObjectLogic_TYPE_ID 3
typedef struct ObjectLogic_s {
    Component component;
    void (*init) (Entity *);
    void (*update) (Entity *);
    void (*close) (Entity *);
} ObjectLogic;

#define Transform_TYPE_ID 4
typedef struct Transform_s {
    Component component;
    double x;
    double y;
    double theta;
    double scale_x;
    double scale_y;
} Transform;
static void _get_global_transform(Transform *transform, Transform *to);
void get_global_transform(Transform *transform, Transform *to)
{
    to->x = 0;
    to->y = 0;
    to->theta = 0;
    to->scale_x = 1;
    to->scale_y = 1;
    _get_global_transform(transform, to);
}
static void _get_global_transform(Transform *transform, Transform *to)
{
    to->x += transform->x;
    to->y += transform->x;
    to->theta += transform->theta;
    to->scale_x *= transform->scale_x;
    to->scale_y *= transform->scale_y;

    Entity *entity = get_entity(transform->component.entity_id);
    Entity *parent = get_entity(entity->parent_id);
    Transform *parent_transform = get_entity_component_of_type(parent->id, Transform);
    if (parent_transform != NULL) {
        _get_global_transform(parent_transform, to);
    }
}

#define Camera_TYPE_ID 5
typedef struct Camera_s {
    Component component;
    double width;
    double height;
    // Enforce Transform?
} Camera;



//================================================================================
// Game and game objects logic
//================================================================================
void polygon_update(Entity *self)
{
    Transform *transform = get_entity_component_of_type(self->id, Transform);
    /* if (alt_arrow_key_down(Up)) { */
    /*     transform->y += 1.0 * dt(); */
    /* } */
    /* if (alt_arrow_key_down(Down)) { */
    /*     transform->y -= 1.0 * dt(); */
    /* } */
    /* if (alt_arrow_key_down(Right)) { */
    /*     transform->x += 1.0 * dt(); */
    /* } */
    /* if (alt_arrow_key_down(Left)) { */
    /*     transform->x -= 1.0 * dt(); */
    /* } */
    /* transform->theta += 5.0 * transform->x * dt(); */

    transform->theta += 1.0 * dt();
}

int LAST_ID = 0;
void game_make_polygon(char *ascii_name, double x, double y, double theta)
{
    EntityID parent_id;
    if (LAST_ID == 0)
        parent_id = UNIVERSE_ID;
    else
        parent_id = LAST_ID;

    EntityID polygon = create_entity(parent_id, ascii_name);
    ObjectLogic *logic = entity_add_component_get(polygon, "logic", ObjectLogic);
    logic->update = polygon_update;
    RendererShape *renderer = entity_add_component_get(polygon, "renderer", RendererShape);
    ascii_polygon(ascii_name, &renderer->poly);
    Transform *transform = entity_add_component_get(polygon, "transform", Transform);
    transform->x = x;
    transform->y = y;
    transform->scale_x = 0.8;
    transform->scale_y = 0.8;
    transform->theta = theta;

    LAST_ID = polygon;
}

void camera_controls(Entity *self)
{
    Transform *transform = get_entity_component_of_type(self->id, Transform);

    if (alt_arrow_key_down(Up)) {
        transform->y += 1.2 * dt();
    }
    if (alt_arrow_key_down(Down)) {
        transform->y -= 1.2 * dt();
    }
    if (alt_arrow_key_down(Right)) {
        transform->x += 1.2 * dt();
    }
    if (alt_arrow_key_down(Left)) {
        transform->x -= 1.2 * dt();
    }
    if (arrow_key_down(Left)) {
        transform->theta += 3 * dt();
    }
    if (arrow_key_down(Right)) {
        transform->theta -= 3 * dt();
    }
}


//================================================================================
// Systems
//================================================================================
void renderer_update(Component *component)
{
    // Enforce a single camera. (this does this every renderer component ...)
    // What about a "uniform" system behaviour, which prepares global state to each separate run of the updater for components,
    // and a way to allow streaming in also of "component groupings" and enforcements, like renderer and transforms ...
    // So, stream components of a certain (super-)type and then do the cases stuff inside the system. It gets components of a 
    // certain type heirarchy, possibly can just access sibling components itself.
    //
    // So components are just data modules. Entities are just empty hulls for connections to systems, which operate with that data
    // and some type flags to work with that in a case by case basis due to the logic of the system.
    Camera *camera;
    Iterator camera_iterator;
    iterator_components_of_type(Camera, &camera_iterator);
    step(&camera_iterator);
    if (camera_iterator.val == NULL) {
        fprintf(stderr, "ERROR: renderer could not find a camera.\n");
        exit(EXIT_FAILURE);
    }
    camera = camera_iterator.val;
    step(&camera_iterator);
    if (camera_iterator.val != NULL) {
        fprintf(stderr, "ERROR: Renderer has too many cameras to choose from. Must disable all except one.\n");
        exit(EXIT_FAILURE);
    }
    Transform *camera_transform = get_entity_component_of_type(camera->component.entity_id, Transform);
    if (camera_transform == NULL) {
        fprintf(stderr, "ERROR: The camera for the renderer does not have a Transform component.\n");
        exit(EXIT_FAILURE);
    }
    RendererShape *shape = (RendererShape *) component;
    Transform *transform = get_entity_component_of_type(shape->component.entity_id, Transform);
    Transform global_transform;
    get_global_transform(transform, &global_transform);

    if (GRID_RENDERING) {
        // Pixel grid rendering
        for (int i = 0; i < shape->poly.num_vertices; i++) {
            int ip = (i + 1) % shape->poly.num_vertices;
            double x, y;
            double xp, yp;
            x =  global_transform.x
                 + global_transform.scale_x * cos(global_transform.theta - camera_transform->theta) * shape->poly.vertices[i].x
                 + global_transform.scale_y * sin(global_transform.theta - camera_transform->theta) * shape->poly.vertices[i].y
                 - camera_transform->x;
            y = global_transform.y
                - global_transform.scale_x * cos(global_transform.theta - camera_transform->theta) * shape->poly.vertices[i].y
                + global_transform.scale_y * sin(global_transform.theta - camera_transform->theta) * shape->poly.vertices[i].x
                - camera_transform->y;
            xp =  global_transform.x
                 + global_transform.scale_x * cos(global_transform.theta - camera_transform->theta) * shape->poly.vertices[ip].x
                 + global_transform.scale_y * sin(global_transform.theta - camera_transform->theta) * shape->poly.vertices[ip].y
                 - camera_transform->x;
            yp = global_transform.y
                 - global_transform.scale_x * cos(global_transform.theta - camera_transform->theta) * shape->poly.vertices[ip].y
                 + global_transform.scale_y * sin(global_transform.theta - camera_transform->theta) * shape->poly.vertices[ip].x
                 - camera_transform->y;
            // --- Transform to "grid-screen coordinates"
            rasterize_line(x, y, xp, yp);
        }
    }
    else {
        // OpenGL primitive polygon rendering
        glBegin(GL_POLYGON);
            for (int i = 0; i < shape->poly.num_vertices; i++) {
                double x, y;
                // Point location relative to the camera
                x =  global_transform.x
                     + global_transform.scale_x * cos(global_transform.theta - camera_transform->theta) * shape->poly.vertices[i].x
                     + global_transform.scale_y * sin(global_transform.theta - camera_transform->theta) * shape->poly.vertices[i].y
                     - camera_transform->x;
                y = global_transform.y
                    - global_transform.scale_x * cos(global_transform.theta - camera_transform->theta) * shape->poly.vertices[i].y
                    + global_transform.scale_y * sin(global_transform.theta - camera_transform->theta) * shape->poly.vertices[i].x
                    - camera_transform->y;

                // Convert to normalized device coordinates, taking into account aspect ratio.
                glVertex2f(x * SCREEN_ASPECT_RATIO, y);
                /* glVertex2f(x, y); */
            }
        glEnd();
    }
}

void object_logic_update(Component *component)
{
    ObjectLogic *logic = (ObjectLogic *) component;
    Entity *entity = get_entity(logic->component.entity_id);
    if (logic->update != NULL) {
        logic->update(entity);
    }
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
        else if (key == GLFW_KEY_L) {
            print_entity_list();
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
        else if (key == GLFW_KEY_E) {
            GRID_RENDERING = !GRID_RENDERING;
        }
        else {
            for (int i = 1; i < NUM_NUMBER_KEYS; i++) {
                if (key == number_key(i)) {
                    Entity *entity = get_entity(i);
                    if (entity == NULL) {
                        continue;
                    }
                    destroy_entity(entity->id);
                    /* if (entity->on) { */
                    /*     disable_entity(i); */
                    /* } else { */
                    /*     enable_entity(i); */
                    /* } */
                }
            }
        }
    }
}

void init_program(void)
{
    init_entity_model();

    MAX_GRID_SIZE = 150;
    GRID_SIZE = MAX_GRID_SIZE;
    grid_init(GRID_SIZE, GRID_SIZE);

    add_system("renderer", RendererShape, renderer_update);
    add_system("object logic", ObjectLogic, object_logic_update);

    EntityID camera_man = create_entity(UNIVERSE_ID, "camera man");
    ObjectLogic *camera_object_logic = entity_add_component_get(camera_man, "controls", ObjectLogic);
    camera_object_logic->update = camera_controls;
    Transform *camera_transform = entity_add_component_get(camera_man, "transform", Transform);
    camera_transform->x = 0;
    camera_transform->y = 0;
    camera_transform->theta = 0;
    Camera *camera = entity_add_component_get(camera_man, "camera", Camera);
    camera->width = 1;
    camera->height = 1;

    /* EntityID camera2 = create_entity(UNIVERSE_ID, "camera2"); */
    /* entity_add_component(camera2, "transform", Transform); */
    /* entity_add_component(camera2, "camera", Camera); */
}

void loop(GLFWwindow *window)
{
    if (arrow_key_down(Up)) {
        if (GRID_SIZE < MAX_GRID_SIZE) {
            GRID_SIZE += 1;
        }
        resize_grid(GRID_SIZE, GRID_SIZE);
    }
    if (arrow_key_down(Down)) {
        if (GRID_SIZE > 1) {
            GRID_SIZE -= 1;
        }
        resize_grid(GRID_SIZE, GRID_SIZE);
    }

    grid_clear();

    update_entity_model();
    render_grid(WINDOW);
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
    WINDOW = init_glfw_create_context("Shapes", 512, 512);
    /* glViewport(100, 100, 512, 512); */
    /* glfwSetWindowAspectRatio(WINDOW, 1, 1); */
    ASPECT_RATIO = SCREEN_ASPECT_RATIO;

    glfwSetKeyCallback(WINDOW, key_callback);
    glfwSetFramebufferSizeCallback(WINDOW, reshape);

    init_program();
    loop_time(WINDOW, loop);
    close_program();
    
    exit(EXIT_SUCCESS);
}
