/*--------------------------------------------------------------------------------
notes:
    It might be fine not to use the resource system at all. Painting would be a very special
    purpose thing, with its own materials, which could be hardcoded. This would bypass the overhead
    of doing resource lookups, caching, etc.
--------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
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


