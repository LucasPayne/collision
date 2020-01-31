/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

static Camera *g_camera = NULL; // Intend only to have one camera.

typedef struct Plaster_s {
    bool active;
    GLuint plaster_texture;
    GLuint plaster_framebuffer;
    GLuint depth_texture;
    GLuint depth_color_texture; // not sure if need this, is it required to make the framebuffer complete?
    GLuint depth_framebuffer;
    vec3 camera_position;
    vec3 camera_angles;
    mat4x4 matrix; // vp matrix of the camera when the plaster was made.
} Plaster;
#define NUM_PLASTERS 8
Plaster g_plasters[NUM_PLASTERS] = { 0 };

static void camera_update(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    float speed = 100;
    float move_x = 0, move_z = 0;
    if (alt_arrow_key_down(Right)) move_x += speed * dt;
    if (alt_arrow_key_down(Left)) move_x -= speed * dt;
    if (alt_arrow_key_down(Up)) move_z -= speed * dt;
    if (alt_arrow_key_down(Down)) move_z += speed * dt;
    Transform_move_relative(t, new_vec3(move_x, 0, move_z));

    float look_speed = 4;
    if (arrow_key_down(Left)) t->theta_y -= look_speed * dt;
    if (arrow_key_down(Right)) t->theta_y += look_speed * dt;
}

void init_plasters(void)
{
    int width, height;
    float viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    width = viewport[2];
    height = viewport[3];

    for (int i = 0; i < MAX_NUM_PLASTERS; i++) {
        Plaster *plaster = &g_plasters[i];
        plaster->active = false;

        // Create the plastering framebuffer. The only plastered object is rendered to this, with RGBA color.
        glGenTextures(1, &plaster->plaster_texture);
        glBindTexture(GL_TEXTURE_2D, plaster->plaster_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
        // Framebuffer
        glGenFramebuffers(1, &plaster->plaster_framebuffer);    
        glBindFramebuffer(GL_FRAMEBUFFER, plaster->plaster_framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, plaster->plaster_texture, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        // Create the depth framebuffer. Everything except the plastered object is rendered to this,
        // and the only thing wanted is the depth component rendered to the depth texture.
        glGenTextures(1, &plaster->depth_color_texture);
        glBindTexture(GL_TEXTURE_2D, plaster->depth_color_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
        glGenTextures(1, &plaster->depth_texture);
        glBindTexture(GL_TEXTURE_2D, plaster->depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
        // Framebuffer
        glGenFramebuffers(1, &plaster->depth_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, plaster->depth_framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, plaster->depth_color_texture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, plaster->depth_texture, 0);
        
    }
}

void create_plaster(Camera *camera)
{
    Plaster *plaster = NULL;
    for (int i = 0; i < MAX_NUM_PLASTERS; i++) {
        if (!g_plasters[i].active) plaster = &g_plasters[i];
    }
    if (plaster == NULL) {
        fprintf(stderr, "made too many plasters\n");
        exit(EXIT_FAILURE);
    }
    Transform *t = get_sibling_aspect(camera, Transform);
    plaster->camera_position = Transform_position(t);
    plaster->camera_angles = Transform_angles(t);
    plaster->matrix = invert_rigid_mat4x4(Transform_matrix(t));
    right_multiply_matrix4x4f(&plaster->matrix, &camera->projection_matrix);
}

static void camera_key_input(Input *input, int key, int action, int mods)
{
    Transform *t = get_sibling_aspect(input, Transform);
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE) t->y += 5;
        if (key == GLFW_KEY_LEFT_SHIFT) t->y -= 5;
        if (key == GLFW_KEY_P) create_plaster(g_camera);
    }
}

extern void input_event(int key, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    open_scene(g_scenes, "block_on_floor");

    EntityID camera_man = new_entity(4);
    Transform_set(entity_add_aspect(camera_man, Transform), -70,50,70,  0,M_PI/4,0);
    Camera *camera = entity_add_aspect(camera_man, Camera);
    Camera_init(camera, ASPECT_RATIO, 1, 0.9, 10);
    Logic *logic = entity_add_aspect(camera_man, Logic);
    logic->update = camera_update;
    Input_init(entity_add_aspect(camera_man, Input), INPUT_KEY, camera_key_input, true);
    g_camera = camera; // Make the camera globally accessible.

    EntityID light = new_entity(4);
    Transform_set(entity_add_aspect(light, Transform), 0,100,20,  M_PI/2+0.4,0,0);
    DirectionalLight_init(entity_add_aspect(light, DirectionalLight),  1,1,1,1,  200,200,400);

    init_plasters();
}
extern void loop_program(void)
{
    
}
extern void close_program(void)
{
}
