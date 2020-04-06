/*================================================================================
    Camera
================================================================================*/
#include "Engine.h"

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

    mat4x4 frustum_matrix = {0};
    //------
    // Actually derived formulation, assuming r = -l and t = -b.
    fill_mat4x4(frustum_matrix, 1/r, 0,   0,    0,
                                0,   1/t, 0,    0,
                                0,   0,   -1/n,  -1/n,
                                0,   0,   -1,  0);

    //---Incorrect formulation copied incorrectly.
    // frustum_matrix.vals[4*0 + 0] = (2*n*n)/(r-l);
    // frustum_matrix.vals[4*1 + 1] = (2*n*n)/(t-b);
    // frustum_matrix.vals[4*2 + 2] = -(f+n)/(f-n);
    // frustum_matrix.vals[4*0 + 2] = (r+l)/(r-l);
    // frustum_matrix.vals[4*1 + 2] = (t+b)/(t-b);
    // frustum_matrix.vals[4*2 + 3] = -(2*n*f)/(f-n);
    // frustum_matrix.vals[4*3 + 2] = -1;

    camera->plane_r = r;
    camera->plane_l = l;
    camera->plane_t = t;
    camera->plane_b = b;
    camera->plane_n = n;
    camera->plane_f = f;

    camera->aspect_ratio = aspect_ratio;
    // By default, the camera renders to the entire screen.
    camera->blx = 0;
    camera->bly = 0;
    camera->trx = 1;
    camera->try = 1;

    camera->projection_matrix = frustum_matrix;

    camera->override_bg_color = false;
    camera->bg_color = new_vec4(0,0,0,1);
}
mat4x4 Camera_vp_matrix(Camera *camera)
{
    mat4x4 vp_matrix = camera->projection_matrix;
    mat4x4 view_matrix = invert_rigid_mat4x4(Transform_matrix(get_sibling_aspect(camera, Transform)));
    right_multiply_matrix4x4f(&vp_matrix, &view_matrix);
    return vp_matrix;
}

// Bottom-left of camera rectangle is (0,0), top-right is (1,1).
// This method gives the origin and direction of a ray cast outward from the position of the camera,
// starting on the near plane.
void Camera_ray(Camera *camera, float x, float y, vec3 *origin, vec3 *direction)
{
    Transform *t = other_aspect(camera, Transform);
    mat4x4 matrix = Transform_matrix(t);
    vec3 position = Transform_position(t);

    vec3 camera_space_p = new_vec3(camera->plane_l + (camera->plane_r - camera->plane_l) * x,
                                   camera->plane_b + (camera->plane_t - camera->plane_b) * y,
                                   -camera->plane_n);
    // printf("camera_space_p: "); print_vec3(camera_space_p);
    *origin = mat4x4_vec3(&matrix, camera_space_p);
    *direction = vec3_sub(*origin, position);
}

