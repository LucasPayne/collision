/*--------------------------------------------------------------------------------
    Everything to do with the initialization and maintenance of shadowing information.
--------------------------------------------------------------------------------*/
#include "Engine.h"

#define SHADOW_MAP_TEXTURE_WIDTH 2048
#define SHADOW_MAP_TEXTURE_HEIGHT 2048
// Currently only doing directional light shadows.
typedef struct ShadowMap_s {
    GLuint framebuffer;
    GLuint depth_texture;
    GLuint color_texture;
    // For debugging purposes, the shadow map keeps a resource handle for a material which has the depth texture attached.
    // This won't be destroyed, and can be used to render the depth map to a quad.
    ResourceHandle depth_texture_material; // Resource: Material
} ShadowMap;
ShadowMap g_directional_light_shadow_maps[MAX_NUM_DIRECTIONAL_LIGHTS];
static Material *g_shadow_map_material = NULL;

void init_shadows(void)
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
void do_shadows(void)
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
            render_body_with_material(shadow_matrix, body, g_shadow_map_material);
        end_for_aspect()
        index ++;
        if (index == MAX_NUM_DIRECTIONAL_LIGHTS) break;
    end_for_aspect()
    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#if 0
    index = 0;
    for_aspect(DirectionalLight, light)
        // View the depth maps
        ResourceHandle material = Material_create("Materials/render_shadow_map");
        ResourceHandle texture;
        Texture *tex = oneoff_resource(Texture, texture);
        tex->texture_id = g_directional_light_shadow_maps[index].depth_texture;
        material_set_texture(resource_data(Material, material), "shadow_map", texture);
        paint2d_sprite_m(0.1 * index,0,0.1,0.1, material);
        destroy_resource_handle(&texture);
        index ++;
    end_for_aspect();
#endif
}
