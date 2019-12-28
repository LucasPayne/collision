/*================================================================================
PROJECT_LIBS:
    + glad
    + helper_gl
    + helper_input
    + dictionary
    + resources
    + rendering
    + ply
    + iterator
    + entity
    + matrix_mathematics
    + aspect_library/gameobjects
================================================================================*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "helper_definitions.h"
#include "helper_gl.h"
#include "helper_input.h"
#include "ply.h"
#include "rendering.h"
#include "resources.h"
#include "entity.h"
#include "matrix_mathematics.h"
#include "aspect_library/gameobjects.h"

ShaderBlockID ShaderBlockID_StandardLoopWindow;
typedef struct ShaderBlock_StandardLoopWindow_s {
    // padding bytes would be in here if neccessary.
    float aspect_ratio;
    float time;
} ShaderBlock_StandardLoopWindow;


typedef struct mat4x4_s {
    float vals[16];
} mat4x4;
ShaderBlockID ShaderBlockID_Standard3D;
typedef struct ShaderBlock_Standard3D_s {
    mat4x4 mvp_matrix;
} ShaderBlock_Standard3D;

ShaderBlockID ShaderBlockID_DirectionalLights;
typedef struct ShaderBlock_DirectionalLights_s {
    float aspect_ratio;
    float time;
} ShaderBlock_DirectionalLights;


typedef struct CameraControlData_s {
    float move_speed;
    float rotate_speed;
    bool frozen;
    Matrix4x4f frozen_matrix;
} CameraControlData;
static void camera_controls_update(Logic *logic)
{
    get_logic_data(data, logic, CameraControlData);

    Transform *transform = get_sibling_aspect(logic, Transform);
    /* printf("move speed: %.2f\n", data->move_speed); */
    if (arrow_key_down(Left)) { 
        transform->x += data->move_speed * dt();
    }
    if (arrow_key_down(Right)) {
        transform->x -= data->move_speed * dt();
    }
    if (arrow_key_down(Up)) {
        transform->z += data->move_speed * dt();
    }
    if (arrow_key_down(Down)) {
        transform->z -= data->move_speed * dt();
    }
    if (alt_arrow_key_down(Left)) {
        transform->theta_y -= data->rotate_speed*dt();
    }
    if (alt_arrow_key_down(Right)) {
        transform->theta_y += data->rotate_speed*dt();
    }
    if (alt_arrow_key_down(Up)) {
        transform->theta_x -= data->rotate_speed*dt();
        if (transform->theta_x < -M_PI/3) transform->theta_x = -M_PI/3;
    }
    if (alt_arrow_key_down(Down)) {
        transform->theta_x += data->rotate_speed*dt();
        if (transform->theta_x > M_PI/3) transform->theta_x = M_PI/3;
    }
}

static void rotate_update(Logic *logic)
{
    Transform *transform = get_sibling_aspect(logic, Transform);
    transform->theta_y += 2 * dt();
    transform->theta_z += 1.6 * dt();
    transform->theta_x += 2.3 * dt();
}


enum FloorMode {
    FLOOR_ROTATING,
    FLOOR_STATIC,
};
typedef struct FloorData_s {
    float rotate_speed;
    int mode;
    float rotate_amount;
    float cur_rotate;
    bool tilt_side;
} FloorData;
static void floor_update(Logic *logic)
{
    Transform *transform = get_sibling_aspect(logic, Transform);
    get_logic_data(data, logic, FloorData);

    Body *body = get_sibling_aspect(logic, Body);

    if (data->tilt_side) {
        transform->theta_y = 0.3*sin(time() + 0.01*transform->x);
    }
    else {
        transform->theta_y = -0.3*sin(time() + 0.01*transform->x);
    }
    body->scale = 20+2.5*(sin(time() + 0.01*transform->x)/2 + 1);

    if (data->mode == FLOOR_STATIC) {
        if (frand() < 0.001) {
            data->mode = FLOOR_ROTATING;
            int i = rand() % 3;
            data->rotate_amount = (M_PI/2) * i;
            data->cur_rotate = 0;
        }
    } else if (data->mode == FLOOR_ROTATING) {
        float theta = data->rotate_speed * dt();
        data->cur_rotate += theta;
        if (data->cur_rotate > data->rotate_amount) {
            theta = data->rotate_amount - (data->cur_rotate - theta);
            data->mode = FLOOR_STATIC;
        }
        transform->theta_z += theta;
    }
}

void make_floor(int x, int z)
{
    EntityID floor = new_entity(2);
    Body *body = entity_add_aspect(floor, Body);
    /* Body_init(body, "Materials/floor", "Models/quad"); */
    Body_init(body, "Materials/green", "Models/quad");
    float placing = 20;
    body->scale = placing;
    Transform_set(entity_add_aspect(floor, Transform), 2*placing*x, -12, 2*placing*z,  M_PI/2,0,0);
    /* Transform_set(entity_add_aspect(floor, Transform), offx,-12 - 2*i,offz,  M_PI/2,0,0); */
    Logic *logic = entity_add_aspect(floor, Logic);
    init_get_logic_data(data, logic, FloorData, floor_update);
    data->rotate_speed = 5;
    data->mode = FLOOR_STATIC;
    data->rotate_amount = 0;
    data->cur_rotate = 0;
    data->tilt_side = x % 2 == 0 ? false : true;
}

static float ASPECT_RATIO;

void init_program(void)
{
    init_resources_rendering();
    resource_path_add("Textures", "/home/lucas/code/collision/src/texture_test/textures");
    resource_path_add("Shaders", "/home/lucas/code/collision/src/texture_test/shaders");
    resource_path_add("Models", "/home/lucas/code/collision/data/meshes");
    resource_path_add("Materials", "/home/lucas/code/collision/src/texture_test/materials");

    add_shader_block(StandardLoopWindow);
    add_shader_block(Standard3D);
    add_shader_block(DirectionalLights);
    add_shader_block(MaterialProperties); //----important

    set_uniform_float(StandardLoopWindow, aspect_ratio, ASPECT_RATIO);

    init_entity_model();
    init_aspects_gameobjects();

    {
        EntityID camera_man = new_entity(4);
        Transform_set(entity_add_aspect(camera_man, Transform), 0,0,0,0,0,0);
        Camera *camera = entity_add_aspect(camera_man, Camera);
/* void Camera_init(Camera *camera, float aspect_ratio, float near_half_width, float near, float far); */
        Camera_init(camera, ASPECT_RATIO, 1, 0.9, 10);
    
        Logic *logic = entity_add_aspect(camera_man, Logic);
        init_get_logic_data(data, logic, CameraControlData, camera_controls_update);
        data->move_speed = 12;
        data->rotate_speed = 5;
        data->frozen = false;

        /* Body *body = entity_add_aspect(camera_man, Body); */
        /* Body_init(body, "Materials/simple1", "Models/cube"); */
        /* body->scale = 5; */
    }

    make_floor(0, 0);
#if 0
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            make_floor(i, j);
        }
    }
#endif

#if 0
    for (int j = 0; j < 1; j++) {
        float offx = 60*frand()-30;
        float offz = 60*frand()-30;
        for (int i = 0; i < 20; i++) {
            EntityID floor = new_entity(2);
            Body *body = entity_add_aspect(floor, Body);
            Body_init(body, "Materials/floor", "Models/quad");
            body->scale = 50;
            Transform_set(entity_add_aspect(floor, Transform), offx,-12 - 2*i,offz,  M_PI/2,0,0);
            Logic *logic = entity_add_aspect(floor, Logic);
            init_get_logic_data(data, logic, FloorData, floor_update);
            data->rotate_speed = sin(i*0.1);
        }
    }
#endif
#if 0
    for (int i = 0; i < 100; i++) {
        EntityID thing = new_entity(3);
        Body *body = entity_add_aspect(thing, Body);
        Body_init(body, "Materials/simple1", "Models/dolphins");
        body->scale = 0.08;
        Transform_set(entity_add_aspect(thing, Transform), 300*(2*frand()*frand()-1), 50*(2*frand()*frand()-1), 300*(2*frand()*frand()-1), 0,0,0);
        /* entity_add_aspect(floor, Logic)->update = floor_update; */
    }
#endif
}


void loop(void)
{
    set_uniform_float(StandardLoopWindow, time, time());

    for_aspect(Logic, logic)
        logic->update(logic);
    end_for_aspect()

    for_aspect(Camera, camera)
        get_logic_data(data, get_sibling_aspect(camera, Logic), CameraControlData); // here, using this logic-data for application-specific attached functionality.

        Transform *camera_transform = get_sibling_aspect(camera, Transform);
        Matrix4x4f view_matrix = Transform_matrix(camera_transform);
        Matrix4x4f vp_matrix = camera->projection_matrix;
        if (data->frozen) {
            // For viewing purpouses, when the camera is frozen it keeps using this matrix. However, the other matrices aren't affected,
            // so you can view culling from the outside.
            right_multiply_matrix4x4f(&vp_matrix, &data->frozen_matrix);
        } else {
            right_multiply_matrix4x4f(&vp_matrix, &view_matrix);
        }

#if 0
        // Calculate some camera positioning information for further usage in frustum culling.
        // Unit-length camera direction vector
        vec3 n_camera_dir; {
            vec3 v = {0,0,1};
            n_camera_dir = matrix4_vec3_normal(&view_matrix, v);
        }
#if 0
        printf("%.2f, %.2f, %.2f\n", view_matrix.vals[0 + 4*2]
                                   ,view_matrix.vals[1 + 4*2]
                                   ,view_matrix.vals[2 + 4*2]);
        printf("%.2f, %.2f, %.2f\n", n_camera_dir.vals[0], n_camera_dir.vals[1], n_camera_dir.vals[2]);
#endif
#endif

        for_aspect(Body, body)
            Transform *transform = get_sibling_aspect(body, Transform);
            Geometry *mesh = resource_data(Geometry, body->geometry);
#if 0
            // Frustum cull
            float r = mesh->bounding_sphere_radius;
            /* vec3 c; */
            /* c.vals[0] = camera_transform->x; */
            /* c.vals[1] = camera_transform->y; */
            /* c.vals[2] = camera_transform->z; */
            /* vec3 v = Transform_global_position(transform); */
            vec4 vp;
            vp.vals[0] = transform->x;
            vp.vals[1] = transform->y;
            vp.vals[2] = transform->z;
            vp.vals[3] = 1;
            vec4 vpp = matrix_vec4(&view_matrix, vp);
            vec3 v = *((vec3*) &vpp);


            // Let v=(x,y,z) be the position of the sphere and r be its radius. For each frustum plane, take a reference point p on the plane and an inward-pointing normal n.
            // Let p' = p - rn be an outwardly extruded plane reference point.
            // Then, if the dot product of the vector v-p' with the vector n is >= 0, the sphere's center is touching/inside the extruded frustum volume.
            // This extruded volume is approximately the Minkowski product of the sphere and the frustum, and is a superset of it, so no unwanted culls will be made,
            // and the intersection test is much simpler.
            // ---
            // Near plane
            /* vec3 rn = vec3_mul(n_camera_dir, camera->plane_n - r); // reference point on the extruded near plane */
            /* if (vec3_dot(v */

            vec3 forward = {{0,0,1}};
            vec3 rn = vec3_mul(forward, camera->plane_n - r);

            if (vec3_dot(v, rn) < 0) {
                continue;
            }
#endif
#if 1
            Material *material = resource_data(Material, body->material);

            Matrix4x4f model_matrix = Transform_matrix(transform);
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    model_matrix.vals[4*i + j] *= body->scale;
                }
            }
            Matrix4x4f mvp_matrix = vp_matrix;
            right_multiply_matrix4x4f(&mvp_matrix, &model_matrix);

            set_uniform_mat4x4(Standard3D, mvp_matrix.vals, mvp_matrix.vals);


            gm_draw(*mesh, material);
#endif

            /* printf("%.2f, %.2f, %.2f\n", v.vals[0], v.vals[1], v.vals[2]); */
            /* getchar(); */
        end_for_aspect()
    end_for_aspect()

#if 0
    {
        gm_triangles(VERTEX_FORMAT_3);
#if 0
        for (int i = 0; i < 100; i++) {
            attribute_3f(Position, frand() * 500, frand() * 500, frand() * 500);
        }
        for (int i = 0; i < 500; i++) {
            gm_index((13*i + i*i*i*i*7 + i*i*31) % 100);
        }
#else
        attribute_3f(Position, 500 + 50*cos(time()), 500, 500);
        attribute_3f(Position, 500 + 50*cos(2*time()), 500, -500);
        attribute_3f(Position, 500 + 50*cos(3*time()), -500, 500);
        attribute_3f(Position, 500 + 50*cos(4*time()), -500, -500);
        attribute_3f(Position, -500, 500, 500);
        attribute_3f(Position, -500, 500, -500);
        attribute_3f(Position, -500, -500, 500);
        attribute_3f(Position, -500, -500, -500);
        gm_index(0); gm_index(1); gm_index(2); gm_index(1); gm_index(3); gm_index(2);
        gm_index(3); gm_index(1); gm_index(5); gm_index(7); gm_index(3); gm_index(5);
        gm_index(5); gm_index(6); gm_index(7); gm_index(5); gm_index(4); gm_index(6);
        gm_index(4); gm_index(0); gm_index(2); gm_index(4); gm_index(2); gm_index(6);
        gm_index(6); gm_index(2); gm_index(3); gm_index(6); gm_index(3); gm_index(7);
        gm_index(4); gm_index(5); gm_index(1); gm_index(4); gm_index(1); gm_index(0);
#endif
        Geometry id = gm_done();
        ResourceHandle mat = new_resource_handle(Material, "Materials/green");
        gm_draw(id, resource_data(Material, mat));
        gm_free(id);
    }

    {
        gm_lines(VERTEX_FORMAT_3);
        attribute_3f(Position, 0, 0, 0);
        attribute_3f(Position, 5000*sin(time()), 5000*cos(time()), 5000);
        Geometry id = gm_done();
        ResourceHandle mat = new_resource_handle(Material, "Materials/green");
        gm_draw(id, resource_data(Material, mat));
        gm_free(id);
    }


    ResourceHandle green = new_resource_handle(Material, "Materials/green");
    ResourceHandle red = new_resource_handle(Material, "Materials/red");

    float size = 200;
    float elevation[11][11];
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            elevation[i][j] = 40 * (sin(5*time() + i) + cos(4.3*time() + j));
        }
    }
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            gm_triangles(VERTEX_FORMAT_3);
            uint32_t a = attribute_3f(Position, size*i, size*j, elevation[i][j]);
            uint32_t b = attribute_3f(Position, size*(i + 1), size*j, elevation[i+1][j]);
            uint32_t c = attribute_3f(Position, size*(i + 1), size*(j + 1), elevation[i+1][j+1]);
            uint32_t d = attribute_3f(Position, size*i, size*(j + 1), elevation[i][j+1]);
            gm_index(a); gm_index(b); gm_index(c);
            gm_index(a); gm_index(c); gm_index(d);
            Geometry g = gm_done();

             


            vec4 color = new_vec4(0.5*(sin(i)+1), 0.5*(cos(j)+1), (sin(j) + cos(i) + 2) / 4, 1);
            memcpy(resource_data(Material, green)->properties, &color, sizeof(color));
            gm_draw(g, resource_data(Material, green));

            /* if ((i + j) % 2 == 0) { */
            /*     gm_draw(g, resource_data(Material, green)); */
            /* } else { */
            /*     gm_draw(g, resource_data(Material, blue)); */
            /* } */
            gm_free(g);

            float cx, cy, cz;
            cx = size*(i + 0.5);
            cy = size*(j + 0.5);
            cz = (elevation[i][j] + elevation[i+1][j] + elevation[i][j+1] + elevation[i+1][j+1])/4.0;
            gm_lines(VERTEX_FORMAT_3 | (1 << ATTRIBUTE_TYPE_INDEX));
            attribute_3f(Position, cx, cy, cz);
            attribute_1u(Index, 0);
            for (int k = 0; k < 10; k++) {
                attribute_3f(Position, cx + 100*sin(k), cy + 100*cos(k), cz + 5*k);
                attribute_1u(Index, k+1);
            }
            g = gm_done();
            ResourceHandle res = new_resource_handle(Material, "Materials/dashed_red");
            gm_draw(g, resource_data(Material, res));
            gm_free(g);
        }
    }
#endif
}
void close_program(void)
{
}
static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
#define CASE(ACTION,KEY)\
    if (action == ( GLFW_ ## ACTION ) && key == ( GLFW_KEY_ ## KEY ))
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);
#if 1
    CASE(PRESS, C) {
        // Should probably make a for_resource_type or for_resource.
#if 0
        for_aspect(Body, body)
            /* reload_resource(&resource_data(MaterialType, resource_data(Material, body->material)->material_type)->shaders[Vertex]); */
            /* reload_resource(&resource_data(MaterialType, resource_data(Material, body->material)->material_type)->shaders[Fragment]); */
            /* reload_resource(&resource_data(Material, body->material)->material_type); */
            reload_resource(&body->material);
        end_for_aspect()
#endif
        // reloading resources that aren't attached to entities, for now without a for_resource kind of thing
        ResourceHandle res = new_resource_handle(Material, "Materials/green");
        reload_resource(&res);
    }
    CASE(PRESS, M) {
        for_aspect(Body, body)
            /* reload_resource(&body->mesh); */
        end_for_aspect()
    }
#endif
    CASE(PRESS, SPACE) {
        for_aspect(Camera, camera)
            get_logic_data(data, get_sibling_aspect(camera, Logic), CameraControlData);
            data->move_speed = 120;
        end_for_aspect()
    }
    CASE(RELEASE, SPACE) {
        for_aspect(Camera, camera)
            get_logic_data(data, get_sibling_aspect(camera, Logic), CameraControlData);
            data->move_speed = 50;
        end_for_aspect()
    }

    // Freeze the camera (to view culling)
    CASE(PRESS, F) {
        for_aspect(Camera, camera)
            get_logic_data(data, get_sibling_aspect(camera, Logic), CameraControlData);
            Transform *transform = get_sibling_aspect(camera, Transform);
            if (!data->frozen) {
                data->frozen = true;
                data->frozen_matrix = Transform_matrix(transform);
            }
            else {
                data->frozen = false;
            }
        end_for_aspect()
    }
#undef CASE
}

void reshape(GLFWwindow* window, int width, int height)
{
    force_aspect_ratio(window, width, height, ASPECT_RATIO);
}
int main(int argc, char *argv[])
{
    GLFWwindow *window;
    char *name = "Texture testing";
    //--------------------------------------------------------------------------------
    if (!glfwInit()) {
        fprintf(stderr, "GLFW error: something went wrong initializing GLFW\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1, 1, name, NULL, NULL);
    if (!window) {
        fprintf(stderr, "GLFW error: failed to create a window properly.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);
    ASPECT_RATIO = SCREEN_ASPECT_RATIO;

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop, GL_COLOR_BUFFER_BIT);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
