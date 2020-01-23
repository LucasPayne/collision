/*--------------------------------------------------------------------------------
Application base.
Add this as a library in an application, and it will provide a main function.
The application must provide:
    Data
    ----
    conf.dd:
        A file containing application configuration, to be included as app_config. This is written to the ApplicationConfiguration type interface declared
        in this base's .dd file. This is for things like the window name, size, OpenGL version, etc.
    resources.dd:
        This will be included as Resources, and that dictionary will be set to the resource system's dictionary to search for resources.
    data.dd:
        Any extra data the application wants to include is sourced from here.
    Code
    ----
    void init_program(void);
    void loop_program(void);
    void close_program(void);
    void input_event(int key, int action, int mods);
    

base_libs:
    + glad
    + helper_definitions
    + helper_gl
    + helper_input
    + memory
    + glsl_utilities
    + data_dictionary
    + matrix_mathematics
    + entity
    + aspect_library/gameobjects
    + iterator
    + resources
    + rendering
    + ply
--------------------------------------------------------------------------------*/
#define BASE_DIRECTORY "/home/lucas/collision/lib/bases/interactive_3D/"
#define PROJECT_DIRECTORY "/home/lucas/collision/"
#include "bases/interactive_3D.h"

DataDictionary *g_data; // Global data dictionary for the application.
float ASPECT_RATIO;

float time = 0;
float dt = 0;

extern void init_program(void);
extern void loop_program(void);
extern void close_program(void);
extern void input_event(int key, int action, int mods);

#define config_error(str)\
    { fprintf(stderr, ERROR_ALERT "Application configuration error: non-existent or malformed \"" str "\" entry.\n");\
      exit(EXIT_FAILURE); }

static void reshape(GLFWwindow *window, int width, int height)
{
    force_aspect_ratio(window, width, height, ASPECT_RATIO);
}
static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);

    // Send input events to Input aspects (listeners and handlers).
    for_aspect(Input, inp)
        if (inp->listening) {
            inp->callback(inp, key, action, mods);
        }
    end_for_aspect()

    input_event(key, action, mods);
}

static void init_base(void)
{
    /*--------------------------------------------------------------------------------
        Program memory.
    --------------------------------------------------------------------------------*/
    // Allocate program memory. Ideally this will be used instead of malloc.
    /////?? Why is this number incorrect? It is small enough to fit in a 32-bit int as well.
    // printf("Allocating %zu\n bytes\n", (size_t) bytes_GB(2));getchar();
    // size_t num_bytes = 1024*1024*1024*( 2 );
    // printf("Allocating %zu\n bytes\n", num_bytes);
    // mem_init(2147483648); // 2GB
    mem_init(1000000000); // ~0.9313 GB

    // Initialize the small memory allocator, where, for example, the resource data will be allocated.
    // (A small memory allocator is a pool consisting of multiple pools, each with power-of-two cell sizes. The allocation routine
    //  infers from the size what pool to use.)
    static const SMAPoolInfo sma_pool_info[] = { // Edit this to change the available pool sizes.
        { 3, 1 << (12 - 3)},
        { 4, 1 << (12 - 4)},
        { 5, 1 << (12 - 5)},
        { 6, 1 << (12 - 6)},
        { 7, 1 << (12 - 7)},
        { 8, 1 << (12 - 8)},
        { 12, 128 }, // 4 KB for "big resources".
    };
    static const int num_sma_pools = sizeof(sma_pool_info)/sizeof(SMAPoolInfo);
    init_small_memory_allocator(sma_pool_info, num_sma_pools);

    /*--------------------------------------------------------------------------------
        Non window/context initialization.
    --------------------------------------------------------------------------------*/
    // Create the basic shader blocks.
    add_shader_block(MaterialProperties); // The definition of this block is part of the rendering module.
    add_shader_block(StandardLoopWindow);
    add_shader_block(Standard3D);
    // add_shader_block(Lights);
    
    // Initialize the entity and aspect system, and load type data for aspects
    // forming the "GameObject" library.
    init_entity_model();
    init_aspects_gameobjects();

    init_resources_rendering();

    glsl_include_path_add(PROJECT_DIRECTORY "glsl/shader_blocks");
}

static void render(void)
{
    set_uniform_float(StandardLoopWindow, time, time);
    for_aspect(Camera, camera)
        Transform *camera_transform = get_sibling_aspect(camera, Transform);
        Matrix4x4f view_matrix = Transform_matrix(camera_transform);
        Matrix4x4f vp_matrix = camera->projection_matrix;
        right_multiply_matrix4x4f(&vp_matrix, &view_matrix);

        for_aspect(Body, body)
            Transform *transform = get_sibling_aspect(body, Transform);
            Geometry *mesh = resource_data(Geometry, body->geometry);

            Material *material = resource_data(Material, body->material);
            Matrix4x4f model_matrix = Transform_matrix(transform);
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    model_matrix.vals[4*i + j] *= body->scale;
                }
            }
            Matrix4x4f mvp_matrix = vp_matrix;
            right_multiply_matrix4x4f(&mvp_matrix, &model_matrix);

            set_uniform_mat4x4(Standard3D, mvp_matrix.vals, mvp_matrix.vals);

            gm_draw(*mesh, material);
        end_for_aspect()
    end_for_aspect()
}

static void loop_base(void)
{
    for_aspect(Logic, logic)
        logic->update(logic);
    end_for_aspect()

    render();
    loop_program();
}

int main(void)
{
    DD *base_config = dd_fopen(BASE_DIRECTORY "interactive_3D.dd");
    if (base_config == NULL) {
        fprintf(stderr, ERROR_ALERT "Application base is missing its .dd file.\n");
        exit(EXIT_FAILURE);
    }
    // Set the resource dictionary.
    g_resource_dictionary = dd_open(base_config, "Resources");
    if (g_resource_dictionary == NULL) {
        fprintf(stderr, ERROR_ALERT "Could not open resource dictionary.\n");
        exit(EXIT_FAILURE);
    }
    DD *app_config = dd_open(base_config, "app_config");
    if (app_config == NULL) {
        fprintf(stderr, ERROR_ALERT "Missing app_config.\n");
        exit(EXIT_FAILURE);
    }
    g_data = dd_open(base_config, "Data");
    if (g_data == NULL) {
        fprintf(stderr, ERROR_ALERT "Could not open data dictionary.\n");
        exit(EXIT_FAILURE);
    }

    if (!glfwInit()) {
        fprintf(stderr, "GLFW error: something went wrong initializing GLFW\n");
        exit(EXIT_FAILURE);
    }
    int gl_version[2];
    if (!dd_get(app_config, "gl_version", "ivec2", gl_version)) config_error("gl_version");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_version[0]);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_version[1]);
    // GLFW_OPENGL_PROFILE:     GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_COMPAT_PROFILE or GLFW_OPENGL_CORE_PROFILE
    bool core_profile;
    if (!dd_get(app_config, "core_profile", "bool", &core_profile)) config_error("core_profile");
    if (core_profile) glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    else              glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    char *name = "Window";
    GLFWwindow *window = glfwCreateWindow(1, 1, name, NULL, NULL);
    if (!window) {
        fprintf(stderr, ERROR_ALERT "GLFW error: failed to create a window properly.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);

    float clear_color[4];
    if (!dd_get(app_config, "clear_color", "vec4", clear_color)) config_error("clear_color");
    glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
    glfwSetFramebufferSizeCallback(window, reshape);
    glfwSetKeyCallback(window, key_callback);

    if (!dd_get(app_config, "aspect_ratio", "float", &ASPECT_RATIO)) config_error("aspect_ratio");

    init_base();

    init_program();
    double last_time = time;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        last_time = time;
        time = glfwGetTime();
        dt = time - last_time;

        /* Clearing: window clear to black, viewport clear to the clear colour.
         * (restore clear colour after window clear)
         */
        glClear(GL_COLOR_BUFFER_BIT);
        GLfloat clear_color[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT); //----config: clear mask

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glEnable(GL_SCISSOR_TEST);
        glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
        glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT); //----config: clear mask

        loop_base();

        glFlush();
        glfwSwapBuffers(window);
    }
    // Cleanup
    close_program();
    glfwDestroyWindow(window);
    glfwTerminate();
}
