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
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{

}

// Systems

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
    Camera *camera = get_singleton_camera(); // horrible

    MeshRenderer *mesh_renderer = (MeshRenderer *) component;
    Transform *transform = (Transform *) get_component(mesh_renderer->transform); // dangerous?

    render_mesh(&camera->renderer, &mesh_renderer->mesh_handle, &transform->matrix);
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

        Camera *camera = entity_add_component_get(camera_entity, "Camera", Camera);
        identity_matrix4x4f(&camera->projection_matrix); // for now
        new_renderer_vertex_fragment(&camera->renderer, SHADERS_LOCATION "isometric.vert", SHADERS_LOCATION "isometric.frag");
        // hook up reference to transform in the camera component (it depends on a Transform sibling)
        camera->transform = transform->component.id;
    }
    {
        EntityID thing = create_entity(UNIVERSE_ID, "thing");
        Transform *transform = entity_add_component_get(thing, "Transform", Transform);
        identity_matrix4x4f(&transform->matrix);

        MeshRenderer *mesh_renderer = entity_add_component_get(thing, "Mesh renderer", MeshRenderer);
        mesh_renderer->transform = transform->component.id; // hook up reference to transform

        Mesh mesh;
        /* create_cube_mesh(&mesh, 0.93); */
        make_sphere(&mesh, 0.7, 10);
        upload_and_free_mesh(&mesh_renderer->mesh_handle, &mesh);
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

    init_program();
    loop_time(window, loop, GL_COLOR_BUFFER_BIT);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
