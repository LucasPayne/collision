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
    void cursor_move_event(double x, double y);

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
    + scenes
--------------------------------------------------------------------------------*/
#define BASE_DIRECTORY "/home/lucas/collision/lib/bases/interactive_3D/"
#define PROJECT_DIRECTORY "/home/lucas/collision/"
#include "bases/interactive_3D.h"

static GLFWwindow *window;

DataDictionary *g_scenes;
DataDictionary *g_data; // Global data dictionary for the application.
float ASPECT_RATIO;

static const int g_glfw_sma_debug_overlay_key = GLFW_KEY_F11; // debugging small memory allocator.
static bool g_sma_debug_overlay = false;
static bool g_raw_mouse = false; // If enabled in config, a meta-key will toggle this.
static const int g_glfw_raw_mouse_key = GLFW_KEY_F12;
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
static void cursor_position_callback(GLFWwindow *window, double x, double y)
{
    static bool set_last = false;
    static double last_x;
    static double last_y;
    if (!set_last) { // so that the first relative position is 0,0.
        set_last = true;
        last_x = x;
        last_y = y;
    }

    // Send input events to Input aspects listening for mouse absolute or relative movements.
    for_aspect(Input, inp)
        if (inp->input_type == INPUT_MOUSE_POSITION) {
            inp->callback.mouse_position(inp, x, y);
        } else if (inp->input_type == INPUT_MOUSE_MOVE) {
            inp->callback.mouse_move(inp, x - last_x, y - last_y);
        }
    end_for_aspect()

    // Call the application's move event handler.
    cursor_move_event(x, y);

    last_x = x;
    last_y = y;
}

float time = 0;
float dt = 0;

extern void init_program(void);
extern void loop_program(void);
extern void close_program(void);
extern void input_event(int key, int action, int mods);
extern void cursor_move_event(double x, double y);

#define SHADOW_MAP_TEXTURE_WIDTH 512
#define SHADOW_MAP_TEXTURE_HEIGHT 512

// Currently only doing directional light shadows.
typedef struct ShadowMap_s {
    GLuint framebuffer;
    GLuint depth_texture;
    GLuint color_texture;
    // For debugging purposes, the shadow map keeps a resource handle for a material which has the depth texture attached.
    // This won't be destroyed, and can be used to render the depth map to a quad.
    ResourceHandle depth_texture_material; // Resource: Material
} ShadowMap;
static ShadowMap g_directional_light_shadow_maps[MAX_NUM_DIRECTIONAL_LIGHTS];
static Material *g_shadow_map_material = NULL;


static void init_shadows(void)
{
    // Load the shadow depth-pass shaders into a material.
    ResourceHandle shadow_map_material_handle = Material_create("Materials/shadows");
    //ResourceHandle shadow_map_material_handle = Material_create("Materials/red");
    g_shadow_map_material = resource_data(Material, shadow_map_material_handle);
    // Force-load the shadow depth-pass material-type.
    resource_data(MaterialType, g_shadow_map_material->material_type);

    // For each directional light slot, initialize its shadow map.
    for (int i = 0; i < MAX_NUM_DIRECTIONAL_LIGHTS; i++) {
        ShadowMap *shadow_map = &g_directional_light_shadow_maps[i];
        glGenTextures(1, &shadow_map->color_texture);
        glBindTexture(GL_TEXTURE_2D, shadow_map->color_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SHADOW_MAP_TEXTURE_WIDTH, SHADOW_MAP_TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glGenTextures(1, &shadow_map->depth_texture);
        glBindTexture(GL_TEXTURE_2D, shadow_map->depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_TEXTURE_WIDTH, SHADOW_MAP_TEXTURE_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        shadow_map->depth_texture_material = Material_create("Materials/render_shadow_map");
        ResourceHandle depth_texture_handle;
        Texture *depth_texture = oneoff_resource(Texture, depth_texture_handle);
        depth_texture->texture_id = shadow_map->depth_texture;
        material_set_texture(resource_data(Material, shadow_map->depth_texture_material), "shadow_map", depth_texture_handle);
                             
        glGenFramebuffers(1, &shadow_map->framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_map->framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadow_map->color_texture, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map->depth_texture, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr, ERROR_ALERT "Incomplete framebuffer when initializing shadow maps.");
            exit(EXIT_FAILURE);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
static void do_shadows(void)
{
    // GLint prev_viewport[4];
    // glGetIntegerv(GL_VIEWPORT, prev_viewport);
    // glViewport(0, 0, SHADOW_MAP_TEXTURE_WIDTH, SHADOW_MAP_TEXTURE_HEIGHT);
    int index;
    for_aspect(DirectionalLight, light)
        ShadowMap *shadow_map = &g_directional_light_shadow_maps[index];
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_map->framebuffer);
        glClearDepth(1.0);
        glClear(GL_DEPTH_BUFFER_BIT);

        mat4x4 shadow_matrix;
        //---rendering the shadow map from the camera
        Camera *camera;
        for_aspect(Camera, getting_camera)
            camera = getting_camera;
            break;
        end_for_aspect()
        Transform *camera_transform = get_sibling_aspect(camera, Transform);
        mat4x4 view_matrix = invert_rigid_mat4x4(Transform_matrix(camera_transform));
        mat4x4 vp_matrix = camera->projection_matrix;
        right_multiply_matrix4x4f(&vp_matrix, &view_matrix);
        shadow_matrix = vp_matrix;

        for_aspect(Body, body)
            Transform *transform = get_sibling_aspect(body, Transform);
            mat4x4 model_matrix = Transform_matrix(transform);
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    model_matrix.vals[4*i + j] *= body->scale;
                }
            }
            mat4x4 mvp_matrix = vp_matrix;
            right_multiply_matrix4x4f(&mvp_matrix, &model_matrix);
            set_uniform_mat4x4(Standard3D, mvp_matrix.vals, mvp_matrix.vals);

            Geometry *geometry = resource_data(Geometry, body->geometry);
            gm_draw(*geometry, g_shadow_map_material);
        end_for_aspect()

        paint2d_sprite_m(index*0.15,0,  0.15,0.15,  shadow_map->depth_texture_material);
        index ++;
    end_for_aspect()
    // glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void __old_do_shadows(void)
{
#if 0
    int index = 0;
    for_aspect(DirectionalLight, light)
        Transform *t = get_sibling_aspect(light, Transform);
        ShadowMap __old_*shadow_map = &g_directional_light_shadow_maps[index];
        mat4x4 shadow_matrix;
#if 0
        //mat4x4 shadow_view_matrix = invert_rigid_mat4x4(Transform_matrix(t));
        Camera *camera;
        for_aspect(Camera, getting_camera)
            camera = getting_camera;
            break;
        end_for_aspect()
        Transform *camera_transform = get_sibling_aspect(camera, Transform);
        Matrix4x4f view_matrix = invert_rigid_mat4x4(Transform_matrix(camera_transform));
        //Matrix4x4f vp_matrix = camera->projection_matrix;
        //right_multiply_matrix4x4f(&vp_matrix, &view_matrix);
        //shadow_matrix = vp_matrix;
        //shadow_matrix = view_matrix;
        shadow_matrix = identity_mat4x4();

        //set_uniform_mat4x4(Lights, active_shadow_matrix.vals, shadow_matrix.vals);

        glBindFramebuffer(GL_FRAMEBUFFER, shadow_map->framebuffer);
        // Save the viewport, switch it for rendering to the shadow map, then later switch it back.
        //:: glViewport specifies the affine transformation of x and y from normalized device coordinates to window coordinates. 
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glViewport(0, 0, SHADOW_MAP_TEXTURE_WIDTH, SHADOW_MAP_TEXTURE_HEIGHT);
        // Clearing the depth buffer while this framebuffer is bound clears the shadow map (sets all values to 1.0).
        // glClearDepth is the analogue to glClearColor.
        glClearDepth(1.0); //---maybe should save this and restore it after.
        glClear(GL_DEPTH_BUFFER_BIT);
{
        ResourceHandle gres = new_resource_handle(Geometry, "Models/quad");
        Geometry *geom = resource_data(Geometry, gres);
        ResourceHandle mres = Material_create("Materials/red");
        Material *mat = resource_data(Material, mres);
        mat4x4 matrix = {{
            0.25,  0,    0,    0.25,
            0,    0.25,  0,    0.25,
            0,    0,    0.25,  0,
            0,    0,    0,    1,
        }};
        set_uniform_mat4x4(Standard3D, mvp_matrix.vals, matrix.vals);
        gm_draw(*geom, mat);
        destroy_resource_handle(&gres);
        destroy_resource_handle(&mres);
}
#if 0
        for_aspect(Body, body)
            Transform *t = get_sibling_aspect(body, Transform);
            Matrix4x4f model_matrix = Transform_matrix(t);
            //---This body rescaling is a hack.
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    model_matrix.vals[4*i + j] *= body->scale;
                }
            }
            Geometry *geometry = resource_data(Geometry, body->geometry);
            mat4x4 mvp_matrix = shadow_matrix;
            right_multiply_matrix4x4f(&mvp_matrix, &model_matrix);
            print_matrix4x4f(&mvp_matrix);
            set_uniform_mat4x4(Standard3D, mvp_matrix.vals, mvp_matrix.vals);
            gm_draw(*geometry, g_shadow_map_material);
        end_for_aspect()
#endif
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

        static float depth_data[SHADOW_MAP_TEXTURE_WIDTH * SHADOW_MAP_TEXTURE_HEIGHT];
#define select 0
#if select == 0
        glBindTexture(GL_TEXTURE_2D, shadow_map->texture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depth_data);
        glBindTexture(GL_TEXTURE_2D, 0);
#elif select == 1
        glReadPixels(0, 0, SHADOW_MAP_TEXTURE_WIDTH, SHADOW_MAP_TEXTURE_HEIGHT,
                     GL_DEPTH_COMPONENT,
                     GL_FLOAT,
                     depth_data);
#elif select == 2
        for (int i = 0; i < SHADOW_MAP_TEXTURE_WIDTH; i++) {
            for (int j = 0; j < SHADOW_MAP_TEXTURE_WIDTH; j++) {
                depth_data[SHADOW_MAP_TEXTURE_WIDTH*j + i] = j % 2;
            }
        }
#endif
#if 1
        // Uncomment for debugging the shadow map framebuffer without viewing a texture.
        // This counts the number of non-zero entries, which if the light is pointing at things, should hint at whether it is working.
        int count = 0;
        for (int i = 0; i < SHADOW_MAP_TEXTURE_WIDTH; i++) {
            for (int j = 0; j < SHADOW_MAP_TEXTURE_HEIGHT; j++) {
                if (depth_data[SHADOW_MAP_TEXTURE_HEIGHT*i + j] != 0) {
                    count ++;
                    // printf("%.2f\n", depth_data[SHADOW_MAP_TEXTURE_WIDTH*i + j]);
                    // getchar();
                }
            }
        }
        printf("%d / %d\n", count, SHADOW_MAP_TEXTURE_WIDTH*SHADOW_MAP_TEXTURE_HEIGHT);
#endif
        // Create a new texture to show the shadow map.
        GLuint map_tex;
        glGenTextures(1, &map_tex);
        glBindTexture(GL_TEXTURE_2D, map_tex);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT, SHADOW_MAP_TEXTURE_WIDTH, SHADOW_MAP_TEXTURE_HEIGHT);
#if 1
        glTexSubImage2D(GL_TEXTURE_2D,
                        0, // first mipmap level
                        0, 0, // x and y offset
                        SHADOW_MAP_TEXTURE_WIDTH, SHADOW_MAP_TEXTURE_HEIGHT,
                        GL_DEPTH_COMPONENT, GL_FLOAT,
                        depth_data);
#endif

        ResourceHandle tres;
        // Test view the depth buffer as a rendered texture.
        ResourceHandle gres = new_resource_handle(Geometry, "Models/quad");
        Geometry *geom = resource_data(Geometry, gres);
        ResourceHandle mres = Material_create("Materials/render_shadow_map");
        Material *mat = resource_data(Material, mres);
#if 0
        material_set_texture_path(mat, "shadow_map", "Textures/minecraft/dirt");
#else
        Texture *tex = oneoff_resource(Texture, tres);
        tex->texture_id = map_tex;
        material_set_texture(mat, "shadow_map", tres);
#endif

        mat4x4 mvp_matrix = {{
            0.25,  0,    0,    0.25,
            0,    0.25,  0,    0.25,
            0,    0,    0.25,  0,
#endif
            0,    0,    0,    1,
        }};
        set_uniform_mat4x4(Standard3D, mvp_matrix.vals, mvp_matrix.vals);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        gm_draw(*geom, mat);
        float loop[2*4] = {
            0.5,0.5,  0.75,0.5,  0.75,0.75,  0.5,0.75
        };
        paint2d_loop_c(loop, 4, "r");

        destroy_resource_handle(&gres);
        destroy_resource_handle(&mres);
        destroy_resource_handle(&tres);
        glDeleteTextures(1, &map_tex);

        index ++;
    end_for_aspect()

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}



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

    // Meta-keys (debug, input, etc.)
    if (g_raw_mouse) { // if the raw mouse option is enabled, it is toggleable with a meta-key.
        if (action == GLFW_PRESS && key == g_glfw_raw_mouse_key) {
            toggle_raw_mouse();
        }
    }
    if (action == GLFW_PRESS && key == g_glfw_sma_debug_overlay_key) {
        g_sma_debug_overlay = !g_sma_debug_overlay;
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
    // Application-specific source/asset directories (these directories need not exist.)
    resource_path_add("Meshes", "resources/meshes");
    resource_path_add("Images", "resources/images");
    resource_path_add("Shaders", "resources/shaders");

    painting_init();
    init_shadows();
}

static void render(void)
{
    set_uniform_float(StandardLoopWindow, time, time);

    do_shadows();
    for_aspect(Camera, camera)
        //------
        // ---Allow cameras to have rectangles, and render to these.
        // ---Could do resize rectangles (so the camera aspect ratio is infered and this is used to compute the projection matrix)
        // ---as well as masking rectangles.
        //------

        // Form the view-projection matrix.
        Transform *camera_transform = get_sibling_aspect(camera, Transform);
        Matrix4x4f view_matrix = invert_rigid_mat4x4(Transform_matrix(camera_transform));
        Matrix4x4f vp_matrix = camera->projection_matrix;
        right_multiply_matrix4x4f(&vp_matrix, &view_matrix);
        
        // Upload the camera position and direction, and other information.
        set_uniform_vec3(Standard3D, camera_position, new_vec3(camera_transform->x, camera_transform->y, camera_transform->z));
        vec4 camera_forward_vector = matrix_vec4(&view_matrix, new_vec4(0,0,1,1));
        set_uniform_vec3(Standard3D, camera_direction, *((vec3 *) &camera_forward_vector)); //... get better matrix/vector routines.
        // Upload camera parameters.
        set_uniform_float(Standard3D, near_plane, camera->plane_n);
        set_uniform_float(Standard3D, far_plane, camera->plane_f);

        // Upload the uniform half-vectors for directional lights. This depends on the camera, and saves recomputation of the half-vector per-pixel,
        // since in the case of directional lights this vector is constant.
        int directional_light_index = 0; //---may be artifacts due to the desynchronization of the index of the directional light through each frame when a light changes.
        for_aspect(DirectionalLight, directional_light)
            vec3 direction = DirectionalLight_direction(directional_light);
            // Both the directional light direction and the camera forward vector are unit length, so their sum gives a half vector, then this is normalized.
            float hx = camera_forward_vector.vals[0] + direction.vals[0];
            float hy = camera_forward_vector.vals[1] + direction.vals[1];
            float hz = camera_forward_vector.vals[2] + direction.vals[2];
            float inv_length = 1/sqrt(hx*hx + hy*hy + hz*hz);
            hx *= inv_length;
            hy *= inv_length;
            hz *= inv_length;
            set_uniform_vec3(Lights, directional_lights[directional_light_index].half_vector, new_vec3(hx, hy, hz));
            directional_light_index ++;
        end_for_aspect()

        // Render each body.
        for_aspect(Body, body)
            Transform *transform = get_sibling_aspect(body, Transform);
            Geometry *mesh = resource_data(Geometry, body->geometry);
            Material *material = resource_data(Material, body->material);
            // Form the mvp matrix and upload it, along with other Standard3D information.
            Matrix4x4f model_matrix = Transform_matrix(transform);
            //---This body rescaling is a hack.
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    model_matrix.vals[4*i + j] *= body->scale;
                }
            }
            Matrix4x4f mvp_matrix = vp_matrix;
            right_multiply_matrix4x4f(&mvp_matrix, &model_matrix);
            set_uniform_vec3(Standard3D, model_position, new_vec3(transform->x, transform->y, transform->z));
            set_uniform_mat4x4(Standard3D, model_matrix.vals, model_matrix.vals);
            Matrix4x4f normal_matrix;
            euler_rotation_matrix4x4f(&normal_matrix, transform->theta_x, transform->theta_y, transform->theta_z);
            set_uniform_mat4x4(Standard3D, normal_matrix.vals, normal_matrix.vals); // assuming only rigid transformations.
            set_uniform_mat4x4(Standard3D, mvp_matrix.vals, mvp_matrix.vals);
            gm_draw(*mesh, material);
        end_for_aspect()
        // Draw the buffered paint (in global coordinates).
        set_uniform_mat4x4(Standard3D, mvp_matrix.vals, vp_matrix.vals);
        painting_draw(Canvas3D);
    end_for_aspect()
    // Flush the standard 3D paint canvas.

    // Draw the 2D overlay paint. ----make this an actual overlay.
    bool culling;
    glGetIntegerv(GL_CULL_FACE, &culling);
    glDisable(GL_CULL_FACE);
    mat4x4 overlay_matrix = matrix_paint2d();
    set_uniform_mat4x4(Standard3D, mvp_matrix.vals, overlay_matrix.vals);
    painting_draw(Canvas2D);
    if (culling) glEnable(GL_CULL_FACE);
}
// Applications are not supposed to use this, but it is exposed here for testing.
void ___render(void)
{
    render();
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
        set_uniform_vec3(Lights, directional_lights[index].direction, DirectionalLight_direction(directional_light));
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
    glfwSetKeyCallback(window, key_callback);

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
    glfwSetCursorPosCallback(window, cursor_position_callback);

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
        glClearColor(fg_color[0], fg_color[1], fg_color[2], fg_color[3]);
        glClear(clear_mask);

        loop_base();

        glFlush();
        glfwSwapBuffers(window);
    }
    // Cleanup
    close_program();
    glfwDestroyWindow(window);
    glfwTerminate();
}
