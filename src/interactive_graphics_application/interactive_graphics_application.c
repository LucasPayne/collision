/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
    + scenes
    + painting
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"
#include "scenes.h"
#include "painting.h"

//--------------------------------------------------------------------------------
// Testing and debugging.
//--------------------------------------------------------------------------------
static Material *g_flat_color;
//--------------------------------------------------------------------------------
// Random behaviour/logic functions.
static void logic_misc1(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    t->theta_x += dt;
    t->theta_y += 0.7 * dt;
}
//--------------------------------------------------------------------------------

static void camera_controls(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    // float rotate_speed = 3;
    // if (arrow_key_down(Right)) t->theta_y += rotate_speed * dt;
    // if (arrow_key_down(Left)) t->theta_y -= rotate_speed * dt;
    // if (arrow_key_down(Up)) t->theta_x -= rotate_speed * dt;
    // if (arrow_key_down(Down)) t->theta_x += rotate_speed * dt;
    float speed = 20;
    if (alt_arrow_key_down(Right)) t->x -= speed * dt;
    if (alt_arrow_key_down(Left)) t->x += speed * dt;
    if (alt_arrow_key_down(Up)) t->z += speed * dt;
    if (alt_arrow_key_down(Down)) t->z -= speed * dt;
}
static void quad_test_update(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    float speed = 5;
    if (arrow_key_down(Right)) t->theta_y -= speed * dt;
    if (arrow_key_down(Left)) t->theta_y += speed * dt;
    if (arrow_key_down(Up)) t->theta_x += speed * dt;
    if (arrow_key_down(Down)) t->theta_x -= speed * dt;
}
static void light_test_update(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    // printf("Light is going.\n");
    t->theta_y += 0.5 * dt;
}

void make_thing(float x, float y, float z)
{
    EntityID thing = new_entity(3);
    Transform_set(entity_add_aspect(thing, Transform), x,y,z,0,0,0);
    Body *body = entity_add_aspect(thing, Body);
    body->scale = 0.01;
    Material *mat = oneoff_resource(Material, body->material);
    mat->material_type = new_resource_handle(MaterialType, "Materials/red");
    body->geometry = new_resource_handle(Geometry, "Models/icosohedron");
}
static void spawn_cubes(int n)
{
    for (int i = 0; i < n; i++) {
        EntityID quad = new_entity(4);
        Transform_set(entity_add_aspect(quad, Transform), frand()*50-25,frand()*50-25,-2, frand()*M_PI*2,0,0);
        Body *body = entity_add_aspect(quad, Body);
        body->scale = 1;
        body->material = Material_create("Materials/textured_phong");
        material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/minecraft/stone_bricks");
        body->geometry = new_resource_handle(Geometry, "Models/cube");
        Logic *logic = entity_add_aspect(quad, Logic);
        logic->update = quad_test_update;
        // Input_init(entity_add_aspect(quad, Input), INPUT_KEY, input_test_1, true);
    }
}


extern void input_event(int key, int action, int mods)
{
#define CASE(ACTION,KEY)\
    if (action == ( GLFW_ ## ACTION ) && key == ( GLFW_KEY_ ## KEY ))
    CASE(PRESS, O) open_scene(g_data, "Scenes/scene2");
    CASE(PRESS, L) printf("Pressed L\n");
    CASE(PRESS, C) spawn_cubes(5);
}
extern void cursor_move_event(double x, double y)
{
}

// Entity-attached input handlers.
#define NewKeyListener(NAME) void NAME (Input *inp, int key, int action, int mods)
#define NewMousePositionListener(NAME) void NAME (Input *inp, double x, double y)
#define NewMouseMoveListener(NAME) void NAME (Input *inp, double dx, double dy)
NewKeyListener(input_test_1)
{
    Transform *t = get_sibling_aspect(inp, Transform);
    CASE(PRESS, I) t->x -= 1;
}
NewMousePositionListener(mouse_position_test_1)
{
    printf("mouse position: %g, %g\n", x, y);
}

NewMouseMoveListener(camera_mouse_move)
{
    Transform *t = get_sibling_aspect(inp, Transform);
    printf("Camera position: %f, %f, %f\n", t->x, t->y, t->z);
    printf("mouse velocity: %g, %g\n", dx, dy);

    t->theta_y += dx * -0.002;
    t->theta_x += dy * 0.002;
}

NewKeyListener(light_test_key)
{
    Transform *t = get_sibling_aspect(inp, Transform);
    CASE(PRESS, M) t->theta_x += 0.5;
    CASE(PRESS, N) t->theta_x -= 0.5;
}

#undef CASE

extern void init_program(void)
{
    painting_init(); //---
    
    resource_path_add("Meshes", "resources/meshes");
    resource_path_add("Images", "resources/images");
    resource_path_add("Shaders", "resources/shaders");

    EntityID camera_man = new_entity(4);
    Transform_set(entity_add_aspect(camera_man, Transform), 0,0,0,0,0,0);
    Camera *camera = entity_add_aspect(camera_man, Camera);
    Camera_init(camera, ASPECT_RATIO, 1, 0.9, 10);
    Logic *logic = entity_add_aspect(camera_man, Logic);
    logic->update = camera_controls;
    Input_init(entity_add_aspect(camera_man, Input), INPUT_MOUSE_MOVE, camera_mouse_move, true);

    // Textured thing
#if 0
    for (int i = 0; i < 200; i++)
    { 
        EntityID thing = new_entity(3);
        Transform_set(entity_add_aspect(thing, Transform), frand()*50-25,frand()*50-25,frand()*50-25, frand()*2*M_PI, frand()*2*M_PI, frand()*2*M_PI);
        Body *body = entity_add_aspect(thing, Body);
        body->scale = 1;
        Material *mat = oneoff_resource(Material, body->material);
        // mat->material_type = new_resource_handle(MaterialType, "Materials/tinted_texture");
        // material_set_property_vec4(mat, "flat_color", new_vec4(0,0,0,1));
        mat->material_type = new_resource_handle(MaterialType, "Materials/textured_phong");
        float f = frand();
        if (f > 0.7) {
	    material_set_texture_path(mat, "diffuse_map", "Textures/minecraft/dirt");
        } else if (f > 0.3) {
            material_set_texture_path(mat, "diffuse_map", "Textures/minecraft/ladder");
        } else {
            material_set_texture_path(mat, "diffuse_map", "Textures/minecraft/stone_bricks");
        }
        // body->geometry = new_resource_handle(Geometry, frand() > 0.5 ? "Models/quad" : "Models/cube");
        body->geometry = new_resource_handle(Geometry, "Models/cube");
        Logic *logic = entity_add_aspect(thing, Logic);
        logic->update = logic_misc1;
        Input_init(entity_add_aspect(thing, Input), INPUT_KEY, input_test_1, true);
    }
#endif
    
    // Lighting testing
    spawn_cubes(5);
    {
        EntityID light = new_entity(4);
        Transform_set(entity_add_aspect(light, Transform), 0,0,0,  0,0,0);
        Logic *logic = entity_add_aspect(light, Logic);
        logic->update = light_test_update;
        DirectionalLight *directional_light = entity_add_aspect(light, DirectionalLight);
        directional_light->color = new_vec4(1,0,0,1);
        Input_init(entity_add_aspect(light, Input), INPUT_KEY, light_test_key, true);
    }

    
    // open_scene(g_data, "Scenes/scene1");
    // ResourceHandle r1 = new_resource_handle(Geometry, "Models/quad");
    // resource_data(Geometry, r1);
    // ResourceHandle r2 = new_resource_handle(Geometry, "Models/quad");
    // resource_data(Geometry, r2);

}

extern void loop_program(void)
{
    // paint_line(new_vec3(0,0,0), new_vec3(50,50,50), "g");

    for (int i = 0; i < 100; i++) {
        float theta = 6 * i * 2*M_PI / 100;
        float thetap = 6 * (i + 1) * 2*M_PI / 100;
        // paint_line(new_vec3(sin(theta), cos(theta), i*0.1), new_vec3(sin(thetap), cos(thetap), i*0.1), str_to_color_key("g"));
        paint_line(sin(theta), cos(theta), i*0.1,
                   sin(thetap), cos(thetap), i*0.1,
                   0,0,i/100.0,1);
    }
    float loop[] = {
        0,0,0,  5,0,0,  5,5,0,  0,5,0
    };
    paint_loop(loop, 4, 1,0,0,1);
}


extern void close_program(void)
{
    printf("i'm out\n");
}
