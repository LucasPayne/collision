/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"


float sigma, beta, rho;

#if 1
float sigma_base = 10;
float beta_base = 8.0/3.0;
float rho_base = 28;
#else
float sigma_base = 28;
float beta_base = 4;
float rho_base = 46.92;
#endif
// a = 10, b = 28, c = 8 / 3
// a = 28, b = 46.92, c = 4

#define X(VEC) ( ( VEC ).vals[0] )
#define Y(VEC) ( ( VEC ).vals[1] )
#define Z(VEC) ( ( VEC ).vals[2] )
vec3 gradient(vec3 p)
{
    return new_vec3(sigma * (Y(p) - X(p)),
                    X(p) * (rho - Z(p)) - Y(p),
                    X(p)*Y(p) - beta*Z(p));
}

vec3 heuns(vec3 (*f)(vec3), vec3 x0, float t_start, float t_end, float step_size, vec4 color)
{
    // Heun's method for systems of three first order autonomous ODEs.
    vec3 x = x0;
    vec3 aux_term, aux_x, new_term, new_x;
    for (float t = t_start; t <= t_end; t += step_size)
    {
        aux_term = f(x);
        aux_x = vec3_add(x, vec3_mul(aux_term, step_size));
        new_term = f(aux_x);
        new_x = vec3_add(x, vec3_mul(vec3_add(aux_term, new_term), 0.5 * step_size));
        float size = 3;
        vec3 p = vec3_mul(new_x, size);
        // paint_points_c(Canvas3D, &p, 1, "k", 1);
        paint_line_v(Canvas3D, vec3_mul(x, size), vec3_mul(new_x, size), color, 2);
        x = new_x;
    }
    return x;
}

extern void input_event(int key, int action, int mods)
{
}
extern void mouse_button_event(int button, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    create_key_camera_man(0,0,0,  0,0,0);
    
}
extern void loop_program(void)
{
    int grid_tess_x = 84;
    int grid_tess_y = 84;
    float width = 2000;
    float height = 2000;
    vec3 base[4] = {
        {{-width/2,-30,-height/2}},
        {{width/2,-30,-height/2}},
        {{width/2,-30,height/2}},
        {{-width/2,-30,height/2}},
    };
    //paint_sphere_cv(Canvas3D, vec3_zero(), 0.3, "k");
    //paint_line_cv(Canvas3D, vec3_zero(), new_vec3(width/2,0,0), "tr", 3);
    //paint_line_cv(Canvas3D, vec3_zero(), new_vec3(0,width/2,0), "tg", 3);
    //paint_line_cv(Canvas3D, vec3_zero(), new_vec3(0,0,height/2), "tb", 3);
    //paint_grid_v(Canvas3D, base[0], base[1], base[2], base[3], new_vec4(0,0.3,0.9,0.3), grid_tess_x, grid_tess_y, 1);
    paint_grid_v(Canvas3D, base[0], base[1], base[2], base[3], new_vec4(0.5,0.1,0.12,0.6), grid_tess_x, grid_tess_y, 1);
    
#if 1
    if (TEST_SWITCH) return;
    int n = 15;
    for (int i = 0; i < n; i++) {
        vec4 color;
        float perturb = 20;
        sigma = sigma_base;
        beta = beta_base;
        rho = rho_base;
        //sigma = sigma_base - perturb/2 + i*(perturb/n);
        beta = beta_base - perturb/2 + i*(perturb/n);
        //rho = rho_base - perturb/2 + i*(perturb/n);
        float r = 0.15 + 0.85 * i * 1.0 / n;
        //color = new_vec4(r,r,r,0.1);
        color = new_vec4(r,r,r,0.6);
        // sigma = sigma_base + perturb*cos(i * 2*M_PI/n);
        // rho = rho_base + perturb*sin(i * 2*M_PI/n);
        // beta = beta_base + perturb*sin(i * 2*M_PI/n);
        // color = new_vec4(2*cos(i*2*M_PI/n)-1,2*sin(i*2*M_PI/n)-1,0,0.3)
        heuns(gradient, new_vec3(1,1,1), 0, 100, 0.01, color);
    }
#else
    heuns(gradient, new_vec3(1,1,1), 0, 100, 0.01, new_vec4(1,1,1,1));
#endif
}
extern void close_program(void)
{
}
