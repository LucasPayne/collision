/*--------------------------------------------------------------------------------
    Game-renderer module. This is the rendering stuff that depends on the gameobject
    system. Any general utilities for working with OpenGL and glsl, material, texture, and geometry resources,
    etc., are part of the rendering module. The game-renderer is a client of these rendering facilities.
--------------------------------------------------------------------------------*/
#include "Engine.h"

// This is probably smaller than both the horizontal and vertical resolution of the screen.
// Apparently the back buffer is the screen size, and rendering to a texture larger than this just does not fill the texture.
// Larger shadow maps could be done with multiple render passes on tiles, for example 512x512 may be trusted to be a working size,
// then the light frustum split into tiles, each one rendering into a tile of the texture.
#define SHADOW_MAP_TEXTURE_WIDTH 750
#define SHADOW_MAP_TEXTURE_HEIGHT 750
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        //---Setting a border color does not seem to work.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
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
            fprintf(stderr, ERROR_ALERT "Incomplete framebuffer when initializing shadow maps.\n");
            exit(EXIT_FAILURE);
        }
        // Bind the depth texture to its reserved texture unit. --------------------------------
        set_uniform_texture(Lights, directional_light_shadow_maps[i], shadow_map->depth_texture);
        // --------------------------------------------------------------------------------------
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
static void do_shadows(void)
{
    GLint prev_viewport[4];
    glGetIntegerv(GL_VIEWPORT, prev_viewport);
    glViewport(0, 0, SHADOW_MAP_TEXTURE_WIDTH, SHADOW_MAP_TEXTURE_HEIGHT);
    int index = 0;
    for_aspect(DirectionalLight, light)
        ShadowMap *shadow_map = &g_directional_light_shadow_maps[index];
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_map->framebuffer);
        glClearDepth(1.0);
        glClear(GL_DEPTH_BUFFER_BIT);

        mat4x4 inverse_light_model_matrix = invert_rigid_mat4x4(Transform_matrix(get_sibling_aspect(light, Transform)));
        mat4x4 model_to_ndc = {{
            2/light->shadow_width,  0,                       0,                      0,
            0,                      2/light->shadow_height,  0,                      0,
            0,                      0,                       2/light->shadow_depth,  0,
            0,                      0,                       -1,                      1,
        }};
        mat4x4 shadow_matrix = model_to_ndc;
        right_multiply_matrix4x4f(&shadow_matrix, &inverse_light_model_matrix);
        set_uniform_mat4x4(Lights, directional_lights[index].shadow_matrix.vals, shadow_matrix.vals);

        for_aspect(Body, body)
            Transform *transform = get_sibling_aspect(body, Transform);
            mat4x4 model_matrix = Transform_matrix(transform);
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    model_matrix.vals[4*i + j] *= body->scale;
                }
            }
            mat4x4 mvp_matrix = shadow_matrix;
            right_multiply_matrix4x4f(&mvp_matrix, &model_matrix);
            set_uniform_mat4x4(Standard3D, mvp_matrix.vals, mvp_matrix.vals);

            Geometry *geometry = resource_data(Geometry, body->geometry);
            gm_draw(*geometry, g_shadow_map_material);
        end_for_aspect()
        index ++;
    end_for_aspect()
    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void render_body_with_material(mat4x4 vp_matrix, Body *body, Material *material)
{
    Transform *transform = get_sibling_aspect(body, Transform);
    Geometry *mesh = resource_data(Geometry, body->geometry);
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
}
void render_body(mat4x4 vp_matrix, Body *body)
{
    Material *material = resource_data(Material, body->material);
    render_body_with_material(vp_matrix, body, material);
}

mat4x4 Camera_prepare(Camera *camera)
{
    // Prepare the rendering context for rendering with this camera.

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
    return vp_matrix;
}

void init_game_renderer(void)
{
    painting_init();
    init_shadows();
}

void render(void)
{
    set_uniform_float(StandardLoopWindow, time, time);

    do_shadows();
    for_aspect(Camera, camera)
        mat4x4 vp_matrix = Camera_prepare(camera);
        // Render each body.
        for_aspect(Body, body)
            render_body(vp_matrix, body);
        end_for_aspect()
        // Draw the buffered paint (in global coordinates).
        set_uniform_mat4x4(Standard3D, mvp_matrix.vals, vp_matrix.vals);
        painting_draw(Canvas3D);
    end_for_aspect()
    // Flush the standard 3D paint canvas.
    painting_flush(Canvas3D);

    // Draw the 2D overlay paint. ----make this an actual overlay.
    bool culling;
    glGetIntegerv(GL_CULL_FACE, &culling);
    glDisable(GL_CULL_FACE);
    mat4x4 overlay_matrix = matrix_paint2d();
    set_uniform_mat4x4(Standard3D, mvp_matrix.vals, overlay_matrix.vals);
    painting_draw(Canvas2D);
    if (culling) glEnable(GL_CULL_FACE);
}

