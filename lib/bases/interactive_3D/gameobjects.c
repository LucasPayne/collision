/*--------------------------------------------------------------------------------
"Gameobject" aspects.

Transform
    3D position, orientation, stored with Euler angles.
Body
    Viewable mesh aspect.
Logic
    Per-frame update logic.
Input
Camera
DirectionalLight
PointLight

Currently, this is not really a "library". A useful "game object" system above
the entity and resource systems should probably only be made by editing this a lot
and then deciding what aspects are useful, then making it a proper library.
--------------------------------------------------------------------------------*/
#include "bases/interactive_3D.h"

void init_aspects_gameobjects(void)
{
    new_default_manager(Transform, NULL);
    new_default_manager(Body, NULL);
    new_default_manager(Logic, NULL);
    new_default_manager(Input, NULL);
    new_default_manager(Camera, NULL);
    new_default_manager(DirectionalLight, NULL);
    new_default_manager(PointLight, NULL);
}

/*================================================================================
    Transform
================================================================================*/
AspectType Transform_TYPE_ID;
void Transform_set(Transform *transform, float x, float y, float z, float theta_x, float theta_y, float theta_z)
{
    transform->x = x;
    transform->y = y;
    transform->z = z;
    transform->theta_x = theta_x;
    transform->theta_y = theta_y;
    transform->theta_z = theta_z;
}
vec3 Transform_position(Transform *t)
{
    return new_vec3(t->x, t->y, t->z);
}
Matrix4x4f Transform_matrix(Transform *transform)
{
    Matrix4x4f mat;
    translate_rotate_3d_matrix4x4f(&mat, transform->x, transform->y, transform->z, transform->theta_x, transform->theta_y, transform->theta_z);
    return mat;
}
vec3 Transform_relative_direction(Transform *t, vec3 direction)
{
    return matrix_vec3(rotation_part_rigid_mat4x4(Transform_matrix(t)), direction);
}
void Transform_move(Transform *t, vec3 translation)
{
    //--idea: could make vectors structs with x,y,z[,w], and cast to an array when iteration is wanted.
    t->x += translation.vals[0];
    t->y += translation.vals[1];
    t->z += translation.vals[2];
}
void Transform_move_relative(Transform *t, vec3 translation)
{
    Transform_move(t, Transform_relative_direction(t, translation));
}


/*================================================================================
    Body
================================================================================*/
AspectType Body_TYPE_ID;
void Body_init(Body *body, char *material_path, char *mesh_path)
{
    body->scale = 1;
    body->material = new_resource_handle(Material, material_path);
    body->geometry = new_resource_handle(Geometry, mesh_path);
}

/*================================================================================
    Logic
================================================================================*/
AspectType Logic_TYPE_ID;

/*================================================================================
    Input
================================================================================*/
AspectType Input_TYPE_ID;
void Input_init(Input *inp, uint8_t input_type, /* generic function pointer (no type safety) */ void *callback, bool listening)
{
    switch (input_type) {
        case INPUT_KEY: break;
        case INPUT_MOUSE_POSITION: break;
        case INPUT_MOUSE_MOVE: break;
        default:
            fprintf(stderr, ERROR_ALERT "Invalid input type given when creating an Input aspect.\n");
            exit(EXIT_FAILURE);
    }
    inp->input_type = input_type;
    inp->callback.key = (KeyListener) callback; // cast to a function, does not matter which type.
    inp->listening = listening;
}

/*================================================================================
    Lights
================================================================================*/
AspectType DirectionalLight_TYPE_ID;
void DirectionalLight_init(DirectionalLight *directional_light, float cr, float cg, float cb, float ca)
{
    directional_light->color = new_vec4(cr, cg, cb, ca);
}
vec3 DirectionalLight_direction(DirectionalLight *directional_light)
{
    // The direction of the light is in the light entity's local z direction.
    return Transform_relative_direction(get_sibling_aspect(directional_light, Transform), new_vec3(0,0,1));
}

AspectType PointLight_TYPE_ID;
void PointLight_init(PointLight *point_light, float linear_attenuation, float quadratic_attenuation, float cubic_attenuation, float cr, float cg, float cb, float ca)
{
    point_light->color = new_vec4(cr, cg, cb, ca);
    point_light->linear_attenuation = linear_attenuation;
    point_light->quadratic_attenuation = quadratic_attenuation;
    point_light->cubic_attenuation = cubic_attenuation;
}


/*================================================================================
    Camera
================================================================================*/
AspectType Camera_TYPE_ID;
void Camera_init(Camera *camera, float aspect_ratio, float near_half_width, float near, float far)
{
    float l,r,b,t,n,f;
    r = near_half_width;
    l = -r;
    t = aspect_ratio * r;
    b = -t;
    n = near;
    f = far;

    Matrix4x4f frustrum_matrix = {0};
    frustrum_matrix.vals[4*0 + 0] = (2*n)/(r-l);
    frustrum_matrix.vals[4*1 + 1] = (2*n)/(t-b);
    frustrum_matrix.vals[4*2 + 2] = -(f+n)/(f-n);
    frustrum_matrix.vals[4*0 + 2] = (r+l)/(r-l);
    frustrum_matrix.vals[4*1 + 2] = (t+b)/(t-b);
    frustrum_matrix.vals[4*2 + 3] = -(2*n*f)/(f-n);
    frustrum_matrix.vals[4*3 + 2] = -1;

    camera->plane_r = r;
    camera->plane_l = l;
    camera->plane_t = t;
    camera->plane_b = b;
    camera->plane_n = n;
    camera->plane_f = f;
    camera->projection_matrix = frustrum_matrix;
}

