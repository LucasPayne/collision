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
    static float velocity_y = 0.0;
    static bool on_ground = false;
    

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
    return;
    if (alt_arrow_key_down(Right)) t->theta_y += speed * dt;
    if (alt_arrow_key_down(Left)) t->theta_y -= speed * dt;

    if (alt_arrow_key_down(Up)) t->z += speed * dt;
    if (alt_arrow_key_down(Down)) t->z -= speed * dt;

    velocity_y -= dt;
    t->y -= velocity_y;
    if (t->y < 2) {
        on_ground = true;
        t->y = 0;
        velocity_y = 0;
    }
}
static void test_controls(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    float speed = 20;
    if (arrow_key_down(Right)) t->x -= speed * dt;
    if (arrow_key_down(Left)) t->x += speed * dt;
    if (arrow_key_down(Up)) t->z += speed * dt;
    if (arrow_key_down(Down)) t->z -= speed * dt;
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
        body->geometry = new_resource_handle(Geometry, "Models/block");
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
    // spawn_cubes(5);
    #if 1 // create directional lights
    {
        EntityID light = new_entity(4);
        Transform_set(entity_add_aspect(light, Transform), 0,0,0,  0,0,0);
        Logic *logic = entity_add_aspect(light, Logic);
        logic->update = light_test_update;
        DirectionalLight *directional_light = entity_add_aspect(light, DirectionalLight);
        directional_light->color = new_vec4(1,0.7,0.7,1);
    }
    {
        EntityID light = new_entity(4);
        Transform_set(entity_add_aspect(light, Transform), 0,0,0,  0,0,0);
        DirectionalLight *directional_light = entity_add_aspect(light, DirectionalLight);
        directional_light->color = new_vec4(1,1,0.1,1);
        Input_init(entity_add_aspect(light, Input), INPUT_KEY, light_test_key, true);
    }
    #endif
    {
        EntityID light = new_entity(4);
        Transform_set(entity_add_aspect(light, Transform), 1,1,0,  0,0,0);
        PointLight_init(entity_add_aspect(light, PointLight),  0.07,0,0,  1,1,1,1);
        Logic *logic = entity_add_aspect(light, Logic);
        logic->update = test_controls;

        Body *body = entity_add_aspect(light, Body);
        body->scale = 0.003;
        body->material = Material_create("Materials/red");
        body->geometry = new_resource_handle(Geometry, "Models/icosohedron");
    }
    { // make a floor
        EntityID quad = new_entity(4);
        Transform_set(entity_add_aspect(quad, Transform), 0,0,2,  M_PI/2,0,0);
        Body *body = entity_add_aspect(quad, Body);
        body->scale = 50;
        body->material = Material_create("Materials/textured_phong");
        material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/minecraft/stone_bricks");
        body->geometry = new_resource_handle(Geometry, "Models/quad");
        // Input_init(entity_add_aspect(quad, Input), INPUT_KEY, input_test_1, true);
    }
    // open_scene(g_data, "Scenes/scene1");
    // ResourceHandle r1 = new_resource_handle(Geometry, "Models/quad");
    // resource_data(Geometry, r1);
    // ResourceHandle r2 = new_resource_handle(Geometry, "Models/quad");
    // resource_data(Geometry, r2);

    open_scene(g_data, "Scenes/scene1");

}

extern void loop_program(void)
{
    // paint_line(new_vec3(0,0,0), new_vec3(50,50,50), "g");

    const float quad_size = 5;
    static vec4 grid[5*5] = { 0 };
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (frand() < 0.05) grid[5*i + j] = new_vec4(frand(),frand(),frand(),1);
            paint_quad_v(new_vec3(quad_size*i,quad_size*j,0),
                         new_vec3(quad_size*(i+1),quad_size*j,0),
                         new_vec3(quad_size*(i+1),quad_size*(j+1),0),
                         new_vec3(quad_size*i,quad_size*(j+1),0),
                         grid[5*i + j]);
        }
    }

    vec3 spiral[100];
    for (int i = 0; i < 100; i++) {
        float theta = 6 * i * 2*M_PI / 100;
        spiral[i] = new_vec3(sin(theta + time), 3 + cos(theta + time), i*0.1);
    }
    paint_chain_c((float *) spiral, 100, "g");

    float quad_loop[] = {
        0,0,0,  5,0,0,  5,5,0,  0,5,0
    };
    paint_loop(quad_loop, 4, 1,0,0,1);
}


extern void close_program(void)
{
    printf("i'm out\n");
}
