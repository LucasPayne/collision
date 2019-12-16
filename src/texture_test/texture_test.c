/*================================================================================
PROJECT_LIBS:
    + glad
    + helper_gl
    + helper_input
    + dictionary
    + resources
    + rendering
    + ply
    + iterator
    + entity
    + matrix_mathematics
    + aspect_library/gameobjects
================================================================================*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "helper_definitions.h"
#include "helper_gl.h"
#include "helper_input.h"
#include "ply.h"
#include "rendering.h"
#include "resources.h"
#include "entity.h"
#include "matrix_mathematics.h"
#include "aspect_library/gameobjects.h"


ShaderBlockID ShaderBlockID_StandardLoopWindow;
typedef struct ShaderBlock_StandardLoopWindow_s {
    float aspect_ratio;
    float time;
} ShaderBlock_StandardLoopWindow;

ShaderBlockID ShaderBlockID_Standard3D;
typedef struct ShaderBlock_Standard3D_s {
    float aspect_ratio;
    float time;
} ShaderBlock_Standard3D;

ShaderBlockID ShaderBlockID_DirectionalLights;
typedef struct ShaderBlock_DirectionalLights_s {
    float aspect_ratio;
    float time;
} ShaderBlock_DirectionalLights;


static float ASPECT_RATIO;

void init_program(void)
{
    init_resources_rendering();
    resource_path_add("Textures", "/home/lucas/code/collision/src/texture_test/textures");
    resource_path_add("Shaders", "/home/lucas/code/collision/src/texture_test/shaders");
    resource_path_add("Models", "/home/lucas/code/collision/data/meshes");
    resource_path_add("Materials", "/home/lucas/code/collision/src/texture_test/materials");

    add_shader_block(StandardLoopWindow);
    add_shader_block(Standard3D);
    add_shader_block(DirectionalLights);

    set_uniform_float(StandardLoopWindow, aspect_ratio, ASPECT_RATIO);

    ResourceHandle mt = new_resource_handle(MaterialType, "Materials/simple");
    resource_data(MaterialType, mt);

    /* printf("%.2f\n", ((ShaderBlock_StandardGlobal *) &g_shader_blocks[ShaderBlockID_StandardGlobal].shader_block)->aspect_ratio); */

    /* ResourceHandle mt = new_resource_handle(MaterialType, "Materials/textured_phong"); */
    /* printf("%s\n", resource_data(MaterialType, mt)->texture_names[0]); */


#if 0
    ResourceHandle res_artist = new_resource_handle(Artist, "Virtual/artist");
    Artist *artist = resource_data(Artist, res_artist);
    artist->graphics_program = new_resource_handle(GraphicsProgram, "Shaders/texturing");
    Artist_add_uniform(artist, "time", (UniformGetter) uniform_get_time, UNIFORM_FLOAT);

    init_entity_model();
    init_aspects_gameobjects();
    {
        EntityID entity = new_entity(2);
        Body_init(entity_add_aspect(entity, Body), "Virtual/artist", "Models/cube");
        Transform_set(entity_add_aspect(entity, Transform), 0,0,0, 0,0,0);
    }
#endif
}
void loop(void)
{
    set_uniform_float(StandardLoopWindow, time, time());


#if 0
    for_aspect(Body, body)
        Material *material = resource_data(Material, body->material);
        Mesh *mesh = resource_data(Mesh, body->mesh);

        Artist_draw_mesh(artist, mesh);
    end_for_aspect()
#endif
}
void close_program(void)
{
}
static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
#define CASE(ACTION,KEY)\
    if (action == ( GLFW_ ## ACTION ) && key == ( GLFW_KEY_ ## KEY ))
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);
#if 0
    CASE(PRESS, C) {
        // Should probably make a for_resource_type or for_resource.
        for_aspect(Body, body)
            GraphicsProgram_reload(resource_data(Artist, body->artist)->graphics_program);
        end_for_aspect()
    }
#endif
#undef CASE
}

void reshape(GLFWwindow* window, int width, int height)
{
    force_aspect_ratio(window, width, height, ASPECT_RATIO);
}
int main(int argc, char *argv[])
{
    GLFWwindow *window;
    char *name = "Texture testing";
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
