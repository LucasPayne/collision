/*================================================================================
PROJECT_LIBS:
    + glad
    + helper_gl
    + helper_input
    + entity
    + iterator
    + mesh
    + ply
    + matrix_mathematics
================================================================================*/
#define SHADERS_LOCATION "/home/lucas/code/collision/src/swim/"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "project_definitions.h"
#include "helper_gl.h"
#include "helper_input.h"
#include "entity.h"
#include "mesh.h"
#include "ply.h"
#include "matrix_mathematics.h"

// Aspects of things
//--------------------------------------------------------------------------------
#define Transform_TYPE_ID 1
typedef struct Transform_s {
ASPECT_PROPERTIES()
    float x;
    float y;
    float z;
    float theta_x;
    float theta_y;
    float theta_z;
} Transform;
static void set_transform(Transform *transform, float x, float y, float z, float theta_x, float theta_y, float theta_z)
{
    transform->x = x;
    transform->y = y;
    transform->z = z;
    transform->theta_x = theta_x;
    transform->theta_y = theta_y;
    transform->theta_z = theta_z;
}
static Matrix4x4f transform_matrix(Transform *transform)
{
    Matrix4x4f mat;
    translate_rotate_3d_matrix4x4f(&mat, transform->x, transform->y, transform->z, transform->theta_x, transform->theta_y, transform->theta_z);
    return mat;
}
#define Body_TYPE_ID 2
typedef struct Body_s {
ASPECT_PROPERTIES()
    MeshHandle *mesh_handle;
    Renderer *renderer;
    bool visible;
} Body;
#define Logic_TYPE_ID 3
typedef struct Logic_s {
ASPECT_PROPERTIES()
    void (*update)(struct Logic_s *);
    void *data;
} Logic;
#define init_logic(LOGIC,OBJ_TYPE_NAME,UPDATE)\
{\
    ( LOGIC )->update = ( UPDATE );\
    ( LOGIC )->data = (struct OBJ_TYPE_NAME ## Data *) calloc(1, sizeof(struct OBJ_TYPE_NAME ## Data));\
    mem_check(( LOGIC )->data);\
}
#define object_data(LOGIC,OBJ_TYPE_NAME)\
    ((struct OBJ_TYPE_NAME ## Data *) ( LOGIC )->data)


//--------------------------------------------------------------------------------
static double ASPECT_RATIO;

static Renderer g_dolphin_renderer;
static Renderer g_bunny_renderer;

static MeshHandle g_dolphin_mesh_handle;
static MeshHandle g_bunny_mesh_handle;
static MeshHandle g_cube_mesh_handle;

static Matrix4x4f g_mvp_matrix;
//--------------------------------------------------------------------------------
// Entities and entity logic

typedef struct DolphinData {
    float tilt_offset;
    float swim_speed;
};
static void dolphin_update(Logic *logic)
{

    struct DolphinData *data = object_data(logic, Dolphin);
    Transform *transform = get_sibling_aspect(logic, Transform);

    for_aspect(Transform, other_transform)
        transform->x += sin(time()) * (other_transform->x - transform->x) * 0.005 * dt();
        transform->y += sin(time()) * (other_transform->y - transform->y) * 0.005 * dt();
        transform->z += sin(time()) * (other_transform->z - transform->z) * 0.005 * dt();
    end_for_aspect()
    
    float rotate_speed = 7;
    if (arrow_key_down(Right)) transform->theta_y += rotate_speed * dt();
    if (arrow_key_down(Left)) transform->theta_y -= rotate_speed * dt();

    float move_speed = 20;
    if (alt_arrow_key_down(Left)) transform->x -= move_speed * dt();
    if (alt_arrow_key_down(Right)) transform->x += move_speed * dt();
    if (alt_arrow_key_down(Up)) transform->y += move_speed * dt();
    if (alt_arrow_key_down(Down)) transform->y -= move_speed * dt();

    transform->theta_y += data->swim_speed * sin(time() * 3 + data->tilt_offset) * dt();
}

typedef struct BunnyData {
    float rotational_velocity;
    float accel_speed;
    float x_vel;
    float y_vel;
    float timer;
    int mode;
};
static void bunny_update(Logic *logic)
{
    struct BunnyData *data = object_data(logic, Bunny);
    Transform *transform = get_sibling_aspect(logic, Transform);
    transform->theta_y += data->rotational_velocity * dt();

    Body *body = get_sibling_aspect(logic, Body);
    switch(data->mode) {
    case 0:
        body->mesh_handle = &g_bunny_mesh_handle;
        break;
    case 1:
        body->mesh_handle = &g_dolphin_mesh_handle;
        break;
    case 2:
        body->mesh_handle = &g_cube_mesh_handle;
        break;
    }

    data->x_vel *= 1 - 0.2 * dt();
    data->y_vel *= 1 - 0.2 * dt();
    if (arrow_key_down(Left)) data->x_vel -= data->accel_speed * dt();
    if (arrow_key_down(Right)) data->x_vel += data->accel_speed * dt();
    if (arrow_key_down(Up)) data->y_vel += data->accel_speed * dt();
    if (arrow_key_down(Down)) data->y_vel -= data->accel_speed * dt();

    if (data->timer <= 0) {
        data->mode = (data->mode + 1) % 3;
        data->timer = 4 * frand() + 1;
    }
    data->timer -= dt();

    transform->x += data->x_vel * dt();
    transform->y += data->y_vel * dt();
}

//--------------------------------------------------------------------------------

static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_K) {
            renderer_recompile_shaders(&g_dolphin_renderer);
            renderer_recompile_shaders(&g_bunny_renderer);
        }
    }
}


static UniformData get_uniform_value_aspect_ratio(void)
{
    return (UniformData) (float) ASPECT_RATIO;
}
static UniformData get_uniform_value_mvp_matrix(void)
{
    UniformData data;
    memcpy(data.mat4x4_value, g_mvp_matrix.vals, 4 * 4 * sizeof(float));
    return data;
}
static UniformData get_uniform_value_time(void)
{
    return (UniformData) (float) time();
}
static UniformData get_uniform_value_frand(void)
{
    return (UniformData) (float) frand();
}

static EntityID create_game_object(MeshHandle *mesh_handle, Renderer *renderer, float x, float y, float z, float theta_x, float theta_y, float theta_z)
{
    EntityID new = new_entity(3);
    set_transform(entity_add_aspect(new, Transform), x, y, z, theta_x, theta_y, theta_z);
    Body *body = entity_add_aspect(new, Body);
    body->visible = true;
    body->renderer = renderer;
    body->mesh_handle = mesh_handle;
    return new;
}

void init_program(void)
{
    // Entity system
    //--------------------------------------------------------------------------------
    init_entity_model();
    new_default_manager(Transform, NULL);
    new_default_manager(Body, NULL);
    new_default_manager(Logic, NULL);
    // Shaders
    //--------------------------------------------------------------------------------
    new_renderer_vertex_fragment(&g_dolphin_renderer, VERTEX_FORMAT_3, SHADERS_LOCATION "dolphin.vert", SHADERS_LOCATION "passthrough.frag");
    renderer_add_uniform(&g_dolphin_renderer, "aspect_ratio", get_uniform_value_aspect_ratio, UNIFORM_FLOAT);
    renderer_add_uniform(&g_dolphin_renderer, "mvp_matrix", get_uniform_value_mvp_matrix, UNIFORM_MAT4X4);
    renderer_add_uniform(&g_dolphin_renderer, "frand", get_uniform_value_frand, UNIFORM_FLOAT);

    new_renderer_vertex_fragment(&g_bunny_renderer, VERTEX_FORMAT_3, SHADERS_LOCATION "bunny.vert", SHADERS_LOCATION "passthrough.frag");
    renderer_add_uniform(&g_bunny_renderer, "aspect_ratio", get_uniform_value_aspect_ratio, UNIFORM_FLOAT);
    renderer_add_uniform(&g_bunny_renderer, "mvp_matrix", get_uniform_value_mvp_matrix, UNIFORM_MAT4X4);
    renderer_add_uniform(&g_bunny_renderer, "time", get_uniform_value_time, UNIFORM_FLOAT);
    // Meshes
    //--------------------------------------------------------------------------------
    Mesh mesh;
    load_mesh_ply(&mesh, VERTEX_FORMAT_3, DATA_DIR "models/dolphins.ply", 0.01);
    upload_and_free_mesh(&g_dolphin_mesh_handle, &mesh);
    load_mesh_ply(&mesh, VERTEX_FORMAT_3, DATA_DIR "models/stanford_bunny_low.ply", 50);
    upload_and_free_mesh(&g_bunny_mesh_handle, &mesh);
    load_mesh_ply(&mesh, VERTEX_FORMAT_3C, DATA_DIR "models/plytest2.ply", 1);
    upload_and_free_mesh(&g_cube_mesh_handle, &mesh);
    // Entities
    //--------------------------------------------------------------------------------
    {
        for (int i = 0; i < 65; i++) {
            EntityID dolphin = create_game_object(&g_dolphin_mesh_handle, &g_dolphin_renderer, 50 * frand() - 25, 50 * frand() - 25, -20, M_PI / 2, 0, 0);
            Logic *logic = entity_add_aspect(dolphin, Logic);
            init_logic(logic, Dolphin, dolphin_update);
            object_data(logic, Dolphin)->tilt_offset = 2 * M_PI * frand();
            object_data(logic, Dolphin)->swim_speed = 3 + frand();
        }
    }
    {
        for (int i = 0; i < 20; i++) {
            EntityID bunny = create_game_object(&g_bunny_mesh_handle, &g_bunny_renderer, 50 * frand() - 25, 50 * frand() - 25, -20, 0, 0, 0);
            Logic *logic = entity_add_aspect(bunny, Logic);
            init_logic(logic, Bunny, bunny_update);
            object_data(logic, Bunny)->rotational_velocity = 6 * frand();
            object_data(logic, Bunny)->accel_speed = 20 + 10 * frand();
        }
    }
}
void loop()
{
    for_aspect(Logic, logic)
        if (logic->update != NULL) logic->update(logic);
    end_for_aspect()

    for_aspect(Body, body)
        if (body->visible) {
            Matrix4x4f model_matrix = transform_matrix(get_sibling_aspect(body, Transform));
            identity_matrix4x4f(&g_mvp_matrix);
            right_multiply_matrix4x4f(&g_mvp_matrix, &model_matrix);
            render_mesh(body->renderer, body->mesh_handle, GL_TRIANGLES);
        }
    end_for_aspect()
}
void close_program(void)
{
}

void reshape(GLFWwindow* window, int width, int height)
{
    force_aspect_ratio(window, width, height, ASPECT_RATIO);
}

int main(int argc, char *argv[])
{
    ASPECT_RATIO = SCREEN_ASPECT_RATIO;

    GLFWwindow *window = gl_core_standard_window("Dolphins swimming", init_program, loop, close_program);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, reshape);
    gladLoadGL();


    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    init_program();
    loop_time(window, loop, GL_COLOR_BUFFER_BIT);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
