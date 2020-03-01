/*--------------------------------------------------------------------------------
    Rendering
--------------------------------------------------------------------------------*/
#ifndef HEADER_DEFINED_GAME_RENDERER
#define HEADER_DEFINED_GAME_RENDERER
void init_game_renderer(void);
void render_body_with_material(mat4x4 vp_matrix, Body *body, Material *material);
void render_body(mat4x4 vp_matrix, Body *body);
void render(void);


void render_paint2d();
/*--------------------------------------------------------------------------------
    Shadows
--------------------------------------------------------------------------------*/
#define SHADOW_MAP_TEXTURE_WIDTH 4096
#define SHADOW_MAP_TEXTURE_HEIGHT 4096
// Currently only doing directional light shadows.
typedef struct ShadowMap_s {
    GLuint framebuffer;
    GLuint depth_texture;
    GLuint color_texture;
    // For debugging purposes, the shadow map keeps a resource handle for a material which has the depth texture attached.
    // This won't be destroyed, and can be used to render the depth map to a quad.
    ResourceHandle depth_texture_material; // Resource: Material
} ShadowMap;
extern ShadowMap g_directional_light_shadow_maps[];
void init_shadows(void);
void do_shadows(Camera *camera);

#endif // HEADER_DEFINED_GAME_RENDERER
