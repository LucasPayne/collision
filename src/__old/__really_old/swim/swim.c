/*================================================================================
PROJECT_LIBS:
    + glad
    + helper_gl
    + helper_input
    + helper_io
    + entity
    + iterator
    + mesh
    + ply
    + matrix_mathematics
    + resources
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
#include "helper_io.h"
#include "entity.h"
#include "mesh.h"
#include "ply.h"
#include "matrix_mathematics.h"
#include "resources.h"

// Aspects of things
//--------------------------------------------------------------------------------
static AspectType Transform_TYPE_ID;
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
static AspectType Body_TYPE_ID;
typedef struct Body_s {
ASPECT_PROPERTIES()
    ResourceHandle mesh; // MeshHandle
    Renderer *renderer;
    bool visible;
} Body;
static AspectType Logic_TYPE_ID;
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
// Resources
//--------------------------------------------------------------------------------
// Resourcify the transform aspect.
/* static ResourceType Transform_RTID; */
/* static void *Transform_load(char *path) */
/* { */
/*     FILE *image_file = resource_file_open(path, ".image", "rb"); */
/*     fopen_check(image_file); */
/*     int size; */
/*     void *contents = load_file(image_file, &size); */
/*     if (contents == NULL) { */
/*         printf("Couldn't read file.\n"); */
/*         return NULL; */
/*     } */
/*     fclose(image_file); */

/*     return contents; */
/* } */

//--------------------------------------------------------------------------------



//--------------------------------------------------------------------------------
static double ASPECT_RATIO;

static Renderer g_dolphin_renderer;
static Renderer g_bunny_renderer;

/* static MeshHandle g_dolphin_mesh_handle; */
/* static MeshHandle g_bunny_mesh_handle; */
/* static MeshHandle g_cube_mesh_handle; */

static Matrix4x4f g_mvp_matrix;
static float g_model_x;
static float g_model_y;
static float g_model_z;
static float g_light_pos_x = 0.0;
static float g_light_pos_y = 0.0;
static float g_light_pos_z = 0.0;

//--------------------------------------------------------------------------------
// Entities and entity logic

struct DolphinData {
    float tilt_offset;
    float swim_speed;
};
static void dolphin_update(Logic *logic)
{

    struct DolphinData *data = object_data(logic, Dolphin);
    Transform *transform = get_sibling_aspect(logic, Transform);

    for_aspect(Transform, other_transform)
        transform->x += sin(time()) * (other_transform->x - transform->x) * .5e-2 * dt();
        transform->y += sin(time()) * (other_transform->y - transform->y) * .5e-2 * dt();
        transform->z += sin(time()) * (other_transform->z - transform->z) * .5e-2 * dt();
    end_for_aspect()
    
    float rotate_speed = 7;
    if (arrow_key_down(Right)) transform->theta_y += rotate_speed * dt();
    if (arrow_key_down(Left)) transform->theta_y -= rotate_speed * dt();

#if 0
    float move_speed = 20;
    if (alt_arrow_key_down(Left)) transform->x -= move_speed * dt();
    if (alt_arrow_key_down(Right)) transform->x += move_speed * dt();
    if (alt_arrow_key_down(Up)) transform->y += move_speed * dt();
    if (alt_arrow_key_down(Down)) transform->y -= move_speed * dt();
#endif

    transform->theta_y += data->swim_speed * sin(time() * 3 + data->tilt_offset) * dt();
}

struct BunnyData {
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

#if 1
    Body *body = get_sibling_aspect(logic, Body);
    switch(data->mode) {
    case 0:
        init_resource_handle(MeshHandle, body->mesh, "Application/models/cube");
        break;
    case 1:
        init_resource_handle(MeshHandle, body->mesh, "Application/models/dolphins");
        break;
    case 2:
        init_resource_handle(MeshHandle, body->mesh, "Application/models/stanford_bunny_big_boy");
        break;
    case 3:
        init_resource_handle(MeshHandle, body->mesh, "Application/models/icosohedron");
        break;
    }
#endif

    data->x_vel *= 1 - 0.2 * dt();
    data->y_vel *= 1 - 0.2 * dt();
    if (arrow_key_down(Left)) data->x_vel -= data->accel_speed * dt();
    if (arrow_key_down(Right)) data->x_vel += data->accel_speed * dt();
    if (arrow_key_down(Up)) data->y_vel += data->accel_speed * dt();
    if (arrow_key_down(Down)) data->y_vel -= data->accel_speed * dt();

    if (data->timer <= 0) {
        data->mode = (data->mode + 1) % 4;
        data->timer = 10 * frand() + 1;
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
static UniformData get_uniform_value_model_x(void) { return (UniformData) (float) g_model_x; }
static UniformData get_uniform_value_model_y(void) { return (UniformData) (float) g_model_y; }
static UniformData get_uniform_value_model_z(void) { return (UniformData) (float) g_model_z; }
static UniformData get_uniform_value_light_pos_x(void) { return (UniformData) (float) g_light_pos_x; }
static UniformData get_uniform_value_light_pos_y(void) { return (UniformData) (float) g_light_pos_y; }
static UniformData get_uniform_value_light_pos_z(void) { return (UniformData) (float) g_light_pos_z; }

static EntityID create_game_object(char *mesh_path, Renderer *renderer, float x, float y, float z, float theta_x, float theta_y, float theta_z)
{
    EntityID new = new_entity(3);
    set_transform(entity_add_aspect(new, Transform), x, y, z, theta_x, theta_y, theta_z);
    Body *body = entity_add_aspect(new, Body);
    body->visible = true;
    body->renderer = renderer;
    /* body->mesh_handle = mesh_handle; */
    init_resource_handle(MeshHandle, body->mesh, mesh_path);
    return new;
}


//================================================================================
// TESTING
//================================================================================

// for testing the resource system
void Transform_serialize(char *filename, Transform *transform)
{
    FILE *file = fopen(filename, "wb+");
    fopen_check(file);
    if (fwrite(transform, sizeof(Transform), 1, file) != 1) {
        fprintf(stderr, ERROR_ALERT "Failed to serialize.\n");
        exit(EXIT_FAILURE);
    }
    fclose(file);
}
static void test1(char *transform_resource_path)
{
    ResourceHandle transform_handle;
    init_resource_handle(Transform, transform_handle, transform_resource_path);
    puts(transform_handle._path);
    Transform *transform = resource_data(Transform, transform_handle);
    printf("%f %f %f\n", transform->x, transform->y, transform->z);
    printf("%f %f %f\n", transform->theta_x, transform->theta_y, transform->theta_z);
    transform = resource_data(Transform, transform_handle);
    printf("AGAIN: %f %f %f\n", transform->x, transform->y, transform->z);
    printf("AGAIN: %f %f %f\n", transform->theta_x, transform->theta_y, transform->theta_z);
}
//================================================================================

void init_program(void)
{
#if 0
    /* add_resource_type(MeshHandle); */
    add_resource_type(Transform);
    
    /* print_resource_types(); */
    /* exit(EXIT_SUCCESS); */

    resource_path_add("Project", "/home/lucas/code/collision/data");
    resource_path_add("Application", "/home/lucas/code/collision/src/swim/data");

    Transform write_transform;
    set_transform(&write_transform, 0, 1, 2, 2.5, 1.5, 0.5);
    Transform_serialize("/home/lucas/code/collision/src/swim/data/transform.image", &write_transform);
    set_transform(&write_transform, 80, 80, 80, 80, 80, 80);
    Transform_serialize("/home/lucas/code/collision/src/swim/data/another_transform.image", &write_transform);

    test1("Application/transform");
    print_resource_tree();
    test1("Application/another_transform");
    print_resource_tree();
    test1("Application/transform");
    print_resource_tree();
    test1("Application/another_transform");
    print_resource_tree();

    /* test_resource_tree(); */

    /* FILE *texture_file = resource_file_open("Application/dirt", ".png", "rb"); */
    /* if (texture_file == NULL) { */
    /*     printf("Texture not found.\n"); */
    /*     return 0; */
    /* } */
    /* int size; */
    /* void *texture_data = load_file(texture_file, &size); */

    /* FILE *test_out = fopen("TEST.png", "wb+"); */
    /* if (test_out != NULL) fwrite(texture_data, size, 1, test_out); */

    /* exit(EXIT_SUCCESS); */
#endif
    
    // Resource system
    //--------------------------------------------------------------------------------
    add_resource_type(MeshHandle);
    add_resource_type(Shader);
    resource_path_add("Project", "/home/lucas/code/collision/data");
    resource_path_add("Application", "/home/lucas/code/collision/src/swim/data");
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
    renderer_add_uniform(&g_bunny_renderer, "model_x", get_uniform_value_model_x, UNIFORM_FLOAT);
    renderer_add_uniform(&g_bunny_renderer, "model_y", get_uniform_value_model_y, UNIFORM_FLOAT);
    renderer_add_uniform(&g_bunny_renderer, "model_z", get_uniform_value_model_z, UNIFORM_FLOAT);
    renderer_add_uniform(&g_bunny_renderer, "light_pos_x", get_uniform_value_light_pos_x, UNIFORM_FLOAT);
    renderer_add_uniform(&g_bunny_renderer, "light_pos_y", get_uniform_value_light_pos_y, UNIFORM_FLOAT);
    renderer_add_uniform(&g_bunny_renderer, "light_pos_z", get_uniform_value_light_pos_z, UNIFORM_FLOAT);
    
    // Entities
    //--------------------------------------------------------------------------------
    {
        for (int i = 0; i < 65; i++) {
            EntityID dolphin = create_game_object("Application/models/dolphins_sad", &g_dolphin_renderer, 50 * frand() - 25, 50 * frand() - 25, -20, M_PI / 2, 0, 0);
            Logic *logic = entity_add_aspect(dolphin, Logic);
            init_logic(logic, Dolphin, dolphin_update);
            object_data(logic, Dolphin)->tilt_offset = 2 * M_PI * frand();
            object_data(logic, Dolphin)->swim_speed = 3 + frand();
        }
    }
    {
        for (int i = 0; i < 20; i++) {
            EntityID bunny = create_game_object("Application/models/dolphins", &g_bunny_renderer, 50 * frand() - 25, 50 * frand() - 25, -20, 0, 0, 0);
            Logic *logic = entity_add_aspect(bunny, Logic);
            init_logic(logic, Bunny, bunny_update);
            object_data(logic, Bunny)->rotational_velocity = 6 * frand();
            object_data(logic, Bunny)->accel_speed = 20 + 10 * frand();
        }
    }
}
void loop()
{
    float light_move_speed = 0.5;
    if (alt_arrow_key_down(Left)) g_light_pos_x -= light_move_speed * dt();
    if (alt_arrow_key_down(Right)) g_light_pos_x += light_move_speed * dt();
    if (alt_arrow_key_down(Up)) g_light_pos_y += light_move_speed * dt();
    if (alt_arrow_key_down(Down)) g_light_pos_y -= light_move_speed * dt();

#if 1
    for_aspect(Logic, logic)
        if (logic->update != NULL) logic->update(logic);
    end_for_aspect()
#endif

#if 1
    for_aspect(Body, body)
        if (body->visible) {
            Transform *transform = get_sibling_aspect(body, Transform);
            Matrix4x4f model_matrix = transform_matrix(transform);
            g_model_x = transform->x;
            g_model_y = transform->y;
            g_model_z = transform->z;
            identity_matrix4x4f(&g_mvp_matrix);
            right_multiply_matrix4x4f(&g_mvp_matrix, &model_matrix);
            render_mesh(body->renderer, resource_data(MeshHandle, body->mesh), GL_TRIANGLES);
        }
    end_for_aspect()
#endif
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
