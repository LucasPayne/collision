/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

// Should have these match the resolution.
#define PLASTER_WIDTH 512
#define PLASTER_HEIGHT 512

static Camera *g_camera = NULL; // Intend only to have one camera.
static Body *g_plastering_body = NULL; // testing. you should be able to change the object being plastered.

typedef struct Plaster_s {
    bool active;
    GLuint plaster_texture;
    GLuint plaster_framebuffer;
    ResourceHandle plaster_texture_handle; // Resource: Texture
    GLuint depth_texture;
    GLuint depth_color_texture; // not sure if need this, is it required to make the framebuffer complete?
    GLuint depth_framebuffer;
    vec3 camera_position;
    vec3 camera_angles;
    mat4x4 matrix; // vp matrix of the camera when the plaster was made.
} Plaster;
#define MAX_NUM_PLASTERS 8
Plaster g_plasters[MAX_NUM_PLASTERS] = { 0 };

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

    if (g_plastering_body != NULL) {
        Transform *control_t = get_sibling_aspect(g_plastering_body, Transform);
        vec3 control_pos = Transform_relative_position(t, new_vec3(0,0,-20));
        control_t->x = control_pos.vals[0];
        control_t->y = control_pos.vals[1];
        control_t->z = control_pos.vals[2];
    }
}

void init_plasters(void)
{
    int width, height;
    width = PLASTER_WIDTH;
    height = PLASTER_HEIGHT;

    for (int i = 0; i < MAX_NUM_PLASTERS; i++) {
        Plaster *plaster = &g_plasters[i];
        plaster->active = false;

        // Create the plastering framebuffer. The only plastered object is rendered to this, with RGBA color.
        glGenTextures(1, &plaster->plaster_texture);
        glBindTexture(GL_TEXTURE_2D, plaster->plaster_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
        // Framebuffer
        glGenFramebuffers(1, &plaster->plaster_framebuffer);    
        glBindFramebuffer(GL_FRAMEBUFFER, plaster->plaster_framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, plaster->plaster_texture, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr, ERROR_ALERT "Incomplete framebuffer when initializing plasters.\n");
            exit(EXIT_FAILURE);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Create the depth framebuffer. Everything except the plastered object is rendered to this,
        // and the only thing wanted is the depth component rendered to the depth texture.
        glGenTextures(1, &plaster->depth_color_texture);
        glBindTexture(GL_TEXTURE_2D, plaster->depth_color_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
        glGenTextures(1, &plaster->depth_texture);
        glBindTexture(GL_TEXTURE_2D, plaster->depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
        // Framebuffer
        glGenFramebuffers(1, &plaster->depth_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, plaster->depth_framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, plaster->depth_color_texture, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, plaster->depth_texture, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr, ERROR_ALERT "Incomplete framebuffer when initializing plasters.\n");
            exit(EXIT_FAILURE);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Create a texture resource so this can be easily rendered as a sprite for debugging.
        Texture *plaster_tex = oneoff_resource(Texture, plaster->plaster_texture_handle);
        plaster_tex->texture_id = plaster->plaster_texture;
    }
}

void update_plasters(void)
{

    int num = 0;
    float size = 0.2;
    for (int i = 0; i < MAX_NUM_PLASTERS; i++) {
        if (!g_plasters[i].active) continue;
        paint2d_sprite(size * num, 0, size, size, g_plasters[i].plaster_texture_handle);
        num ++;
    }
}
void print_plasters(void)
{
    printf("--------------------------------------------------------------------------------\n");
    printf("Plasters\n");
    printf("--------------------------------------------------------------------------------\n");
    for (int i = 0; i < MAX_NUM_PLASTERS; i++) {
        if (!g_plasters[i].active) continue;
        printf("camera_position: %.2f %.2f %.2f\n", g_plasters[i].camera_position.vals[0], g_plasters[i].camera_position.vals[1], g_plasters[i].camera_position.vals[2]);
        printf("camera_angles: %.2f %.2f %.2f\n", g_plasters[i].camera_angles.vals[0], g_plasters[i].camera_angles.vals[1], g_plasters[i].camera_angles.vals[2]);
        printf("matrix:\n");
        print_matrix4x4f(&g_plasters[i].matrix);
    }
    printf("--------------------------------------------------------------------------------\n");
}

void create_plaster(Camera *camera, Body *body)
{
    Plaster *plaster = NULL;
    for (int i = 0; i < MAX_NUM_PLASTERS; i++) {
        if (!g_plasters[i].active) plaster = &g_plasters[i];
    }
    if (plaster == NULL) {
        fprintf(stderr, "made too many plasters\n");
        exit(EXIT_FAILURE);
    }
    plaster->active = true;
    Transform *t = get_sibling_aspect(camera, Transform);
    plaster->camera_position = Transform_position(t);
    plaster->camera_angles = Transform_angles(t);
    plaster->matrix = invert_rigid_mat4x4(Transform_matrix(t));
    right_multiply_matrix4x4f(&plaster->matrix, &camera->projection_matrix);

    // Fill the plaster texture.

    GLint prev_viewport[4];
    glGetIntegerv(GL_VIEWPORT, prev_viewport);

    glBindFramebuffer(GL_FRAMEBUFFER, plaster->plaster_framebuffer);
    glClearColor(1,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, PLASTER_WIDTH, PLASTER_HEIGHT);
    mat4x4 model_matrix = Transform_matrix(get_sibling_aspect(body, Transform));
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    model_matrix.vals[4*i + j] *= body->scale;
                }
            }
    mat4x4 mvp_matrix = Camera_vp_matrix(g_camera);
    right_multiply_matrix4x4f(&mvp_matrix, &model_matrix);
    set_uniform_mat4x4(Standard3D, mvp_matrix.vals, mvp_matrix.vals);
    gm_draw(*resource_data(Geometry, body->geometry), resource_data(Material, body->material));

    // Can't disable entities, just throw it up to the sky ...
    Transform_move(get_sibling_aspect(body, Transform), new_vec3(0,1000,0));
    g_plastering_body = NULL;

    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void camera_key_input(Input *input, int key, int action, int mods)
{
    Transform *t = get_sibling_aspect(input, Transform);
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE) t->y += 5;
        if (key == GLFW_KEY_LEFT_SHIFT) t->y -= 5;
        if (key == GLFW_KEY_P && g_plastering_body != NULL) create_plaster(g_camera, g_plastering_body);
        if (key == GLFW_KEY_I) {
            EntityID e = new_entity(4);
            Body *body = entity_add_aspect(e, Body);
            body->scale = 10;
            body->material = Material_create("Materials/red");
            body->geometry = new_resource_handle(Geometry, "Models/block");
            entity_add_aspect(e, Transform);
            g_plastering_body = body;
        }
    }
}

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_O) print_plasters();
    }
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
    update_plasters();
}
extern void close_program(void)
{
}