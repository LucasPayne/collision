/*--------------------------------------------------------------------------------
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
    void cursor_move_event(double x, double y);
    void mouse_button_event(int button, int action, int mods);

project_libs:
    + glad
    + helper_definitions
    + helper_gl
    + helper_input
    + memory
    + glsl_utilities
    + data_dictionary
    + matrix_mathematics
    + entity
    + iterator
    + resources
    + rendering
    + ply
    + painting
--------------------------------------------------------------------------------*/
#define BASE_DIRECTORY "/home/lucas/collision/lib/Engine/"
#define PROJECT_DIRECTORY "/home/lucas/collision/"
#include "Engine.h"

static GLFWwindow *window;

DataDictionary *g_scenes;
DataDictionary *g_data; // Global data dictionary for the application.
float ASPECT_RATIO;
float mouse_x;
float mouse_y;

static int window_width = 1;
static int window_height = 1;
static int subwindow_blx = 0;
static int subwindow_bly = 0;
static int subwindow_trx = 1;
static int subwindow_try = 1;
//---
vec2 pixel_to_rect(int pixel_x, int pixel_y, float blx, float bly, float trx, float try)
{    
    // This mapping takes into account the subrectangle of the window that the aspect ratio forces.

    // Given integer pixel coordinates measured from the top-left of the screen, (0,0), map these
    // to floating point coordinates where (blx, bly) is the bottom left-corner of a rectangle
    // in screen coordinates (0.0,0.0) at bottom left and (1.0,1.0) at top right, and (trx, try) is the top
    // right corner.
    // Example:
    //    pixel_x: 100, pixel_y: 100, blx: 0.1, bly: 0.1, trx: 0.9, try: 0.9.
    //    Say the window resolution is 1000x1000, then the pixel is 90 percent up and to the left
    //    of the screen. In rectangle coordinates, this is (0,1), since those range from (0.1,0.1),
    //    a point 90 percent to the left and 10 percent up, to (0.9,0.9), a point 90 percent to the right
    //    and 90 percent up.
    if (blx == trx || bly == try) {
        fprintf(stderr, ERROR_ALERT "Gave pixel_to_rect a degenerate rectangle. This causes a division by zero.\n");
        exit(EXIT_FAILURE);
    }

    // First, convert to the floating point "screen coordinates".
    float sx = ((pixel_x - subwindow_blx) * 1.0) / (subwindow_trx - subwindow_blx);
    float sy = 1 - ((pixel_y - subwindow_bly) * 1.0) / (subwindow_try - subwindow_bly);
    
    // Next, convert to the rectangle coordinates.
    float x = -(blx - sx) / (trx - blx);
    float y = -(bly - sy) / (try - bly);
    return new_vec2(x, y);
}

static const int g_glfw_sma_debug_overlay_key = GLFW_KEY_F11; // debugging small memory allocator.
static bool g_sma_debug_overlay = false;
static bool g_raw_mouse = false; // If enabled in config, a meta-key will toggle this.
static const int g_glfw_raw_mouse_key = GLFW_KEY_F12;
static bool g_freeze_shadows = false; // can toggle this for debugging.
static const int g_glfw_freeze_shadows_key = GLFW_KEY_F10;
static bool g_test_toggle = false;
static const int g_glfw_test_toggle_key = GLFW_KEY_F9;
static void toggle_raw_mouse(void)
{
    ///////--- Why does it only toggle once?
    if (g_raw_mouse) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    g_raw_mouse = !g_raw_mouse;
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    // Call the application's mouse button event handler.
    mouse_button_event(button, action, mods);
}

static void cursor_position_callback(GLFWwindow *window, double x, double y)
{
    static bool set_last = false;
    if (!set_last) { // so that the first relative position is 0,0.
        set_last = true;
        mouse_x = x;
        mouse_y = y;
    }

    // Send input events to Input aspects listening for mouse absolute or relative movements.
    for_aspect(Input, inp)
        if (inp->input_type == INPUT_MOUSE_POSITION) {
            inp->callback.mouse_position(inp, x, y);
        } else if (inp->input_type == INPUT_MOUSE_MOVE) {
            inp->callback.mouse_move(inp, x - mouse_x, y - mouse_y);
        }
    end_for_aspect()

    // Call the application's mouse movement event handler.
    cursor_move_event(x, y);

    mouse_x = x;
    mouse_y = y;
}

float time = 0;
float dt = 0;

extern void init_program(void);
extern void loop_program(void);
extern void close_program(void);
extern void input_event(int key, int action, int mods);
extern void cursor_move_event(double x, double y);
extern void mouse_button_event(int button, int action, int mods);

static void reshape(GLFWwindow *window, int width, int height)
{
    window_width = width;
    window_height = height;
    force_aspect_ratio(window, width, height, ASPECT_RATIO);
}
static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);

    // Meta-keys (debug, input, etc.)
    if (g_raw_mouse) { // if the raw mouse option is enabled, it is toggleable with a meta-key.
        if (action == GLFW_PRESS && key == g_glfw_raw_mouse_key) {
            toggle_raw_mouse();
        }
    }
    if (action == GLFW_PRESS) {
        if (key == g_glfw_sma_debug_overlay_key) g_sma_debug_overlay = !g_sma_debug_overlay;
        if (key == g_glfw_freeze_shadows_key) g_freeze_shadows = !g_freeze_shadows;
        if (key == g_glfw_test_toggle_key) {
            // toggleable bool for testing shaders.
            g_test_toggle = !g_test_toggle;
            set_uniform_bool(Standard3D, test_toggle, g_test_toggle);
        }
    }

    // Send input events to Input aspects listening for keys.
    for_aspect(Input, inp)
        if (inp->input_type == INPUT_KEY) {
            inp->callback.key(inp, key, action, mods);
        }
    end_for_aspect()

    input_event(key, action, mods);
}

static void init_base(void)
{
    //---
    glLineWidth(20);

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
        { 3, 1 << (10 - 3)},
        { 4, 1 << (10 - 4)},
        { 5, 1 << (10 - 5)},
        { 6, 1 << (10 - 6)},
        { 7, 1 << (10 - 7)},
        { 8, 1 << (10 - 8)},
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
    add_shader_block(Lights);
    
    // Initialize the entity and aspect system, and load type data for aspects
    // forming the "GameObject" library.
    init_entity_model();
    init_aspects_gameobjects();

    init_resources_rendering();

    glsl_include_path_add(PROJECT_DIRECTORY "glsl/shader_blocks");
    
    // Project-wide source/asset directories.
    resource_path_add("Meshes", "/home/lucas/collision/project_resources/meshes");
    resource_path_add("Images", "/home/lucas/collision/project_resources/images");
    resource_path_add("Shaders", "/home/lucas/collision/project_resources/shaders");
    resource_path_add("Fonts", "/home/lucas/collision/project_resources/fonts");
    // Application-specific source/asset directories (these directories need not exist.)
    resource_path_add("Meshes", "resources/meshes");
    resource_path_add("Images", "resources/images");
    resource_path_add("Shaders", "resources/shaders");
    resource_path_add("Fonts", "resources/fonts");

    init_game_renderer();

    // Initialize global test textures that shaders can access.
    ResourceHandle test_texture = new_resource_handle(Texture, "Textures/test_texture");
    set_uniform_texture(Standard3D, test_texture, resource_data(Texture, test_texture)->texture_id);
    ResourceHandle test_texture_2 = new_resource_handle(Texture, "Textures/test_texture_2");
    set_uniform_texture(Standard3D, test_texture_2, resource_data(Texture, test_texture_2)->texture_id);
    // Initialize global test toggle bool to false. This is switched with a meta-key, and can be used in shaders for testing.
    g_test_toggle = false; //---currently can not read back the written uniform values.
    set_uniform_bool(Standard3D, test_toggle, false);
}

static void loop_base(void)
{
    // Update entity logic
    for_aspect(Logic, logic)
        logic->update(logic);
    end_for_aspect()

    // Handle lights
{
    // Directional lights
    int index = 0;
    for_aspect(DirectionalLight, directional_light)
        if (index >= MAX_NUM_DIRECTIONAL_LIGHTS) {
            fprintf(stderr, ERROR_ALERT "scene error: Too many directional lights have been created. The maximum number is set to %d.\n", MAX_NUM_DIRECTIONAL_LIGHTS);
            exit(EXIT_FAILURE);
        }
        vec3 dir = DirectionalLight_direction(directional_light);
        set_uniform_vec3(Lights, directional_lights[index].direction, dir);
        set_uniform_vec4(Lights, directional_lights[index].color, directional_light->color);

        index ++;
    end_for_aspect()
    set_uniform_int(Lights, num_directional_lights, index);
}
{   
    // Point lights
    int index = 0;
    for_aspect(PointLight, point_light)
        if (index >= MAX_NUM_POINT_LIGHTS) {
            fprintf(stderr, ERROR_ALERT "scene error: Too many point lights have been created. The maximum number is set to %d.\n", MAX_NUM_POINT_LIGHTS);
            exit(EXIT_FAILURE);
        }
        Transform *t = get_sibling_aspect(point_light, Transform);
        set_uniform_vec3(Lights, point_lights[index].position, new_vec3(t->x, t->y, t->z));
        set_uniform_vec4(Lights, point_lights[index].color, point_light->color);
        set_uniform_float(Lights, point_lights[index].linear_attenuation, point_light->linear_attenuation);
        set_uniform_float(Lights, point_lights[index].quadratic_attenuation, point_light->quadratic_attenuation);
        set_uniform_float(Lights, point_lights[index].cubic_attenuation, point_light->cubic_attenuation);
        index ++;
    end_for_aspect()
    set_uniform_int(Lights, num_point_lights, index);
}
    render();
    painting_flush(Canvas3D);

    // Debug rendering
    if (g_sma_debug_overlay) small_memory_allocator_debug_overlay();

    loop_program();
}


#define config_error(str)\
    { fprintf(stderr, ERROR_ALERT "Application configuration error: non-existent or malformed \"" str "\" entry.\n");\
      exit(EXIT_FAILURE); }
int main(void)
{
    DD *base_config = dd_fopen(BASE_DIRECTORY "Engine.dd");
    if (base_config == NULL) {
        printf("Base directory: " BASE_DIRECTORY "\n");
        fprintf(stderr, ERROR_ALERT "Application base is missing its .dd file.\n");
        exit(EXIT_FAILURE);
    }
    // Set the resource dictionary.
    g_resource_dictionary = dd_open(base_config, "Resources");
    if (g_resource_dictionary == NULL) {
        fprintf(stderr, ERROR_ALERT "Could not open resource dictionary.\n");
        exit(EXIT_FAILURE);
    }
    // Set the scene dictionary.
    g_scenes = dd_open(base_config, "Scenes");
    if (g_scenes == NULL) {
        fprintf(stderr, ERROR_ALERT "Could not open scenes dictionary.\n");
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
    window = glfwCreateWindow(1, 1, name, NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, ERROR_ALERT "GLFW error: failed to create a window properly.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, reshape);
    // GLFW input callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    GLbitfield clear_mask = GL_COLOR_BUFFER_BIT; // To be |='d if another buffer is being used.
    float fg_color[4]; // Clear color in the rectangle being rendered onto.
    if (!dd_get(app_config, "fg_color", "vec4", fg_color)) config_error("fg_color");
    float bg_color[4]; // Background behind rectangle, visible if the window proportions do not match the aspect ratio.
    if (!dd_get(app_config, "bg_color", "vec4", bg_color)) config_error("bg_color");

    if (!dd_get(app_config, "aspect_ratio", "float", &ASPECT_RATIO)) config_error("aspect_ratio");

    char *cull_mode;
    if (!dd_get(app_config, "cull_mode", "string", &cull_mode)) config_error("cull_mode");
    if (strcmp(cull_mode, "back") == 0) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    } else if (strcmp(cull_mode, "front") == 0) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
    } else if (strcmp(cull_mode, "none") == 0) {
        // no culling
    } else {
        fprintf(stderr, "Invalid cull_mode option set. The options are \"front\", \"back\", and \"none\".\n");
        exit(EXIT_FAILURE);
    }
    free(cull_mode);

    bool depth_test;
    if (!dd_get(app_config, "depth_test", "bool", &depth_test)) config_error("depth_test");
    if (depth_test) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        clear_mask |= GL_DEPTH_BUFFER_BIT; // Must now clear the depth buffer each frame.
    } else {
        // Nothing needs to be done.
    }

    // Raw mouse input enable/disable, e.g. for 3D camera control.
    // g_raw_mouse is a global variable so that it may be toggled with a meta-key.
    bool raw_mouse;
    if (!dd_get(app_config, "raw_mouse", "bool", &raw_mouse)) config_error("raw_mouse");
    if (raw_mouse) {
        // details: https://www.glfw.org/docs/latest/input_guide.html#input_mouse
        if (glfwRawMouseMotionSupported()) {
            g_raw_mouse = false; // make sure its switched off.
            toggle_raw_mouse();  // now switch it on.
        } else { 
            fprintf(stderr, ERROR_ALERT "GLFW error: Could not find support for raw mouse motion.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        g_raw_mouse = false;
    } 

    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    init_base();

    init_program();
    double last_time = time;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        last_time = time;
        time = glfwGetTime();
        dt = time - last_time;

        // Clearing: window clear to background color, viewport clear to the foreground color.
        
        glClearColor(bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
        glDisable(GL_SCISSOR_TEST);
        glClear(clear_mask);

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glEnable(GL_SCISSOR_TEST);
        glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
        subwindow_blx = viewport[0];
        subwindow_bly = viewport[1];
        subwindow_trx = viewport[0] + viewport[2];
        subwindow_try = viewport[1] + viewport[3];
        glClearColor(fg_color[0], fg_color[1], fg_color[2], fg_color[3]);
        glClear(clear_mask);

        glDisable(GL_SCISSOR_TEST);
        loop_base();

        glFlush();
        glfwSwapBuffers(window);
    }
    // Cleanup
    close_program();
    glfwDestroyWindow(window);
    glfwTerminate();
}
