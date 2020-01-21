/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
    + scenes
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"
#include "scenes.h"

static void camera_controls(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    float rotate_speed = 3;
    if (alt_arrow_key_down(Right)) t->theta_y += rotate_speed * dt;
    if (alt_arrow_key_down(Left)) t->theta_y -= rotate_speed * dt;
    if (alt_arrow_key_down(Up)) t->theta_x -= rotate_speed * dt;
    if (alt_arrow_key_down(Down)) t->theta_x += rotate_speed * dt;
    float speed = 10;
    if (arrow_key_down(Right)) t->x -= speed * dt;
    if (arrow_key_down(Left)) t->x += speed * dt;
    if (arrow_key_down(Up)) t->z += speed * dt;
    if (arrow_key_down(Down)) t->z -= speed * dt;
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

extern void input_event(int key, int action, int mods)
{
#define CASE(ACTION,KEY)\
    if (action == ( GLFW_ ## ACTION ) && key == ( GLFW_KEY_ ## KEY ))
    // CASE(PRESS, K) printf("pressed k\n");
}

// Entity-attached input handlers.
#define InputListener(NAME) void NAME (Input *inp, int key, int action, int mods)
InputListener(input_test_1)
{
    Transform *t = get_sibling_aspect(inp, Transform);
    CASE(PRESS, I) t->x -= 1;
} 
#undef CASE

extern void init_program(void)
{
    resource_path_add("Meshes", "resources/meshes");
    resource_path_add("Images", "resources/images");
    resource_path_add("Shaders", "resources/shaders");

    EntityID camera_man = new_entity(4);
    Transform_set(entity_add_aspect(camera_man, Transform), 0,0,0,0,0,0);
    Camera *camera = entity_add_aspect(camera_man, Camera);
    Camera_init(camera, ASPECT_RATIO, 1, 0.9, 10);
    Logic *logic = entity_add_aspect(camera_man, Logic);
    logic->update = camera_controls;

#define init_get_logic_data(DATA_LVALUE,LOGIC_ASPECT_POINTER,DATA_STRUCT,UPDATE_FUNCTION)\

    // Textured thing
#if 0
    { 
        EntityID thing = new_entity(3);
        Transform_set(entity_add_aspect(thing, Transform), 0,0,-10,0,0,0);
        Body *body = entity_add_aspect(thing, Body);
        body->scale = 1;
        Material *mat = oneoff_resource(Material, body->material);
        mat->material_type = new_resource_handle(MaterialType, "Materials/tinted_texture");
        material_set_property_vec4(mat, "flat_color", new_vec4(0,0,0,1));
        material_set_texture_path(mat, "diffuse_map", "Textures/minecraft/dirt");
        body->geometry = new_resource_handle(Geometry, "Models/quad");
    }
#endif
    
    open_scene(g_data, "Scenes/scene1");

}


extern void loop_program(void)
{
    // for_aspect(Camera, camera)
    //     Transform *t = get_sibling_aspect(camera, Transform);
    //     t->theta_y = sin(time);
    //     t->y = sin(time);
    // end_for_aspect()
}


extern void close_program(void)
{
    printf("i'm out\n");
}
