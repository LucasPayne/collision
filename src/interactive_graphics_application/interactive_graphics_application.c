/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
    + scenes
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"
#include "scenes.h"

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
    CASE(PRESS, O) open_scene(g_data, "Scenes/scene2");
    CASE(PRESS, L) printf("Pressed L\n");
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
#if 1
    for (int i = 0; i < 200; i++)
    { 
        EntityID thing = new_entity(3);
        Transform_set(entity_add_aspect(thing, Transform), frand()*50-25,frand()*50-25,frand()*50-25, frand()*2*M_PI, frand()*2*M_PI, frand()*2*M_PI);
        Body *body = entity_add_aspect(thing, Body);
        body->scale = 1;
        Material *mat = oneoff_resource(Material, body->material);
        mat->material_type = new_resource_handle(MaterialType, "Materials/tinted_texture");
        material_set_property_vec4(mat, "flat_color", new_vec4(0,0,0,1));
        float f = frand();
        if (f > 0.7) {
	    material_set_texture_path(mat, "diffuse_map", "Textures/minecraft/dirt");
        } else if (f > 0.3) {
            material_set_texture_path(mat, "diffuse_map", "Textures/minecraft/ladder");
        } else {
            material_set_texture_path(mat, "diffuse_map", "Textures/minecraft/stone_bricks");
        }
        body->geometry = new_resource_handle(Geometry, "Models/quad");
        Logic *logic = entity_add_aspect(thing, Logic);
        logic->update = logic_misc1;
    }
#endif
    
    // open_scene(g_data, "Scenes/scene1");
    // ResourceHandle r1 = new_resource_handle(Geometry, "Models/quad");
    // resource_data(Geometry, r1);
    // ResourceHandle r2 = new_resource_handle(Geometry, "Models/quad");
    // resource_data(Geometry, r2);

}


vec4 str_to_color_key(char *color)
{
    #define col(STR,R,G,B,A) if (strcmp(color, ( STR )) == 0) return new_vec4((R),(G),(B),(A));
    col("g", 0,1,0,1);
    col("r", 1,0,0,1);
    col("y", 1,0,1,1);
    col("b", 0,0,1,1);
    col("k", 0,0,0,1);
    col("w", 1,1,1,1);
    col("gr", 0.4,0.4,0.4,1);
    fprintf(stderr, ERROR_ALERT "Unsupported color \"%s\".\n", color);
    exit(EXIT_FAILURE);
    #undef col
}


extern void loop_program(void)
{
    // paint_line(new_vec3(0,0,0), new_vec3(50,50,50), "g");

    for (int i = 0; i < 100; i++) {
        float theta = 6 * i * 2*M_PI / 100;
        float thetap = 6 * (i + 1) * 2*M_PI / 100;
        // paint_line(new_vec3(sin(theta), cos(theta), i*0.1), new_vec3(sin(thetap), cos(thetap), i*0.1), str_to_color_key("g"));
        paint_line(new_vec3(sin(theta), cos(theta), i*0.1), new_vec3(sin(thetap), cos(thetap), i*0.1), new_vec4(0,0,i/100.0,1));
    }
}


extern void close_program(void)
{
    printf("i'm out\n");
}
