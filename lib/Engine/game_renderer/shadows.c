/*--------------------------------------------------------------------------------
    Everything to do with the initialization and maintenance of shadowing information.
--------------------------------------------------------------------------------*/
#include "Engine.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) < (Y) ? (Y) : (X))

static Material *g_shadow_map_material = NULL;
ShadowMap g_directional_light_shadow_maps[MAX_NUM_DIRECTIONAL_LIGHTS];

void init_shadows(void)
{
    // Load the shadow depth-pass shaders into a material.
    ResourceHandle shadow_map_material_handle = Material_create("Materials/shadows");
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
    // Seems easier to hardcode distances rather than calculate them due to some mathematical formula.
    // As general guidelines, the earliest segment should be large enough to encompass what is generally in the foreground, while
    // small enough to give high resolution and room for the other segments to also contribute when the camera is on ground level.
    // Then, further segments should get longer, so they occupy roughly the same amount of screen-space.
    vec4 segment_depths = new_vec4(
        n,
        n + 0.1  * (f - n),
        n + 0.3  * (f - n),
        n + 0.6  * (f - n)
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

        // Colors for frustum-segment / cascade visualizations.
        vec4 colors[] = {   
            new_vec4(1,0,0,1),
            new_vec4(0,1,0,1),
            new_vec4(0,0,1,1),
            new_vec4(0,1,1,1),
        };
        for (int segment = 0; segment < 4; segment++) {
        //for (int segment = 0; segment < 4; segment++) {
            // along:    near plane z offset from this frustum segment.
            // along_to: far plane z offset from this frustum segment.
            float along = -segment_depths.vals[segment];
            float along_to = segment == 3 ? -f : -segment_depths.vals[segment + 1];
            vec3 near_p = vec3_add(pos, vec3_mul(Transform_forward(transform), along));
            vec3 far_p =  vec3_add(pos, vec3_mul(Transform_forward(transform), along_to));
            // Calculate the points of the frustum segment, on the near plane and far plane.
            //////////////////////////////////////////////////////////////////////////////////
            vec3 frustum_points[] = {
                vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, t, 0), 2.2 * along / n))),
                vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, b, 0), 2.2 * along / n))),
                vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, b, 0), 2.2 * along / n))),
                vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, t, 0), 2.2 * along / n))),
                vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, t, 0),  2.2 * along_to / n ))),
                vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, b, 0),  2.2 * along_to / n ))),
                vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, b, 0),  2.2 * along_to / n ))),
                vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, t, 0),  2.2 * along_to / n ))),
            };
            vec3 *near_quad = frustum_points;
            vec3 *far_quad = frustum_points + 4;
#if 0
            // paint_box_v(Canvas3D, frustum_points, color_fade(colors[segment], 0.5));
            for (int i = 0; i < 4; i++) {
                paint_line_v(Canvas3D, near_quad[i], near_quad[(i+1)%4], colors[segment], 2);
                paint_line_v(Canvas3D, far_quad[i], far_quad[(i+1)%4], colors[segment], 2);
                paint_line_v(Canvas3D, near_quad[i], far_quad[i], colors[segment], 2);
            }
#endif
            mat4x4 light_matrix = invert_rigid_mat4x4(Transform_matrix(get_sibling_aspect(light, Transform)));
            // Transform frustum segment to light space.
            vec3 light_frustum[8];
            for (int i = 0; i < 4; i++) {
                light_frustum[i] = mat4x4_vec3(&light_matrix, near_quad[i]);
                light_frustum[i + 4] = mat4x4_vec3(&light_matrix, far_quad[i]);
            }
            //--------------------------------------------------------------------------------
            // Find the axis-aligned bounding box of the frustum segment in light coordinates.
            // Find the minimum and maximum corners.
            //--------------------------------------------------------------------------------
            // Scene awareness: winnow the box down so that it more tightly (but not perfectly) encloses the shadow-casting models in the scene.
            // This uses the radius of each body, being the maximal distance from the model-origin of a vertex, to create a bounding box aligned to light space.
            //--------------------------------------------------------------------------------
            vec3 scene_corners[2] = { light_frustum[0], light_frustum[0] };
            for_aspect(Body, body)
                if (body->is_ground) continue; // The is_ground flag can be set on a body so that shadow maps can be made higher resolution,
                                               // since the ground is large but probably won't cast shadows.
                float radius = Body_radius(body);
                vec3 position = mat4x4_vec3(&light_matrix, Transform_position(get_sibling_aspect(body, Transform)));
                for (int i = 0; i < 3; i++) {
		    float min_val = position.vals[i] - radius;
		    float max_val = position.vals[i] + radius;
                    if (min_val < scene_corners[0].vals[i]) scene_corners[0].vals[i] = min_val;
                    if (max_val > scene_corners[1].vals[i]) scene_corners[1].vals[i] = max_val;
                }
            end_for_aspect()

            // Now if this box is larger than the frustum-segment, winnow it down to the frustum-segment, since shadow-casters outside of it do not matter.
            vec3 box_corners[2] = { light_frustum[0], light_frustum[0] };
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 3; j++) {
                    if (light_frustum[i].vals[j] < box_corners[0].vals[j]) box_corners[0].vals[j] = light_frustum[i].vals[j];
                    if (light_frustum[i].vals[j] > box_corners[1].vals[j]) box_corners[1].vals[j] = light_frustum[i].vals[j];
                }
            }
            // Now take the minimum-extent (so, minimum maximums) of each of these pairs of corners.
            for (int i = 0; i < 3; i++) {
                box_corners[0].vals[i] = MAX(box_corners[0].vals[i], scene_corners[0].vals[i]);
                box_corners[1].vals[i] = MIN(box_corners[1].vals[i], scene_corners[1].vals[i]);
            }
            //--------------------------------------------------------------------------------

            // light_to_box:
            // This matrix is a scale and translation matrix which transforms the frustum-segment bounding box to the canonical view volume.
            float w = box_corners[1].vals[0] - box_corners[0].vals[0];
            float h = box_corners[1].vals[1] - box_corners[0].vals[1];
            float d = box_corners[1].vals[2] - box_corners[0].vals[2];
            float x = box_corners[0].vals[0];
            float y = box_corners[0].vals[1];
            float z = box_corners[0].vals[2];
            mat4x4 light_to_box = {{
                1,     0,    0,     0,
                0,     1,    0,     0,
                0,     0,    1,     0,
                1,   1,      0,     1, ///////depth
            }};
            mat4x4 light_to_box_2 = {{
                -2/w,  0,    0,     0,
                0,    -2/h,  0,     0,
                0,    0,    1/d,   0,
                0,    0,    0,     1,
            }};
            mat4x4 light_to_box_3 = {{
                1,    0,    0,     0,
                0,    1,    0,     0,
                0,    0,    1,     0,
                -x,   -y,   -z,    1,
            }};
            right_multiply_matrix4x4f(&light_to_box, &light_to_box_2);
            right_multiply_matrix4x4f(&light_to_box, &light_to_box_3);

            // box_to_quadrant:
            // This matrix transforms the box to the relevant quadrant.
            mat4x4 box_to_quadrant = {{
                0.5,              0,                0, 0,
                0,                0.5,              0, 0,
                0,                0,                1, 0,
                0,                0,                0, 1,
            }};
            mat4x4 box_to_quadrant_2 = {{
                1,                0,                0, 0,
                0,                1,                0, 0,
                0,                0,                1, 0,
                -1 + 2 * (segment % 2),                -1 + 2 * (segment / 2), 0, 1,
            }};
            right_multiply_matrix4x4f(&box_to_quadrant, &box_to_quadrant_2);

            mat4x4 shadow_matrix = box_to_quadrant;
            right_multiply_matrix4x4f(&shadow_matrix, &light_to_box);
/*--------------------------------------------------------------------------------
light to quadrant 0
(0.000000, 0.000000, 0.000000)
(-1.000000, -1.000000, 1.000000)
light to uvd 0
(0.500000, 0.500000, -0.000000)
(0.000000, 0.000000, 1.000000)
--------------------------------------------------------------------------------
light to quadrant 1
(1.000000, 0.000000, 0.000000)
(0.000000, -1.000000, 1.000000)
light to uvd 1
(1.000000, 0.500000, -0.000000)
(0.500000, -0.000000, 1.000000)
--------------------------------------------------------------------------------
light to quadrant 2
(0.000000, 1.000000, 0.000000)
(-1.000000, -0.000000, 1.000000)
light to uvd 2
(0.500000, 1.000000, 0.000000)
(0.000000, 0.500000, 1.000000)
--------------------------------------------------------------------------------
light to quadrant 3
(1.000000, 1.000000, 0.000000)
(-0.000000, 0.000000, 1.000000)
light to uvd 3
(1.000000, 1.000000, -0.000000)
(0.500000, 0.500000, 1.000000)

--------------------------------------------------------------------------------*/


#if 0
{
            vec3 c1 = mat4x4_vec3(&shadow_matrix, box_corners[0]);
            vec3 c2 = mat4x4_vec3(&shadow_matrix, box_corners[1]);
            printf("--------------------------------------------------------------------------------\n");
            printf("light to quadrant %d\n", segment);
            mat4x4 light_model_matrix = Transform_matrix(get_sibling_aspect(light, Transform));
            vec3 box_corner1_worldspace = mat4x4_vec3(&light_model_matrix, box_corners[0]);
            vec3 box_corner2_worldspace = mat4x4_vec3(&light_model_matrix, box_corners[1]);
            print_vec3(c1);
            print_vec3(c2);
}
#endif
            right_multiply_matrix4x4f(&shadow_matrix, &light_matrix);
            //--------------------------------------------------------------------------------
            // The shadow matrices uploaded transform to uvd (UV + depth coordinates) for the quadrant.


            mat4x4 light_to_uvd_quadrant = {{
                1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                0.5,0.5,0.5,1
            }};
            mat4x4 light_to_uvd_quadrant_2 = {{
                0.5,0,0,0,
                0,0.5,0,0,
                0,0,0.5,0,
                0,0,0,1,
            }};
            right_multiply_matrix4x4f(&light_to_uvd_quadrant, &light_to_uvd_quadrant_2);
            mat4x4 uvd_shadow_matrix = light_to_uvd_quadrant;
            // these extra operations prepend the operations the shadow matrix is composed of
            // (they happen afterward, mapping the view-volume quadrant to its UV quadrant).
            right_multiply_matrix4x4f(&uvd_shadow_matrix, &shadow_matrix);
            set_uniform_mat4x4(Lights, directional_lights[index].shadow_matrices[segment].vals, uvd_shadow_matrix.vals);
#if 0
{
            printf("light to uvd %d\n", segment);
            mat4x4 light_model_matrix = Transform_matrix(get_sibling_aspect(light, Transform));
            vec3 box_corner1_worldspace = mat4x4_vec3(&light_model_matrix, box_corners[0]);
            vec3 box_corner2_worldspace = mat4x4_vec3(&light_model_matrix, box_corners[1]);
            vec3 c1 = mat4x4_vec3(&uvd_shadow_matrix, box_corner1_worldspace);
            vec3 c2 = mat4x4_vec3(&uvd_shadow_matrix, box_corner2_worldspace);
            print_vec3(c1);
            print_vec3(c2);
}
#endif
            // Render to this frustum-segment's quadrant of the shadow map.
            glBindFramebuffer(GL_FRAMEBUFFER, shadow_map->framebuffer);
            glClearDepth(1.0);
            glEnable(GL_SCISSOR_TEST);
            float quadrant[4] = {
                SHADOW_MAP_TEXTURE_WIDTH  * 0.5 * (segment % 2),
                SHADOW_MAP_TEXTURE_HEIGHT * 0.5 * (segment / 2),
                SHADOW_MAP_TEXTURE_WIDTH  * 0.5,
                SHADOW_MAP_TEXTURE_HEIGHT * 0.5,
            };
            glScissor(quadrant[0], quadrant[1], quadrant[2], quadrant[3]);
            // Clear the shadow texture.
            glClear(GL_DEPTH_BUFFER_BIT);

            // static int frame_number = 0; //visualize order
            // if (((frame_number ++ / 20) % 4) != segment) continue;
            for_aspect(Body, body)
                render_body_with_material(shadow_matrix, body, g_shadow_map_material);
                // render_body(shadow_matrix, body);
            end_for_aspect()

#if 0
            // Draw the frustum-segment bounding box.
            // Use these to form all points of the box.
            vec3 box_points[8];
            int p = 0;
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        box_points[p] = new_vec3(box_corners[i].vals[0], box_corners[j].vals[1], box_corners[k].vals[2]);
                        p++;
                    }
                }
            }
            // Transform this box to world space.
	    mat4x4 light_to_world = Transform_matrix(get_sibling_aspect(light, Transform));
            for (int i = 0; i < 8; i++) {
                box_points[i] = mat4x4_vec3(&light_to_world, box_points[i]);
            }
            // Draw the box.
            for (int i = 0; i < 4; i++) {
                paint_line_v(Canvas3D, box_points[2*i], box_points[2*i+1], color_mul(colors[segment], 0.5), 1);
                paint_line_v(Canvas3D, box_points[i], box_points[i+4], color_mul(colors[segment], 0.5), 1);
            }
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    paint_line_v(Canvas3D, box_points[i + 4*j], box_points[i + 4*j + 2], color_mul(colors[segment], 0.5), 1);
                }
            }
#endif
        }

        index ++;
        if (index == MAX_NUM_DIRECTIONAL_LIGHTS) break;
    end_for_aspect()
    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#if 0
    index = 0;
    for_aspect(DirectionalLight, light)
        float size = 0.23;
        // View the depth maps
        // ResourceHandle material = Material_create("Materials/render_shadow_map");
        // ResourceHandle texture;
        // Texture *tex = oneoff_resource(Texture, texture);
        // tex->texture_id = g_directional_light_shadow_maps[index].depth_texture;
        // material_set_texture(resource_data(Material, material), "shadow_map", texture);
        // paint2d_sprite_m(size * index,0,size,size, material);
        // destroy_resource_handle(&texture);

        GLuint color_texture = g_directional_light_shadow_maps[index].color_texture;
        GLuint depth_texture = g_directional_light_shadow_maps[index].depth_texture;
        // printf("c:%u d:%u\n", color_texture, depth_texture);

        paint2d_sprite_m(size * index,0,size,size, g_directional_light_shadow_maps[index].depth_texture_material);

        // ResourceHandle texture;
        // Texture *tex = oneoff_resource(Texture, texture);
        // tex->texture_id = g_directional_light_shadow_maps[index].color_texture;
        // paint2d_sprite(size * index,0,size,size, texture);
        index ++;
    end_for_aspect();
#endif
}
