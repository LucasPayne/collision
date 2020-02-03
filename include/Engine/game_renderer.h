/*--------------------------------------------------------------------------------
    Rendering
--------------------------------------------------------------------------------*/
#ifndef HEADER_DEFINED_GAMEOBJECTS
#define HEADER_DEFINED_GAMEOBJECTS
void init_game_renderer(void);
void render_body_with_material(mat4x4 vp_matrix, Body *body, Material *material);
void render_body(mat4x4 vp_matrix, Body *body);
void render(void);

void init_shadows(void);
void do_shadows(void);

void render_paint2d();

#endif // HEADER_DEFINED_GAMEOBJECTS
