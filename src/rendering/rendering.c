/*================================================================================
PROJECT_LIBS:
    + glad
    + helper_gl
    + helper_input
    + dictionary
    + resources
    + rendering
    + iterator
    + entity
    + matrix_mathematics
    + ply
================================================================================*/
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
#include "resources.h"
#include "rendering.h"
#include "entity.h"
#include "matrix_mathematics.h"
//--------------------------------------------------------------------------------
static double ASPECT_RATIO;
static Matrix4x4f g_mvp_matrix;
//--------------------------------------------------------------------------------

// Aspects
//================================================================================

static AspectType Transform_TYPE_ID;
typedef struct /* Aspect */ Transform_s {
ASPECT_PROPERTIES()
    float x;
    float y;
    float z;
    float theta_x;
    float theta_y;
    float theta_z;
} Transform;
static void Transform_set(Transform *transform, float x, float y, float z, float theta_x, float theta_y, float theta_z)
{
    transform->x = x;
    transform->y = y;
    transform->z = z;
    transform->theta_x = theta_x;
    transform->theta_y = theta_y;
    transform->theta_z = theta_z;
}
static Matrix4x4f Transform_matrix(Transform *transform)
{
    Matrix4x4f mat;
    translate_rotate_3d_matrix4x4f(&mat, transform->x, transform->y, transform->z, transform->theta_x, transform->theta_y, transform->theta_z);
    return mat;
}

static AspectType Body_TYPE_ID;
typedef struct /* Aspect */ Body_s {
ASPECT_PROPERTIES()
    ResourceHandle artist; /* Resource: Artist */
    ResourceHandle mesh; /* Resource: Mesh */
    int test_val;
} Body;

static AspectType Camera_TYPE_ID;
typedef struct /* Aspect */ Camera_s {
ASPECT_PROPERTIES()
    Matrix4x4f projection_matrix; // the lens
} Camera;
void Camera_view(Camera *camera)
{
    //todo: Take into account rendering target, probably just a window sub-rectangle. 
    /* GraphicsProgram *program = resource_data(GraphicsProgram, camera->graphics_program); */
    /* printf("~~~ VIEWING ~~~\n"); */
    /* printf("vert path: %s\n", program->shaders[Vertex]._path); */
    /* printf("frag path: %s\n", program->shaders[Fragment]._path); */

    for_aspect(Body, body)
        /* printf("test_val: %d\n", body->test_val); */
        Mesh *mesh = resource_data(Mesh, body->mesh);
        /* printf("mesh num_vertices: %d, num_triangles: %d\n", mesh->num_vertices, mesh->num_triangles); */
        GraphicsProgram *graphics_program = resource_data(GraphicsProgram, resource_data(Artist, body->artist)->graphics_program);
        /* printf("graphics program id: %u\n", graphics_program->program_id); */
        // Set up the mvp matrix this program provides.
        identity_matrix4x4f(&g_mvp_matrix);
        right_multiply_matrix4x4f(&g_mvp_matrix, &camera->projection_matrix);
        Matrix4x4f model_matrix = Transform_matrix(get_sibling_aspect(body, Transform));
        /* right_multiply_by_transpose_matrix4x4f(&g_mvp_matrix, &model_matrix); */
        right_multiply_matrix4x4f(&g_mvp_matrix, &model_matrix);
        /* print_matrix4x4f(&g_mvp_matrix); */
        Artist_draw_mesh(resource_data(Artist, body->artist), mesh);
    end_for_aspect()
}
void Camera_init(Camera *camera)
{
    identity_matrix4x4f(&camera->projection_matrix);
    /* camera->graphics_program = new_resource_handle(GraphicsProgram, graphics_program_path); */
}
//-End aspects----------------------------------------------------------------------

// Uniform getters
GraphicsFloat get_uniform_aspect_ratio(void) { return (GraphicsFloat) ASPECT_RATIO; }
GraphicsMat4x4f get_uniform_mvp_matrix(void)
{
    GraphicsMat4x4f mat;
    memcpy(&mat, &g_mvp_matrix, sizeof(Matrix4x4f));
    /* print_matrix4x4f(&g_mvp_matrix); */
    /* for (int i = 0; i < 16; i++) { */
    /*     printf("%f/%f, ", mat.vals[i], g_mvp_matrix.vals[i]); */
    /* } */
    /* printf("\n"); */
    return mat;
    /* print_matrix4x4f(&g_mvp_matrix); */
    /* return *((GraphicsMat4x4f *) &g_mvp_matrix.vals); */
}
/* GraphicsMat4x4f get_uniform_mvp_matrix(void) { for(int i = 0; i < 16; i++) printf("%d, ", ((GraphicsMat4x4f *) &g_mvp_matrix)->vals[i]);printf("\n");return *((GraphicsMat4x4f *) &g_mvp_matrix); } */

void init_program(void)
{
    identity_matrix4x4f(&g_mvp_matrix);

    init_entity_model();
    new_default_manager(Camera, NULL);
    new_default_manager(Body, NULL);
    new_default_manager(Transform, NULL);

    init_resources_rendering();
    resource_path_add("Shaders", "/home/lucas/code/collision/src/rendering/shaders");
    resource_path_add("Meshes", "/home/lucas/code/collision/src/rendering/meshes");
    // Create artists
    ResourceHandle res_artist_1 = new_resource_handle(Artist, "Virtual/artists/1");
    Artist *artist_1 = resource_data(Artist, res_artist_1);
    artist_1->graphics_program = new_resource_handle(GraphicsProgram, "Shaders/programs/default");

    Artist_add_uniform(artist_1, "mvp_matrix", (UniformGetter) get_uniform_mvp_matrix, UNIFORM_MAT4X4F);
    Artist_add_uniform(artist_1, "aspect_ratio", (UniformGetter) get_uniform_aspect_ratio, UNIFORM_FLOAT);


    EntityID cameraman = new_entity(1);
    Camera_init(entity_add_aspect(cameraman, Camera));
    for (int i = 0; i < 3; i++) {
        EntityID dude = new_entity(2);
        Transform_set(entity_add_aspect(dude, Transform), 2*frand()-1.0, 2*frand()-1.0, 2*frand()-1.0, M_PI*frand(),M_PI*frand(),M_PI*frand());
        Body *body = entity_add_aspect(dude, Body);
        body->test_val = 3;
        body->mesh = new_resource_handle(Mesh, "Meshes/dolphins");
        body->artist = new_resource_handle(Artist, "Virtual/artists/1");
        /* Artist_init(&body->artist, */ 
        /* Artist_add_uniform(&body->artist, (UniformGetter) get_uniform_mvp_matrix */
    }

    /* { EntityID dude = new_entity(1); */
    /* entity_add_aspect(dude, Body)->test_val = 7; } */

#if 0
    {
        ResourceHandle shader = new_resource_handle(Shader, "Shaders/example.vert");
        printf("shader_id: %u\n", resource_data(Shader, shader)->shader_id);
    }
    {
        ResourceHandle shader = new_resource_handle(Shader, "Shaders/passthrough.frag");
        printf("shader_id: %u\n", resource_data(Shader, shader)->shader_id);
    }
#endif
}
void loop(void)
{

    for_aspect(Transform, transform)
        transform->theta_x += dt() * 3;
        transform->z += dt() * 3;
        transform->y += dt() * 3;
        transform->x += dt() * 3;
    end_for_aspect()

    for_aspect(Camera, camera)
        Camera_view(camera);
    end_for_aspect()

#if 0
    {
        ResourceHandle shader = new_resource_handle(Shader, "Shaders/example.vert");
        printf("shader_id: %u\n", resource_data(Shader, shader)->shader_id);
    }
    {
        ResourceHandle shader = new_resource_handle(Shader, "Shaders/passthrough.frag");
        printf("shader_id: %u\n", resource_data(Shader, shader)->shader_id);
    }
    {
        /* ResourceHandle shader = new_resource_handle(Shader, "Shaders/bungel.vert"); */
        /* printf("shader_id: %u\n", resource_data(Shader, shader)->shader_id); */
    }
#endif
}
void close_program(void)
{
}
static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE) {
            for_aspect(Body, body)
                init_resource_handle(Mesh, body->mesh, "Meshes/cube");
            end_for_aspect()
        }
    }
}


void reshape(GLFWwindow* window, int width, int height)
{
    force_aspect_ratio(window, width, height, ASPECT_RATIO);
}

int main(int argc, char *argv[])
{
    GLFWwindow *window;
    char *name = "Shader testing";
    //--------------------------------------------------------------------------------
    if (!glfwInit()) {
        fprintf(stderr, "GLFW error: something went wrong initializing GLFW\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1, 1, name, NULL, NULL);
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
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop, GL_COLOR_BUFFER_BIT);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
