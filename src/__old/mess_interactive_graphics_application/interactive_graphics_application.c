/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include "bases/interactive_3D.h"


static Material *g_mat;


static GLuint g_test_vao;
static GLuint g_shader_test;

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

extern void init_program(void)
{
    resource_path_add("Meshes", "/home/lucas/collision/src/interactive_graphics_application/resources/meshes");
    resource_path_add("Images", "/home/lucas/collision/src/interactive_graphics_application/resources/images");
    resource_path_add("Shaders", "/home/lucas/collision/src/interactive_graphics_application/resources/shaders");

    EntityID camera_man = new_entity(4);
    Transform_set(entity_add_aspect(camera_man, Transform), 0,0,0,0,0,0);
    Camera *camera = entity_add_aspect(camera_man, Camera);
    Camera_init(camera, ASPECT_RATIO, 1, 0.9, 10);

#if 0
    // Textured thing
    { 
        EntityID thing = new_entity(3);
        Transform_set(entity_add_aspect(thing, Transform), 0,0,3,0,0,0);
        Body *body = entity_add_aspect(thing, Body);
        Material *mat = oneoff_resource(Material, body->material);
        mat->material_type = new_resource_handle(MaterialType, "Materials/tinted_texture");
        material_set_property_vec4(mat, "flat_color", new_vec4(1,1,0,1));
        material_set_texture_path(mat, "diffuse_map", "Textures/archimedes");
        body->geometry = new_resource_handle(Geometry, "Models/quad");
    }
#endif

#if 0
    ResourceHandle test = new_resource_handle(MaterialType, "Materials/red");
    resource_data(MaterialType, test);
#endif

    make_thing(0,0,-5);
    make_thing(-6,0,-5);
    make_thing(6,0,5);

#if 0
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = -5; k < 5; k++) {
                make_thing(i, j, k);
            }
        }
    }
#endif


    ResourceHandle res;
    g_mat = oneoff_resource(Material, res);
    g_mat->material_type = new_resource_handle(MaterialType, "Materials/red");


    // GLuint vertex_data_buffer;
    // glGenBuffers(1, &vertex_data_buffer);
    // glBindBuffer(GL_ARRAY_BUFFER, vertex_data_buffer);
    // float vertex_data[4*2] = {
    //     // Positions
    //     -0.5, -0.5,
    //     0.5, -0.5,
    //     0.5, 0.5,
    //     -0.5, 0.5,
    // };
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_DYNAMIC_DRAW);

    // GLuint vao;
    // const int location_vPosition = 0;
    // glGenVertexArrays(1, &vao);
    // glBindVertexArray(vao);
    // glVertexAttribPointer(location_vPosition, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    // glEnableVertexAttribArray(location_vPosition);

    // g_test_vao = vao;

    // char *vertex_shader_path = "test/shader.vert";
    // char *fragment_shader_path = "test/shader.frag";
    char *vertex_shader_path = "resources/shaders/passthrough_none.vert";
    char *fragment_shader_path = "resources/shaders/red.frag";
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    load_and_compile_shader(vertex_shader, vertex_shader_path);
    load_and_compile_shader(fragment_shader, fragment_shader_path);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    link_shader_program(shader_program);
    g_shader_test = shader_program;
    
}

extern void loop_program(void)
{
    for_aspect(Camera, camera)
        Transform *t = get_sibling_aspect(camera, Transform);
        t->theta_y = sin(time);
        t->y = sin(time);
    end_for_aspect()
    // for_aspect(Body, body)
    //     printf("body geometry path: %s\n", body->geometry.data.path);
    // end_for_aspect()

   //  gm_triangles(VERTEX_FORMAT_3);
   //  attribute_3f(Position, 0,0,0);
   //  attribute_3f(Position, 1,0,0);
   //  attribute_3f(Position, 1,1,0);
   //  attribute_3f(Position, 0,1,0);
   //  gm_index(0); gm_index(1); gm_index(2);
   //  gm_index(0); gm_index(2); gm_index(3);
   //  Geometry g = gm_done();
   //  //gm_draw(g, g_mat);

   //  gm_free(g);

    // glUseProgram(g_shader_test);
    // glBindVertexArray(g.vao_id);
    // glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    // gm_free(g);
}


extern void close_program(void)
{
    printf("i'm out\n");
}
