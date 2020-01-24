/*--------------------------------------------------------------------------------
notes:
    It might be fine not to use the resource system at all. Painting would be a very special
    purpose thing, with its own materials, which could be hardcoded. This would bypass the overhead
    of doing resource lookups, caching, etc.
--------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "matrix_mathematics.h"
#include "helper_definitions.h"
#include "data_dictionary.h"
#include "resources.h"
#include "rendering.h"
#include "painting.h"

/*--------------------------------------------------------------------------------
    Painting function calls buffer "paint", which is then drawn when the user wants with
    painting_draw(), and flushed with painting_flush(). (this separation is done so that paint can be rendered multiple times, e.g. with multiple cameras, before flushing.)
- implementation
    This is implemented by making "paint" be a piece of geometry and the material it will
    be rendered with (meaning that both of these resources must be destroyed when the buffer is flushed).
    Flushing goes through the buffer and renders each paint, then destroys it.
-
    This might better be implemented with painting information at a higher level, like colors and types of paint,
    which could then be sorted in the buffer while it is being built up, allowing for example the minimization of
    material switches.
--------------------------------------------------------------------------------*/
typedef struct Paint_s {
    Geometry geometry;
    ResourceHandle material; // Resource: Material
} Paint;
// Implemented now with a fixed size buffer. If this turns out to be a problem, something like a linked list of buffers might work, so
// that paint count doesn't matter, but the paint is still mostly contiguous.
// static const int g_painting_size = 4096;
#define g_painting_size 4096
static int g_painting_count = 0;
static Paint g_paint[g_painting_size];

static enum LineModes {
    LINE_FLAT,
    LINE_DASHED,
};

static bool painting_module_initialized = false;
#define DEBUG_LEVEL 1
#if DEBUG_LEVEL >= 1
#define init_check() {\
    if (!painting_module_initialized) {\
        fprintf(stderr, ERROR_ALERT "Painting module has not been initialized.\n");\
        exit(EXIT_FAILURE);\
    }\
}
#else
#define init_check() { }
#endif

// Initialize the painting module. This opens the global dictionary with painting resources.
void painting_init(void)
{
    resource_path_add("Painting", "/home/lucas/collision/lib/painting"); //---should have each library given access to a compile-time symbol for its directory.
    // Access resources for this module with drive "Painting".
    // note: this duplicates resources which could be shared, like simple shaders and material types, but this is fine.

    painting_module_initialized = true;
}
void painting_add(Geometry geometry, ResourceHandle material)
{
    if (g_painting_count >= g_painting_size) {
        fprintf(stderr, ERROR_ALERT "paint error: The maximum number of pieces of paint has been exceeded. Possibly the buffer has not been flushed, or there is too much paint.\n");
        exit(EXIT_FAILURE);
    }
    g_paint[g_painting_count].geometry = geometry;
    g_paint[g_painting_count].material = material;

    g_painting_count ++;
}
void painting_draw(void)
{
    for (int i = 0; i < g_painting_count; i++) {
        gm_draw(g_paint[i].geometry, resource_data(Material, g_paint[i].material));
    }
}
void painting_flush(void)
{
    for (int i = 0; i < g_painting_count; i++) {
        gm_free(g_paint[i].geometry);
        destroy_resource_handle(&g_paint[i].material);
    }
    g_painting_count = 0;
}



/*--------------------------------------------------------------------------------
    Colors
--------------------------------------------------------------------------------*/
vec4 str_to_color_key(char *color)
{
    #define col(STR1,STR2,R,G,B,A) if (strcmp(color, ( STR1 )) == 0 || strcmp(color, ( STR2 )) == 0) return new_vec4((R),(G),(B),(A));
    col("g", "green", 0,1,0,1);
    col("r", "red", 1,0,0,1);
    col("y", "yellow", 1,0,1,1);
    col("b", "blue", 0,0,1,1);
    col("k", "black", 0,0,0,1);
    col("w", "white", 1,1,1,1);
    col("gr", "gray", 0.4,0.4,0.4,1);
    fprintf(stderr, ERROR_ALERT "Unsupported color \"%s\".\n", color);
    exit(EXIT_FAILURE);
    #undef col
}
/*--------------------------------------------------------------------------------
    Painting
--------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------
Painting function variants:
*   : scalar parameters
*_v : vector parameters
*_c : colors given by string code
Variants can be combined.
--------------------------------------------------------------------------------*/
void paint_line_v(vec3 a, vec3 b, vec4 color)
{
    init_check();
    gm_lines(VERTEX_FORMAT_3);
    attribute_3f(Position, a.vals[0],a.vals[1],a.vals[2]);
    attribute_3f(Position, b.vals[0],b.vals[1],b.vals[2]);
    ResourceHandle mat = Material_create("Painting/Materials/flat_color");
    material_set_property_vec4(resource_data(Material, mat), "flat_color", color);
    painting_add(gm_done(), mat);
}
void paint_line(float ax, float ay, float az, float bx, float by, float bz, COLOR_SCALARS)
{
    paint_line_v(new_vec3(ax, ay, az), new_vec3(bx, by, bz), new_vec4(cr, cg, cb, ca));
}
void paint_line_c(float ax, float ay, float az, float bx, float by, float bz, char *color_str)
{
    paint_line_v(new_vec3(ax, ay, az), new_vec3(bx, by, bz), str_to_color_key(color_str));
}
void paint_line_cv(vec3 a, vec3 b, char *color_str)
{
    paint_line_v(a, b, str_to_color_key(color_str));
}

void paint_chain(float vals[], int num_points, COLOR_SCALARS) // vals length: num_points * 3
{
    gm_lines(VERTEX_FORMAT_3);
    for (int i = 0; i < num_points - 1; i++) {
        int j = i + 1;
        float x = vals[3*i + 0];
        float y = vals[3*i + 1];
        float z = vals[3*i + 2];
        attribute_3f(Position, x,y,z);
    }
    ResourceHandle mat = Material_create("Painting/Materials/flat_color");
    material_set_property_vec4(resource_data(Material, mat), "flat_color", new_vec4(cr,cg,cb,ca));
    painting_add(gm_done(), mat);
}
void paint_chain_c(float vals[], int num_points, char *color_str)
{
    vec4 color = str_to_color_key(color_str);
    paint_chain(vals, num_points, UNPACK_COLOR(color));
}

void paint_loop(float vals[], int num_points, COLOR_SCALARS) // vals length: num_points * 3
{
    gm_lines(VERTEX_FORMAT_3);
    for (int i = 0; i < num_points; i++) {
        int j = (i + 1) % num_points;
        float x = vals[3*i + 0];
        float y = vals[3*i + 1];
        float z = vals[3*i + 2];
        attribute_3f(Position, x,y,z);
    }
    ResourceHandle mat = Material_create("Painting/Materials/flat_color");
    material_set_property_vec4(resource_data(Material, mat), "flat_color", new_vec4(cr,cg,cb,ca));
    painting_add(gm_done(), mat);

}
void paint_loop_c(float vals[], int num_points, char *color_str)
{
    vec4 color = str_to_color_key(color_str);
    paint_loop(vals, num_points, UNPACK_COLOR(color));
}

void paint_quad(float p1x, float p1y, float p1z, 
                float p2x, float p2y, float p2z, 
                float p3x, float p3y, float p3z, 
                float p4x, float p4y, float p4z,
                COLOR_SCALARS)
{
    gm_triangles(VERTEX_FORMAT_3);
    attribute_3f(Position, p1x, p1y, p1z);
    attribute_3f(Position, p2x, p2y, p2z);
    attribute_3f(Position, p3x, p3y, p3z);
    attribute_3f(Position, p4x, p4y, p4z);
    gm_index(0); gm_index(1); gm_index(2);
    gm_index(0); gm_index(2); gm_index(3);
    ResourceHandle mat = Material_create("Painting/Materials/flat_color");
    material_set_property_vec4(resource_data(Material, mat), "flat_color", new_vec4(cr,cg,cb,ca));
    painting_add(gm_done(), mat);
} 
void paint_quad_v(vec3 p1, vec3 p2, vec3 p3, vec3 p4, vec4 color)
{
    paint_quad(p1.vals[0], p1.vals[1], p1.vals[2],
               p2.vals[0], p2.vals[1], p2.vals[2],
               p3.vals[0], p3.vals[1], p3.vals[2],
               p4.vals[0], p4.vals[1], p4.vals[2],
               UNPACK_COLOR(color));
}


// void paint_quad(float p1x, float p1y, float p1z, float nx, float ny, float nz, float ux, float uy, float uz, float w, float h)
// {
//     // Form u' by extracting the part of u in the direction of n, getting a vector orthogonal to n
//     // yet on the same plane formed by the span of u and n. Width will be extended along this vector.
//     float n_length_inv = 1/sqrt(nx*nx + ny*ny + nz*nz);
//     nx *= n_length_inv;
//     ny *= n_length_inv;
//     nz *= n_length_inv;
//     float d = nx*ux + ny*uy + nz*uz;
//     float u_length_inv = 1/sqrt(ux*ux + uy*uy + uz*uz);
//     float upx = (ux - nx*d)*u_length_inv;
//     float upy = (uy - ny*d)*u_length_inv;
//     float upz = (uz - nz*d)*u_length_inv;
//     // Measure the width across u' to get the second point on the quad.
//     float p2x = p1x + upx*w;
//     float p2y = p1y + upy*w;
//     float p2z = p1z + upz*w;
//     // Measure the height across cross(u', n) to get the fourth point on the quad.
//     //...
// 
//     // Add these differences from p1 to get the third point.
//     float p3x = p1x + (p2x - p1x) + (p4x - p1x);
//     float p3y = p1y + (p2y - p1y) + (p4y - p1y);
//     float p3z = p1z + (p2z - p1z) + (p4z - p1z);
//     
//     gm_triangles(VERTEX_FORMAT_3);
//     attribute_3f(Position, p1x, p1y, p1z);
//     attribute_3f(Position, p2x, p2y, p2z);
//     attribute_3f(Position, p3x, p3y, p3z);
//     attribute_3f(Position, p4x, p4y, p4z);
//     gm_index(0); gm_index(1); gm_index(2);
//     gm_index(0); gm_index(2); gm_index(3);
//     ResourceHandle mat = Material_create("Painting/Materials/flat_color");
//     material_set_property_vec4(resource_data(Material, mat), "flat_color", new_vec4(cr,cg,cb,ca));
//     painting_add(gm_done(), mat);
// }
