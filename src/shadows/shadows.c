/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

GLuint test_framebuffer;
GLuint test_framebuffer_texture;
GLuint test_depthbuffer_texture;
ResourceHandle test_framebuffer_texture_handle;
ResourceHandle test_depthbuffer_texture_handle;
const int test_fb_width = 512;
const int test_fb_height = 512;
EntityID test_framebuffer_viewer;
EntityID test_depth_viewer;
bool test_framebuffer_continuously = false;

void render_to_texture(void)
{
    // Moving it out of the way like this for now since entities and aspects can't be disabled.
    Transform_move(get_aspect_type(test_framebuffer_viewer, Transform), new_vec3(0,1000,0));
    Transform_move(get_aspect_type(test_depth_viewer, Transform), new_vec3(0,1000,0));
    paint2d_rect_c(1-0.1,1-0.1,  0.05,0.05,  "r");
    // Testing framebuffers and rendering to a texture.
    glBindFramebuffer(GL_FRAMEBUFFER, test_framebuffer);
    GLint prev_viewport[4];
    glGetIntegerv(GL_VIEWPORT, prev_viewport);
    glViewport(0, 0, test_fb_width, test_fb_height);
    glClearColor(1,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ___render();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
    Transform_move(get_aspect_type(test_framebuffer_viewer, Transform), new_vec3(0,-1000,0));
    Transform_move(get_aspect_type(test_depth_viewer, Transform), new_vec3(0,-1000,0));
}

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_V) render_to_texture();
        if (key == GLFW_KEY_X) test_framebuffer_continuously = !test_framebuffer_continuously;
        if (key == GLFW_KEY_C) test_spawn_cubes(5);
        if (key == GLFW_KEY_Y) test_spawn_stars(10);
    }
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    painting_init();
    create_key_camera_man(0,20,60,  0,0,0);
{
    test_floor("Textures/minecraft/stone_bricks");
    EntityID cube = new_entity(4);
    Transform_set(entity_add_aspect(cube, Transform), 0,3,0,  0,0,0);
    Body *body = entity_add_aspect(cube, Body);
    body->scale = 20;
    body->material = Material_create("Materials/textured_phong_shadows");
    material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/minecraft/dirt");
    body->geometry = new_resource_handle(Geometry, "Models/block");
}

    test_directional_light_controlled();

    // Testing framebuffers and rendering to a texture.
    // Create the texture to render to.
{
    glGenTextures(1, &test_framebuffer_texture);
    glBindTexture(GL_TEXTURE_2D, test_framebuffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, test_fb_width, test_fb_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    Texture *tex = oneoff_resource(Texture, test_framebuffer_texture_handle);
    tex->texture_id = test_framebuffer_texture;
}
    // Also testing rendering the depth buffer to a texture.
{
    glGenTextures(1, &test_depthbuffer_texture);
    glBindTexture(GL_TEXTURE_2D, test_depthbuffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, test_fb_width, test_fb_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    Texture *tex = oneoff_resource(Texture, test_depthbuffer_texture_handle);
    tex->texture_id = test_depthbuffer_texture;
}

    // Create the framebuffer and attach this texture as a color buffer.
    glGenFramebuffers(1, &test_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, test_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, test_framebuffer_texture, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, test_depthbuffer_texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, ERROR_ALERT "incomplete framebuffer");
        exit(EXIT_FAILURE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
{
    EntityID viewer = new_entity(4);
    Transform_set(entity_add_aspect(viewer, Transform),  0,50,-100,  M_PI,0,0);
    Body *body = entity_add_aspect(viewer, Body);
    body->scale = 100;
    body->material = Material_create("Materials/texture");
    material_set_texture(resource_data(Material, body->material), "diffuse_map", test_framebuffer_texture_handle);
    body->geometry = new_resource_handle(Geometry, "Models/quad");
    test_framebuffer_viewer = viewer;
}
{
    EntityID viewer = new_entity(4);
    Transform_set(entity_add_aspect(viewer, Transform),  100,50,0,  M_PI,-M_PI/2,0);
    Body *body = entity_add_aspect(viewer, Body);
    body->scale = 100;
    body->material = Material_create("Materials/render_projection_shadow_map");
    material_set_texture(resource_data(Material, body->material), "shadow_map", test_depthbuffer_texture_handle);
    body->geometry = new_resource_handle(Geometry, "Models/quad");
    test_depth_viewer = viewer;
}
}
extern void loop_program(void)
{
    if (test_framebuffer_continuously) render_to_texture();
}
extern void close_program(void)
{
}
