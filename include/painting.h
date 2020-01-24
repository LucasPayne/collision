/*================================================================================
================================================================================*/
#ifndef HEADER_DEFINED_PAINTING
#define HEADER_DEFINED_PAINTING
#include "matrix_mathematics.h"

// Helper in definitions (since there will be many).
#define COLOR_SCALARS float cr, float cg, float cb, float ca
#define UNPACK_COLOR(color) color.vals[0],color.vals[1],color.vals[2],color.vals[3]

void painting_init(void);
void painting_add(Geometry geometry, ResourceHandle material);
void painting_draw(void);
void painting_flush(void);

vec4 str_to_color_key(char *color);

void paint_line(float ax, float ay, float az, float bx, float by, float bz, COLOR_SCALARS);
void paint_line_c(float ax, float ay, float az, float bx, float by, float bz, char *color_str);
void paint_line_v(vec3 a, vec3 b, vec4 color);
void paint_line_cv(vec3 a, vec3 b, char *color_str);

void paint_chain(float vals[], int num_points, COLOR_SCALARS); // vals length: num_points * 3
void paint_chain_c(float vals[], int num_points, char *color_str);
void paint_loop(float vals[], int num_points, COLOR_SCALARS); // vals length: num_points * 3
void paint_loop_c(float vals[], int num_points, char *color_str);

void paint_quad(float p1x, float p1y, float p1z, 
                float p2x, float p2y, float p2z, 
                float p3x, float p3y, float p3z, 
                float p4x, float p4y, float p4z,
                COLOR_SCALARS);
void paint_quad_v(vec3 p1, vec3 p2, vec3 p3, vec3 p4, vec4 color);

#endif // HEADER_DEFINED_PAINTING
