/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

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
    CASE(PRESS, K) printf("pressed k\n");
}

// Entity-attached input handlers.
#define InputListener(NAME) void NAME (Input *inp, int key, int action, int mods)
InputListener(input_controls_1)
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
    Input_init(entity_add_aspect(camera_man, Input), input_controls_1, true);

    // Textured thing
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
}


extern void loop_program(void)
{
    for_aspect(Camera, camera)
        Transform *t = get_sibling_aspect(camera, Transform);
        t->theta_y = sin(time);
        t->y = sin(time);
    end_for_aspect()
}


extern void close_program(void)
{
    printf("i'm out\n");
}
