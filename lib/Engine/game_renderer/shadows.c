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
void do_shadows(Camera *camera)
{
    // Render to the cascaded shadow maps.
    // -----------------------------------
    float n, f, b, t, l, r;
    n = camera->plane_n;
    f = camera->plane_f;
    b = camera->plane_b;
    t = camera->plane_t;
    l = camera->plane_l;
    r = camera->plane_r;
    Transform *transform = get_sibling_aspect(camera, Transform);
    vec3 pos = Transform_position(transform);
    // Upload the frustum-segment depths so fragment shaders can test which segment the fragment is in, for visualization.
    // float segment_depths[4] = {
    //     n,
    //     n + 0.25 * (f - n),
    //     n + 0.5  * (f - n),
    //     n + 0.75 * (f - n),
    // };
    vec4 segment_depths = new_vec4(
        n,
        n + 0.25 * (f - n),
        n + 0.5  * (f - n),
        n + 0.75 * (f - n)
    );
    set_uniform_vec4(Lights, shadow_map_segment_depths, segment_depths);

    // Set the viewport to align to the shadow map textures. 
    GLint prev_viewport[4];
    glGetIntegerv(GL_VIEWPORT, prev_viewport);
    glViewport(0, 0, SHADOW_MAP_TEXTURE_WIDTH, SHADOW_MAP_TEXTURE_HEIGHT);

    // For each directional light, render to each quadrant of the shadow texture, one for each frustum segment.
    int index = 0;
    for_aspect(DirectionalLight, light)
        ShadowMap *shadow_map = &g_directional_light_shadow_maps[index];
        // Clear the shadow texture.
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_map->framebuffer);
        glClearDepth(1.0);
        glClear(GL_DEPTH_BUFFER_BIT);

        for (int segment = 0; segment < 4; segment++) {
            // along:    near plane z offset from this frustum segment.
            // along_to: far plane z offset from this frustum segment.
            float along = -n - (f - n) * segment/4.0 * f/n;
            float along_to = -n - (f - n) * (segment + 1)/4.0 * f/n;
            vec3 near_p = vec3_add(pos, vec3_mul(Transform_forward(transform), along));
            vec3 far_p =  vec3_add(pos, vec3_mul(Transform_forward(transform), along_to));
            // Calculate the points of the frustum segment, on the near plane and far plane.
            vec3 near_quad[] = {
                vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, t, 0), along/n))),
                vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, b, 0), along/n))),
                vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, b, 0), along/n))),
                vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, t, 0), along/n))),
            };
            vec3 far_quad[] = {
                vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, t, 0),  along_to/n))),
                vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, b, 0),  along_to/n))),
                vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, b, 0),  along_to/n))),
                vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, t, 0),  along_to/n))),
            };
            mat4x4 light_matrix = invert_rigid_mat4x4(Transform_matrix(get_sibling_aspect(light, Transform)));
            // Transform frustum segment to light space.
            vec3 light_frustum[8];
            for (int i = 0; i < 4; i++) {
                light_frustum[i] = mat4x4_vec3(&light_matrix, near_quad[i]);
                light_frustum[i + 4] = mat4x4_vec3(&light_matrix, far_quad[i]);
            }
            // Find the axis-aligned bounding box of the frustum segment in light coordinates.
            // Find the minimum and maximum corners.
            vec3 box_corners[2] = { light_frustum[0], light_frustum[0] };
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 3; j++) {
                    if (light_frustum[i].vals[j] < box_corners[0].vals[j]) box_corners[0].vals[j] = light_frustum[i].vals[j];
                    if (light_frustum[i].vals[j] > box_corners[1].vals[j]) box_corners[1].vals[j] = light_frustum[i].vals[j];
                }
            }
            // light_to_box:
            // This matrix is a scale and translation matrix which transforms the frustum-segment bounding box to the canonical view volume.
            float w = box_corners[1].vals[0] - box_corners[0].vals[0];
            float h = box_corners[1].vals[1] - box_corners[0].vals[1];
            float d = box_corners[1].vals[2] - box_corners[0].vals[2];
            float x = box_corners[0].vals[0];
            float y = box_corners[0].vals[1];
            float z = box_corners[0].vals[2];
            mat4x4 light_to_box = {{
                2/w,  0,    0,   0,
                0,    2/h,  0,   0,
                0,    0,    1/d, 0,
                -x-1, -y-1, -z,  1,
            }};
            mat4x4 shadow_matrix = light_to_box;
            right_multiply_matrix4x4f(&shadow_matrix, &light_matrix);
            set_uniform_mat4x4(Lights, directional_lights[index].shadow_matrices[segment].vals, shadow_matrix.vals);

            for_aspect(Body, body)
                render_body_with_material(shadow_matrix, body, g_shadow_map_material);
            end_for_aspect()
        }
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
