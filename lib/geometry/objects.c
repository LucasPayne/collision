/*--------------------------------------------------------------------------------
    Geometric objects such as the platonic solids.
--------------------------------------------------------------------------------*/
#include "geometry.h"
#include "math.h"

Polyhedron make_icosahedron(float radius)
{
    float theta0 = 2.0*M_PI / 5.0;
    float theta1 = 4.0*M_PI / 5.0;
    float x = sqrt(2 - 2*cos(theta0));
    float r = 1.0 / x;
    
    float y = r * sqrt(1 - 2*cos(theta0));
    float z = 2*y - sqrt(1 - (sin(theta0) - sin(theta1))*(sin(theta0) - sin(theta1)) - (sin(theta0) - sin(theta1))*(sin(theta0) - sin(theta1)));

    vec3 points[12];
    points[0] = new_vec3(0,0,-1.5*y);//------this is not correct.
    points[1] = new_vec3(0,0,z+1.5*y);//------
    for (int i = 0; i < 5; i++) {
        float theta_a = 2.0*M_PI*i/5.0;
        float theta_b = 2.0*M_PI*(i+0.5)/5.0;
        points[2+2*i] = new_vec3(r*cos(theta_a), r*sin(theta_a), y);
        points[2+2*i+1] = new_vec3(r*cos(theta_b), r*sin(theta_b), z - y);
    }
    for (int i = 0; i < 12; i++) points[i] = vec3_mul(vec3_sub(points[i], new_vec3(0,0,z/2)), radius);
    return convex_hull(points, 12);
}
