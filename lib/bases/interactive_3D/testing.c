/*--------------------------------------------------------------------------------
    Testing functions. These are not general things, but functions which can for example
    create a cube to test phong lighting, etc.

---
This might be a good place to start changing entity definitions to be data-driven.
Then instead of these spawning functions, an object would be spawned from a testing dictionary,
for example spawn_gameobject("Testing/Objects/point_light_1", 0,4,0,  0,0,0);
---
Then functions need to be able to be referenced by string in the dictionaries, and somehow correlated to
a wanted function pointer (this could be by name, or sorting them somehow into directories (?)).
Right now a scripting language seems like a far more complex problem, but would be extremely useful, since
most logic would be defined for a specific thing, and it would be nice to create random logic without altering
code, providing function pointers, etc. Then scripts could even be a type in the .dd system, and also sourced with #include(...).
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

/*--------------------------------------------------------------------------------
    Testing entity control schemes.
--------------------------------------------------------------------------------*/
// Control with arrow keys, x-z.
static void test_controls_1(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    float speed = 20;
    if (arrow_key_down(Right)) t->x -= speed * dt;
    if (arrow_key_down(Left)) t->x += speed * dt;
    if (arrow_key_down(Up)) t->z += speed * dt;
    if (arrow_key_down(Down)) t->z -= speed * dt;
}

/*--------------------------------------------------------------------------------
    Testing lights.
--------------------------------------------------------------------------------*/
// Self-rotating directional light.
static void test_directional_light_auto_update(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    // printf("Light is going.\n");
    t->theta_y += 0.5 * dt;
}
void test_directional_light_auto(void)
{
    EntityID light = new_entity(4);
    Transform_set(entity_add_aspect(light, Transform), 0,0,0,  0,0,0);
    Logic *logic = entity_add_aspect(light, Logic);
    logic->update = test_directional_light_auto_update;
    DirectionalLight *directional_light = entity_add_aspect(light, DirectionalLight);
    directional_light->color = new_vec4(1,0.7,0.7,1);
}
// Directional light controlled with keys M and N.
static void test_directional_light_controlled_key_input(Input *input, int key, int action, int mods)
{
    Transform *t = get_sibling_aspect(input, Transform);
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_M) t->theta_x += 0.5;
        if (key == GLFW_KEY_N) t->theta_x -= 0.5;
    }
}
void test_directional_light_controlled(void)
{
    EntityID light = new_entity(4);
    Transform_set(entity_add_aspect(light, Transform), 0,0,0,  3,-1,1);
    DirectionalLight *directional_light = entity_add_aspect(light, DirectionalLight);
    directional_light->color = new_vec4(1,1,0.1,1);
    Input_init(entity_add_aspect(light, Input), INPUT_KEY, test_directional_light_controlled_key_input, true);
}
// Point light with controls and model.
void test_point_light_1(void)
{
    EntityID light = new_entity(4);
    Transform_set(entity_add_aspect(light, Transform), 1,3,0,  0,0,0);
    PointLight_init(entity_add_aspect(light, PointLight),  0.07,0,0,  1,1,1,1);
    Logic *logic = entity_add_aspect(light, Logic);
    logic->update = test_controls_1;

    Body *body = entity_add_aspect(light, Body);
    body->scale = 0.003;
    body->material = Material_create("Materials/red");
    body->geometry = new_resource_handle(Geometry, "Models/icosohedron");
}

/*--------------------------------------------------------------------------------
    Testing objects.
--------------------------------------------------------------------------------*/
// spawn 5 textured, phong-lit cubes.

void test_spawn_cubes(int n)
{
    for (int i = 0; i < n; i++) {
        EntityID quad = new_entity(4);
        Transform_set(entity_add_aspect(quad, Transform), frand()*50-25,frand()*50-25,frand()*50-25, frand()*M_PI*2,0,0);
        Body *body = entity_add_aspect(quad, Body);
        body->scale = 5;
        body->material = Material_create("Materials/textured_phong_shadows");
        material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/mario/sand_bricks");
        body->geometry = new_resource_handle(Geometry, "Models/block");
        // Logic *logic = entity_add_aspect(quad, Logic);
        // logic->update = quad_test_update;
        // Input_init(entity_add_aspect(quad, Input), INPUT_KEY, input_test_1, true);
    }
}
// Create a floor with a given texture.
void test_floor(char *texture_path)
{
    EntityID quad = new_entity(4);
    Transform_set(entity_add_aspect(quad, Transform), 0,0,2,  M_PI/2,0,0);
    Body *body = entity_add_aspect(quad, Body);
    body->scale = 50;
    body->material = Material_create("Materials/textured_phong");
    material_set_texture_path(resource_data(Material, body->material), "diffuse_map", texture_path);
    body->geometry = new_resource_handle(Geometry, "Models/quad");
}
// Test a lot (if wanted) of things appearing.
void test_mass_objects(int number_of_them)
{
    for (int i = 0; i < number_of_them; i++)
    { 
        EntityID thing = new_entity(3);
        Transform_set(entity_add_aspect(thing, Transform), frand()*50-25,frand()*50-25,frand()*50-25, frand()*2*M_PI, frand()*2*M_PI, frand()*2*M_PI);
        Body *body = entity_add_aspect(thing, Body);
        body->scale = 1;
        Material *mat = oneoff_resource(Material, body->material);
        mat->material_type = new_resource_handle(MaterialType, "Materials/textured_phong");
        float f = frand();
        if (f > 0.7) {
            material_set_texture_path(mat, "diffuse_map", "Textures/minecraft/dirt");
        } else if (f > 0.3) {
            material_set_texture_path(mat, "diffuse_map", "Textures/minecraft/ladder");
        } else {
            material_set_texture_path(mat, "diffuse_map", "Textures/minecraft/stone_bricks");
        }
        body->geometry = new_resource_handle(Geometry, "Models/block");
    }
}
// Test painted stars.
static void test_star_update(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    t->x += 4 * sin(time) * dt;
    t->y += 4 * cos(time) * dt;
    t->theta_x -= 1.1 * dt;
    t->theta_y += 1.5 * dt;
    t->theta_z -= 1.3 * dt;

    const int n = 5;
    vec4 colors[] = {
        new_vec4(1,0,0,1),
        new_vec4(1,1,0,1),
        new_vec4(1,1,1,1),
        new_vec4(1,0,1,1),
        new_vec4(0.3,0.5,1,1),
    };
    float speed = 5;
    float a = speed*time - (int) (speed*time);
    
    for (int i = 0; i < n; i++) {

        float mul = (a + i*1.0/n);
        mul -= (int) mul;

        float size = 2;
        float vertices[3 * 10];
        for (int i = 0; i < 10; i++) {
            float outward = i % 2 == 0 ? size : size/2;
            float x = cos(i * 2*M_PI/10);
            float y = sin(i * 2*M_PI/10);
            *((vec3 *) &vertices[3*i]) = vec3_add(Transform_position(t), Transform_relative_direction(t, new_vec3(x*outward*mul, y*outward*mul, 0)));
        }
        paint_loop_v(vertices, 10, colors[i]);
    }
}
void test_spawn_star(float x, float y, float z, float theta_x, float theta_y, float theta_z)
{
    EntityID entity = new_entity(4);
    Transform_set(entity_add_aspect(entity, Transform), x,y,z,  theta_x,theta_y,theta_z);
    Logic *logic = entity_add_aspect(entity, Logic);
    logic->update = test_star_update;
}
void test_spawn_stars(int how_many)
{
    for (int i = 0; i < how_many; i++) test_spawn_star(frand()*50-25, frand()*50-25,frand()*50-25,  2*M_PI*frand(),2*M_PI*frand(),2*M_PI*frand());
}
