/* PROJECT_LIBS:
 *      + glad
 *      + helper_gl
 *      + helper_input
 *      + matrix_mathematics
 *      + mesh
 *      + entity
 *      + iterator
 * 
 * Isometric viewing
 */
#define SHADERS_LOCATION "/home/lucas/code/collision/src/isometric/"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "helper_gl.h"
#include "helper_input.h"
#include "matrix_mathematics.h"
#include "mesh.h"
#include "entity.h"
#include "iterator.h"

#define Transform_TYPE_ID 1
typedef struct Transform_s {
    Component component;
    Matrix4x4f matrix;
} Transform;

#define Camera_TYPE_ID 2
typedef struct Camera_s {
    Component component;
    ComponentID transform; // depends on Transform
    Matrix4x4f projection_matrix;
    Renderer renderer;
} Camera;

#define MeshRenderer_TYPE_ID 3
typedef struct MeshRenderer_s {
    Component component;
    ComponentID transform; // depends on Transform
    MeshHandle mesh_handle;
} MeshRenderer;

#define ObjectLogic_TYPE_ID 4
typedef struct ObjectLogic_s {
    Component component;
    void (*update) (Entity *);
} ObjectLogic;

void reshape(GLFWwindow* window, int width, int height);
void close_program(void);
void loop(GLFWwindow *window);
void init_program(void);
static EntityID create_thing(EntityID parent_id, char *name, float x, float y, float z, Mesh *mesh, void (*update) (Entity *));
static void alt_arrow_keys_move(float speed, float *x, float *y);
static void arrow_keys_move(float speed, float *x, float *y);
static void object_logic_update(Component *component);
static void mesh_rendering_update(Component *component);
static Camera *get_singleton_camera();
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods);
static void movement_controls_1(Entity *entity);
static void movement_controls_2(Entity *entity);
static void random_move(Entity *entity);

// Game logic, update functions for objects
//--------------------------------------------------------------------------------
static void movement_controls_1(Entity *entity)
{
    Transform *transform = get_entity_component_of_type(entity->id, Transform);
    float move_x, move_y;
    arrow_keys_move(3.0, &move_x, &move_y);

    /* euler_rotate_matrix4x4f( &transform->matrix, move_x, move_y, move_z); */
    translate_matrix4x4f(&transform->matrix, move_x, 0, move_y);
}
static void movement_controls_2(Entity *entity)
{
    Transform *transform = get_entity_component_of_type(entity->id, Transform);
    float move_x, move_y, move_z;
    alt_arrow_keys_move(3.0, &move_x, &move_y);
    euler_rotate_matrix4x4f(&transform->matrix, 0, move_x, 0);
}
static void random_move(Entity *entity)
{
    /* "random" */
    Transform *transform = get_entity_component_of_type(entity->id, Transform);
    translate_matrix4x4f(&transform->matrix, (0.2 + 16*frand()) * 0.1 * cos(frand() * time()) * dt(), (0.2 + 16*frand()) * 0.2 * sin(frand() * 1.3*time()) * dt(), (0.2 + 16*frand()) * 0.13 * (sin(frand() * time()) + cos(time()))*dt());
}
//--------------------------------------------------------------------------------

// Globals for testing -----------------------------------------------------------
static double ASPECT_RATIO;
//--------------------------------------------------------------------------------


static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_T) {
            print_entity_tree();
        }
        if (key == GLFW_KEY_SPACE) {
            for (int i = 0; i < 5; i++) {
                Mesh mesh;
                create_cube_mesh(&mesh, 0.2 + 0.2*frand());
                create_thing(UNIVERSE_ID, "cube", 2.0*frand() - 1.0, 2.0*frand() - 1.0, -6.0*frand(), &mesh, NULL);
            }
            for (int i = 0; i < 3; i++) {
                Mesh mesh;
                make_sphere(&mesh, 0.2 + 0.2*frand(), 12);
                EntityID sphere = create_thing(UNIVERSE_ID, "sphere", 2.0*frand() - 1.0, 2.0*frand() - 1.0, -6.0*frand(), &mesh, random_move);
                /* ObjectLogic *object_logic = entity_add_component_get(sphere, "controls", ObjectLogic); */
                /* object_logic->update = movement_controls_1; */
            }
        }
    }

}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{

}

// Systems
//--------------------------------------------------------------------------------
static Camera *get_singleton_camera()
{
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
    return camera;
}

static void mesh_rendering_update(Component *component)
{
    MeshRenderer *mesh_renderer = (MeshRenderer *) component;
    Camera *camera = get_singleton_camera(); // horrible
    Transform *camera_transform = (Transform *) get_component(camera->transform); // dangerous?
    Transform *mesh_transform = (Transform *) get_component(mesh_renderer->transform); // dangerous?
    render_mesh(&camera->renderer, &mesh_renderer->mesh_handle, &mesh_transform->matrix, &camera_transform->matrix, &camera->projection_matrix);
}

static void object_logic_update(Component *component)
{
    ObjectLogic *object_logic = (ObjectLogic *) component;
    Entity *entity = get_entity(object_logic->component.entity_id);
    if (object_logic->update != NULL) {
        object_logic->update(entity);
    }
}

//--------------------------------------------------------------------------------
// Object logic
static void arrow_keys_move(float speed, float *x, float *y)
{
    *x = 0;
    *y = 0;
    if (arrow_key_down(Down)) {
        *y -= speed * dt();
    }
    if (arrow_key_down(Up)) {
        *y += speed * dt();
    }
    if (arrow_key_down(Left)) {
        *x -= speed * dt();
    }
    if (arrow_key_down(Right)) {
        *x += speed * dt();
    }
}
static void alt_arrow_keys_move(float speed, float *x, float *y)
{
    *x = 0;
    *y = 0;
    if (alt_arrow_key_down(Down)) {
        *y -= speed * dt();
    }
    if (alt_arrow_key_down(Up)) {
        *y += speed * dt();
    }
    if (alt_arrow_key_down(Left)) {
        *x -= speed * dt();
    }
    if (alt_arrow_key_down(Right)) {
        *x += speed * dt();
    }
}

// "things", testing viewing these
static EntityID create_thing(EntityID parent_id, char *name, float x, float y, float z, Mesh *mesh, void (*update) (Entity *))
{
    EntityID new_thing = create_entity(parent_id, name);
    Transform *transform = entity_add_component_get(new_thing, "Transform", Transform);
    identity_matrix4x4f(&transform->matrix);
    translate_matrix4x4f(&transform->matrix, x, y, z);

    MeshRenderer *mesh_renderer = entity_add_component_get(new_thing, "Mesh renderer", MeshRenderer);
    mesh_renderer->transform = transform->component.id; // hook up reference to transform
    upload_and_free_mesh(&mesh_renderer->mesh_handle, mesh);

    ObjectLogic *object_logic = entity_add_component_get(new_thing, "thing logic", ObjectLogic);
    object_logic->update = update;

    return new_thing;
}

// Functions to retrieve uniform values for renderers
static UniformData uniform_get_aspect_ratio(void)
{
    UniformData data;
    data.float_value = ASPECT_RATIO;
    return data;
}
//--------------------------------------------------------------------------------
void init_program(void)
{
    init_entity_model();
    add_system("mesh rendering", MeshRenderer, mesh_rendering_update);
    add_system("object logic", ObjectLogic, object_logic_update);
    {
        EntityID camera_entity = create_entity(UNIVERSE_ID, "camera");
        Transform *transform = entity_add_component_get(camera_entity, "Transform", Transform);
        identity_matrix4x4f(&transform->matrix);
        translate_matrix4x4f(&transform->matrix, 0, 0, -2);

        Camera *camera = entity_add_component_get(camera_entity, "Camera", Camera);
        identity_matrix4x4f(&camera->projection_matrix); // for now

        float vals[] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
        };
        memcpy(camera->projection_matrix.vals, vals, sizeof(vals));
        
        new_renderer_vertex_fragment(&camera->renderer, SHADERS_LOCATION "isometric.vert", SHADERS_LOCATION "isometric.frag", GL_TRIANGLES);
        renderer_add_uniform(&camera->renderer, "aspect_ratio", uniform_get_aspect_ratio, GL_FLOAT);

        // hook up reference to transform in the camera component (it depends on a Transform sibling)
        camera->transform = transform->component.id;

        {
            ObjectLogic *object_logic = entity_add_component_get(camera_entity, "controls 1", ObjectLogic);
            object_logic->update = movement_controls_1;
        } {
            ObjectLogic *object_logic = entity_add_component_get(camera_entity, "controls 2", ObjectLogic);
            object_logic->update = movement_controls_2;
        }
    }

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
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    printf("x:%d, y: %d, width: %d, height: %d\n", viewport[0], viewport[1], viewport[2], viewport[3]);
}

int main(int argc, char *argv[])
{
    /* Main function should be purely GLFW, window and context functions, and calls to the program init, loop, and close */

    GLFWwindow *window;
    int horiz = 512;
    int vert = 512;
    char *name = "Shader testing";
    //--------------------------------------------------------------------------------
    if (!glfwInit()) {
        fprintf(stderr, "GLFW error: something went wrong initializing GLFW\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(horiz, vert, name, NULL, NULL);
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
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    glEnable(GL_CULL_FACE); // on the renderer level?
    glCullFace(GL_BACK);

    init_program();
    loop_time(window, loop, GL_COLOR_BUFFER_BIT);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);

}
