/*================================================================================
PROJECT_LIBS:
    + glad
    + helper_gl
    + helper_input
    + entity
    + iterator
    + matrix_mathematics
    + mesh
    + mesh_gen
    + ply

------ BUGS
Alternative management example with SeeingMesh.
Works to a good extent, doesn't seem to make it faster.
--- seems like the pointer relocations don't work, may not segfault because of direct array iteration.


---- destroying aspects
---- removing from aspect list?
------ at least the sort of manipulation is kept internal, but there is a lot of weird stuff to mess around with the entity-aspect model.
================================================================================*/
#define SHADERS_LOCATION "/home/lucas/code/collision/src/storm/"
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
#include "matrix_mathematics.h"
#include "mesh.h"
#include "mesh_gen.h"
#include "ply.h"

#define Transform_TYPE_ID 1
#define SeeingMesh_TYPE_ID 2
#define ObjectLogic_TYPE_ID 3

typedef struct Transform_s {
ASPECT_PROPERTIES()
    Matrix4x4f matrix;
} Transform;
void Transform_serialize(FILE *file, void *self)
{
    fprint_matrix4x4f(file, &((Transform *) self)->matrix);
}

typedef struct SeeingMesh_s {
ASPECT_PROPERTIES()
    MeshHandle mesh_handle;
    Renderer *renderer;
    bool visible;
} SeeingMesh;
static SeeingMesh *g_SeeingMesh_array;
static int g_SeeingMesh_array_size = 0;
static int g_SeeingMesh_array_index = 0;
static void initialize_SeeingMesh_manager(void)
{
    g_SeeingMesh_array_size = 15000;
    g_SeeingMesh_array_index = 0;
    g_SeeingMesh_array = (SeeingMesh *) calloc(g_SeeingMesh_array_size, sizeof(SeeingMesh));
    mem_check(g_SeeingMesh_array);
}
static void SeeingMesh_new(Manager *manager, AspectID aspect)
{
    //--- if this could have more information about what it is making space for, data sharing could be done.
    //--- also, the functionality is split across the aspect creation and this called function, so maybe
    // redo that so sharing functionality is possible. In this, the pointer in the map could point to the same
    // mesh handle if it detects they have the same vao (assuming mesh handles are just wrapped vaos and associated stuff. So, they should be).
    manager->aspect_map[aspect.map_index] = &g_SeeingMesh_array[g_SeeingMesh_array_index];
    g_SeeingMesh_array_index ++;
    if (g_SeeingMesh_array_index >= g_SeeingMesh_array_size) {
        int old_size = g_SeeingMesh_array_size;
        SeeingMesh *old_location = g_SeeingMesh_array;
        g_SeeingMesh_array_size *= 2;
        g_SeeingMesh_array = (SeeingMesh *) realloc(g_SeeingMesh_array, g_SeeingMesh_array_size * sizeof(SeeingMesh));
        mem_check(g_SeeingMesh_array);
        memset(g_SeeingMesh_array, 0, sizeof(SeeingMesh) * (g_SeeingMesh_array_size - old_size));
        // relocate the pointers in the aspect map
        // ------ POSSIBLY BUGGY
        for (int i = 0; i < manager->aspect_map_size; i++) {
            if (manager->aspect_map[i] != NULL) {
                manager->aspect_map[i] = g_SeeingMesh_array + ((SeeingMesh *) manager->aspect_map[i] - old_location);
            }
        }
    }
}
static void SeeingMesh_destroy(Manager *manager, AspectID aspect)
{
    //- no null checking
    
    // sets type of aspect 0 in the struct itself, so iterator can skip it when it does direct iteration over the global array.
    ((SeeingMesh *) manager->aspect_map[aspect.map_index])->aspect_id.type = 0;
}
static void SeeingMesh_iterator(Iterator *iterator)
{
    Manager *manager = iterator->data1.ptr_val;
    int array_index = iterator->data2.int_val;
BEGIN_COROUTINE_SA(iterator)
coroutine_start:
    iterator->data2.int_val = 0;
    array_index = 0;
    iterator->coroutine_flag = COROUTINE_A;
coroutine_a:
    iterator->data2.int_val ++;
    for (int i = array_index; i < g_SeeingMesh_array_size; i++) {
        if (g_SeeingMesh_array[i].aspect_id.type != 0) {
            iterator->val = &g_SeeingMesh_array[i];
            iterator->data2.int_val = i + 1;
            return;
        }
    }
    iterator->val = NULL;
}
static void SeeingMesh_serialize(FILE *file, void *self)
{
    serialize_mesh_handle(file, &((SeeingMesh *) self)->mesh_handle);
}

typedef struct ObjectLogic_s {
ASPECT_PROPERTIES()
    void (*update) (struct ObjectLogic_s *, EntityID);
    void *data; //todo: and manager which knows to free this memory if used
    //------ currently leaks memory
} ObjectLogic;
static void ObjectLogic_destroy(Manager *manager, AspectID aspect)
{
    ObjectLogic *logic = (ObjectLogic *) get_aspect_data(aspect);
    if (logic->data != NULL) {
        free(logic->data);
    }
    free(logic);
}


// declarations
//--------------------------------------------------------------------------------
static EntityID make_thing(MeshHandle *thing_mesh_handle);
static UniformData get_uniform_value_aspect_ratio(void);
static void falling_sphere_update(ObjectLogic *logic, EntityID entity);
static void spawner_update(ObjectLogic *logic, EntityID entity);
static void cube_rotate(ObjectLogic *logic, EntityID entity);
static void arrow_keys_move(float speed, float *x, float *y);
static EntityID make_falling_sphere(Matrix4x4f *matrix);
//--------------------------------------------------------------------------------

// Globals -----------------------------------------------------------------------
static double ASPECT_RATIO;
// global renderer
static Renderer g_color_renderer;
static Renderer g_1920s_renderer;
// global "camera" properties
static Matrix4x4f g_view_matrix;
static Matrix4x4f g_projection_matrix;

// uniform values (done like this so the uniform-value-gotten-from-function thing works)
static Matrix4x4f g_mvp_matrix;

static bool g_printing_frames = false;
static long long int g_frames = 0;
static long long int g_second_start_frame = 0;
static float g_second_counter;

// meshes for instancing
static MeshHandle g_cube_mesh_handle;
static MeshHandle g_sphere_mesh_handle;
static MeshHandle g_dolphin_mesh_handle;
static MeshHandle g_stanford_bunny_mesh_handle;

//--------------------------------------------------------------------------------

static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_K) {
            EntityID entity; // fake an ID
            entity.uuid = 2;
            entity.map_index = 1;
            Transform *transform = get_aspect_type(entity, Transform);
            print_matrix4x4f(&transform->matrix);
        }
        else if (key == GLFW_KEY_F) {
            g_printing_frames = !g_printing_frames;
        }
    }
}

// prefabrications
//--------------------------------------------------------------------------------

typedef struct CubeData_s {
    float speed_x;
    float speed_y;
    float speed_z;
} CubeData;
static EntityID make_thing(MeshHandle *thing_mesh_handle)
{
    EntityID thing = new_entity(1);
        Transform *transform = entity_add_aspect(thing, Transform);
            identity_matrix4x4f(&transform->matrix);
            translate_matrix4x4f(&transform->matrix, 4.0*frand() - 2.0, 4.0*frand() - 2.0, 6.0*frand() - 3.0);
            /* scale_matrix4x4f(&transform->matrix, 0.5 + frand()); */
        SeeingMesh *seeing_mesh = entity_add_aspect(thing, SeeingMesh);
            memcpy(&seeing_mesh->mesh_handle, thing_mesh_handle, sizeof(MeshHandle));
            seeing_mesh->visible = true;
            /* seeing_mesh->renderer = &g_color_renderer; */
            seeing_mesh->renderer = &g_1920s_renderer;
        /* ObjectLogic *logic = entity_add_aspect(thing, ObjectLogic); */
        /*     logic->update = cube_rotate; */
        /*     logic->data = (void *) calloc(1, sizeof(CubeData)); */
        /*     CubeData *data = (CubeData *) logic->data; */
        /*     float speed_base = 10.0; */
        /*     data->speed_x = speed_base * frand() - speed_base / 2; */
        /*     data->speed_y = speed_base * frand() - speed_base / 2; */
        /*     data->speed_z = speed_base * frand() - speed_base / 2; */
    return thing;
}
static void cube_ghost_flicker(ObjectLogic *logic, EntityID entity)
{
    SeeingMesh *seeing_mesh = get_aspect_type(entity, SeeingMesh);
    if (seeing_mesh->visible && frand() > 0.9) {
        seeing_mesh->visible = false;
    } else if (!seeing_mesh->visible && frand() > 0.1) {
        Transform *transform = get_aspect_type(entity, Transform);
        euler_rotate_matrix4x4f(&transform->matrix, frand() * 2 * M_PI, frand() * 2 * M_PI, frand() * 2 * M_PI);
        seeing_mesh->visible = true;
    }
}
static void cube_rotate(ObjectLogic *logic, EntityID entity)
{
    Transform *transform = get_aspect_type(entity, Transform);
    euler_rotate_matrix4x4f(&transform->matrix, ((CubeData *)logic->data)->speed_x * dt(),
                                                ((CubeData *)logic->data)->speed_y * dt(),
                                                ((CubeData *)logic->data)->speed_z * dt());
    float x, y;
    arrow_keys_move(2.0, &x, &y);
    translate_matrix4x4f(&transform->matrix, x, y, 0.0);
    if (frand() > 0.9999) {
        // sort of like a state machine, swap the objectlogic component to another
        /* destroy_aspect(logic->aspect_id); */ //----------------
        ObjectLogic *flicker_logic = entity_add_aspect(entity, ObjectLogic);
        flicker_logic->update = cube_ghost_flicker;
        return;
    }

#if 1
    SeeingMesh *seeing_mesh = get_aspect_type(entity, SeeingMesh);
    if (transform->matrix.vals[0 + 3*4] > 2.0) seeing_mesh->visible = false; // definitely need better matrix usage.
    else seeing_mesh->visible = true;
#endif
}

static EntityID make_falling_sphere(Matrix4x4f *matrix)
{
    EntityID sphere = new_entity(1);
        Transform *transform = entity_add_aspect(sphere, Transform);
            copy_matrix4x4f(&transform->matrix, matrix);
        SeeingMesh *seeing_mesh = entity_add_aspect(sphere, SeeingMesh);
            memcpy(&seeing_mesh->mesh_handle, &g_sphere_mesh_handle, sizeof(MeshHandle));
            /* memcpy(&seeing_mesh->mesh_handle, &g_dolphin_mesh_handle, sizeof(MeshHandle)); */
            seeing_mesh->visible = true;
            /* seeing_mesh->renderer = &g_1920s_renderer; */
#if 1
            if (frand() > 0.5) {
                seeing_mesh->renderer = &g_color_renderer;
            } else {
                seeing_mesh->renderer = &g_1920s_renderer;
            }
#else
            seeing_mesh->renderer = &g_1920s_renderer;
#endif
        ObjectLogic *logic = entity_add_aspect(sphere, ObjectLogic);
            logic->update = falling_sphere_update;
    return sphere;
}
void falling_sphere_update(ObjectLogic *logic, EntityID entity)
{
    Transform *transform = get_aspect_type(entity, Transform);
    /* translate_matrix4x4f(&transform->matrix, 0, -0.3 * dt(), 0); */
    translate_matrix4x4f(&transform->matrix, frand() * dt(), frand() * dt(), frand() * dt());
}


typedef struct SpawnerData_s {
    float offset_theta_x;
    float offset_theta_y;
    float offset_theta_z;
} SpawnerData;
static EntityID make_spawner(float offset_theta_x, float offset_theta_y, float offset_theta_z)
{
    EntityID spawner = new_entity(1);
    Transform *transform = entity_add_aspect(spawner, Transform);
    identity_matrix4x4f(&transform->matrix);
    ObjectLogic *logic = entity_add_aspect(spawner, ObjectLogic);
    logic->update = spawner_update;
    logic->data = (void *) calloc(1, sizeof(SpawnerData));
    SpawnerData *data = (SpawnerData *) logic->data;
    data->offset_theta_x = offset_theta_x;
    data->offset_theta_y = offset_theta_y;
    data->offset_theta_z = offset_theta_z;
    return spawner;
}
static void spawner_update(ObjectLogic *logic, EntityID entity)
{
    SpawnerData *props = (SpawnerData *) logic->data;
    Transform *transform = get_aspect_type(entity, Transform);
    identity_matrix4x4f(&transform->matrix);
    translate_matrix4x4f(&transform->matrix, 1.3 * sin(props->offset_theta_x + 0.8 * time()),
                                             1.3 * cos(props->offset_theta_y + 1.3 * time()),
                                             1.3 * sin(props->offset_theta_z + 2.1 * time() + 1.3));
    make_falling_sphere(&transform->matrix);
}


// object logic helper functions
//--------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------

static UniformData get_uniform_value_aspect_ratio(void)
{
    UniformData data;
    data.float_value = ASPECT_RATIO;
    return data;
}
//--- union on the level of the function pointers instead may allow functions to be passed directly (frand is a macro, though)
static UniformData get_uniform_value_frand(void)
{
    UniformData data;
    data.float_value = frand();
    return data;
}
static UniformData get_uniform_value_mvp_matrix(void)
{
    UniformData data;
    memcpy(data.mat4x4_value, g_mvp_matrix.vals, 4 * 4 * sizeof(float));
    return data;
}

void init_program(void)
{
    new_renderer_vertex_fragment(&g_color_renderer, VERTEX_FORMAT_3C, SHADERS_LOCATION "cube.vert", SHADERS_LOCATION "cube.frag");
    renderer_add_uniform(&g_color_renderer, "aspect_ratio", get_uniform_value_aspect_ratio, UNIFORM_FLOAT);
    renderer_add_uniform(&g_color_renderer, "frand", get_uniform_value_frand, UNIFORM_FLOAT);
    renderer_add_uniform(&g_color_renderer, "mvp_matrix", get_uniform_value_mvp_matrix, UNIFORM_MAT4X4);

    new_renderer_vertex_fragment(&g_1920s_renderer, VERTEX_FORMAT_3, SHADERS_LOCATION "sphere.vert", SHADERS_LOCATION "sphere.frag");
    renderer_add_uniform(&g_1920s_renderer, "aspect_ratio", get_uniform_value_aspect_ratio, UNIFORM_FLOAT);
    renderer_add_uniform(&g_1920s_renderer, "frand", get_uniform_value_frand, UNIFORM_FLOAT);
    renderer_add_uniform(&g_1920s_renderer, "mvp_matrix", get_uniform_value_mvp_matrix, UNIFORM_MAT4X4);

    identity_matrix4x4f(&g_view_matrix);
    identity_matrix4x4f(&g_projection_matrix);

    init_entity_model();

    

    // managers
    new_default_manager(Transform, Transform_serialize);
    new_manager(SeeingMesh, SeeingMesh_new, SeeingMesh_destroy, SeeingMesh_iterator, SeeingMesh_serialize);
    /* new_manager(SeeingMesh, SeeingMesh_new, SeeingMesh_destroy, default_manager_aspect_iterator, SeeingMesh_serialize); */
    initialize_SeeingMesh_manager();
    new_manager(ObjectLogic, default_manager_new_aspect, ObjectLogic_destroy, default_manager_aspect_iterator, NULL);

    // global mesh data for instancing
    { 
      Mesh mesh;
      create_cube_mesh(&mesh, 0.2);
      upload_mesh(&g_cube_mesh_handle, &mesh);
    }
    { 
      Mesh mesh;
      make_sphere(&mesh, 0.2, 5);
      upload_mesh(&g_sphere_mesh_handle, &mesh);
    }
    //================================================================================
    // Test mesh loading
    //================================================================================
    {
        Mesh dolphin_mesh;
        load_mesh_ply(&dolphin_mesh, VERTEX_FORMAT_3, DATA_DIR "models/dolphins.ply", 0.01);
        upload_and_free_mesh(&g_dolphin_mesh_handle, &dolphin_mesh);
    }
    {
        Mesh stanford_bunny_mesh;
        load_mesh_ply(&stanford_bunny_mesh, VERTEX_FORMAT_3, DATA_DIR "models/stanford_bunny.ply", 5);
        upload_and_free_mesh(&g_stanford_bunny_mesh_handle, &stanford_bunny_mesh);
    }

    // make some stuff
    for (int i = 0; i < 10; i++) {
    /* for (int i = 0; i < 10000; i++) { */
        /* EntityID new_thing = make_thing(&g_cube_mesh_handle); */
        if (frand() > 0.5) {
            EntityID new_thing = make_thing(&g_dolphin_mesh_handle);
        } else {
            EntityID new_thing = make_thing(&g_stanford_bunny_mesh_handle);
        }
        /* Transform *transform = get_aspect_type(new_thing, Transform); */
        /* translate_matrix4x4f(&transform->matrix, 2.0*frand() - 1.0, 2.0*frand() - 1.0, -6.0*frand()); */
        /* printf("[Created new entity]\n"); */
        /* print_entity(cube); */
        /* getchar(); */
    }

    // make spawners
#if 0
    for (int i = 0; i < 5; i++) {
        make_spawner(frand() * 2 * M_PI, frand() * 2 * M_PI, frand() * 2 * M_PI);
    }
#endif
    /* print_entities(); */

    printf("global renderer:\n");
    print_renderer(&g_color_renderer);
    print_renderer(&g_1920s_renderer);
    print_vertex_attribute_types();

    /* print_aspects_of_type(Transform); */
    /* print_aspects_of_type(ObjectLogic); */
    /* print_aspects_of_type(SeeingMesh); */
}




void loop(GLFWwindow *window)
{
    g_frames ++;
    g_second_counter += dt();
    if (g_second_counter >= 1.0) {
        if (g_printing_frames) {
            printf("Approx frames per second: %lld\n", g_frames - g_second_start_frame);
        }
        g_second_counter = 0;
        g_second_start_frame = g_frames;
    }




    if (alt_arrow_key_down(Up)) {
        euler_rotate_matrix4x4f(&g_view_matrix, -2 * dt(), 0, 0);
    }
    if (alt_arrow_key_down(Down)) {
        euler_rotate_matrix4x4f(&g_view_matrix, 2 * dt(), 0, 0);
    }
    if (alt_arrow_key_down(Left)) {
        euler_rotate_matrix4x4f(&g_view_matrix, 0, 2 * dt(), 0);
    }
    if (alt_arrow_key_down(Right)) {
        euler_rotate_matrix4x4f(&g_view_matrix, 0, -2 * dt(), 0);
    }

    for_aspect(SeeingMesh, seeing_mesh)
        if (seeing_mesh->visible) {
            Transform *transform = get_aspect_type(seeing_mesh->entity_id, Transform);
            // Prepare mvp_matrix for this render (it is a uniform of the renderer)
            copy_matrix4x4f(&g_mvp_matrix, &transform->matrix);
            right_multiply_matrix4x4f(&g_mvp_matrix, &g_view_matrix);
            right_multiply_matrix4x4f(&g_mvp_matrix, &g_projection_matrix);
            render_mesh(seeing_mesh->renderer, &seeing_mesh->mesh_handle, GL_TRIANGLES);
        }
    end_for_aspect()

    for_aspect(ObjectLogic, logic)
        logic->update(logic, logic->entity_id);
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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    ASPECT_RATIO = SCREEN_ASPECT_RATIO;
    glClearColor(1.0, 0.8, 0.8, 1.0);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop, GL_COLOR_BUFFER_BIT);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
