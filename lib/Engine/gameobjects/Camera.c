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

    mat4x4 frustrum_matrix = {0};
    // Attempting derivation of projection matrix here.
    // frustrum_matrix.vals[4*0 + 0] = (far / near) * near_half_width;
    // frustrum_matrix.vals[4*1 + 1] = (far / near) * aspect_ratio * near_half_width;
    // frustrum_matrix.vals[4*2 + 3] = -far;
    // frustrum_matrix.vals[4*3 + 2] = -1;

    //frustrum_matrix.vals[4*0 + 0] = 1.0 / ((far / near) * near_half_width);
    //frustrum_matrix.vals[4*1 + 1] = 1.0 / ((far / near) * aspect_ratio * near_half_width);
    //frustrum_matrix.vals[4*2 + 3] = -1;
    //frustrum_matrix.vals[4*3 + 2] = 1 / far;
    

    frustrum_matrix.vals[4*0 + 0] = (2*n*n)/(r-l);
    frustrum_matrix.vals[4*1 + 1] = (2*n*n)/(t-b);
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

    camera->aspect_ratio = aspect_ratio;
    // By default, the camera renders to the entire screen.
    camera->blx = 0;
    camera->bly = 0;
    camera->trx = 1;
    camera->try = 1;

    camera->projection_matrix = frustrum_matrix;

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
