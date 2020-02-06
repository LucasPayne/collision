/*--------------------------------------------------------------------------------
notes
-----
It might be fine to use the resource system minimally. Painting would be a very special
purpose thing, with its own materials, which could be hardcoded. This would bypass the overhead
of doing resource lookups, caching, etc.

There are two buffers, one for 3D paint and one for 2D paint. Currently there is no facility
for further separate buffers or "canvases".
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


// Matrix used for 2d painting.
// x: < 0-1 >
// y: v 0-1 ^
mat4x4 matrix_paint2d(void)
{
    static mat4x4 matrix = {{
        2,   0,  0,  0,
        0,   2,  0,  0,
        0,   0,  1,  0,
        -1, -1,  0,  1,
    }};
    return matrix;
}


typedef struct Paint_s {
    Geometry geometry;
    ResourceHandle material; // Resource: Material
} Paint;

// right now, canvases have a fixed amount of paint.
#define g_paint_buffer_size 4096
typedef struct Canvas_s {
    int paint_count;
    int paint_buffer_size;
    Paint paint_buffer[g_paint_buffer_size]; // this could later be a dynamic array.
} Canvas;

static const float paint2d_depth = 0.0; // The standard 2D wrapper functions fill in z values with this depth.
static Canvas g_canvases[NUM_CANVASES]; // Currently, there is a fixed set of canvases.
                                        // There chould be standard canvases with their own function calls,
                                        // which call the canvas-selecting functions.
static Canvas *painting_canvas(int canvas_id)
{
    return &g_canvases[canvas_id];
}

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
    memset(g_canvases, 0, sizeof(g_canvases));
    for (int i = 0; i < NUM_CANVASES; i++) { // initialize the canvases.
        g_canvases[i].paint_buffer_size = g_paint_buffer_size;
    }
}
static void painting_add(int canvas_id, Geometry geometry, ResourceHandle material)
{
    Canvas *canvas = painting_canvas(canvas_id);
    if (canvas->paint_count >= canvas->paint_buffer_size) {
        fprintf(stderr, ERROR_ALERT "paint error: The paint buffer of a canvas has been exceeded. paint_count: %d, paint_buffer_size: %d.\n", canvas->paint_count, canvas->paint_buffer_size);
        exit(EXIT_FAILURE);
    }
    canvas->paint_buffer[canvas->paint_count].geometry = geometry;
    canvas->paint_buffer[canvas->paint_count].material = material;
    canvas->paint_count ++;
}
void painting_draw(int canvas_id)
{
    Canvas *canvas = painting_canvas(canvas_id);
    for (int i = 0; i < canvas->paint_count; i++) {
        gm_draw(canvas->paint_buffer[i].geometry, resource_data(Material, canvas->paint_buffer[i].material));
    }
}
void painting_flush(int canvas_id)
{
    Canvas *canvas = painting_canvas(canvas_id);
    for (int i = 0; i < canvas->paint_count; i++) {
        gm_free(canvas->paint_buffer[i].geometry);
        // destroy_resource_handle(&canvas->paint_buffer[i].material); ////////////////////////////////////////////////////////////////////////////////...
    }
    canvas->paint_count = 0;
}


/*--------------------------------------------------------------------------------
    Colors
--------------------------------------------------------------------------------*/
vec4 str_to_color_key(char *color)
{
    #define col(STR1,STR2,R,G,B,A) if (strcmp(color, ( STR1 )) == 0 || strcmp(color, ( STR2 )) == 0) return new_vec4((R),(G),(B),(A));
    col("g", "green", 0,1,0,1);
    col("r", "red", 1,0,0,1);
    col("y", "yellow", 1,1,0,1);
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
/*--------------------------------------------------------------------------------
    Line painting
    note: no paint2D vector variants.
--------------------------------------------------------------------------------*/
// Generic canvas line painting
// ----------------------------
void canvas_paint_line_v(int canvas, vec3 a, vec3 b, vec4 color)
{
    gm_lines(VERTEX_FORMAT_3);
    attribute_3f(Position, a.vals[0],a.vals[1],a.vals[2]);
    attribute_3f(Position, b.vals[0],b.vals[1],b.vals[2]);
    ResourceHandle mat = Material_create("Painting/Materials/flat_color");
    material_set_property_vec4(resource_data(Material, mat), "flat_color", color);
    painting_add(canvas, gm_done(), mat);
}
void canvas_paint_line(int canvas, float ax, float ay, float az, float bx, float by, float bz, COLOR_SCALARS) {
    canvas_paint_line_v(canvas, new_vec3(ax, ay, az), new_vec3(bx, by, bz), new_vec4(cr, cg, cb, ca));
}
void canvas_paint_line_c(int canvas, float ax, float ay, float az, float bx, float by, float bz, char *color_str) {
    canvas_paint_line_v(canvas, new_vec3(ax, ay, az), new_vec3(bx, by, bz), str_to_color_key(color_str));
}
void canvas_paint_line_cv(int canvas, vec3 a, vec3 b, char *color_str) {
    canvas_paint_line_v(canvas, a, b, str_to_color_key(color_str));
}
// Standard 3D canvas line painting
// --------------------------------
void paint_line(float ax, float ay, float az, float bx, float by, float bz, COLOR_SCALARS) {
    canvas_paint_line_v(Canvas3D, new_vec3(ax, ay, az), new_vec3(bx, by, bz), new_vec4(cr, cg, cb, ca));
}
void paint_line_c(float ax, float ay, float az, float bx, float by, float bz, char *color_str) {
    canvas_paint_line_v(Canvas3D, new_vec3(ax, ay, az), new_vec3(bx, by, bz), str_to_color_key(color_str));
}
void paint_line_v(vec3 a, vec3 b, vec4 color) {
    canvas_paint_line_v(Canvas3D, a, b, color);
}
void paint_line_cv(vec3 a, vec3 b, char *color_str) {
    canvas_paint_line_v(Canvas3D, a, b, str_to_color_key(color_str));
}

// Standard 2D canvas line painting
// --------------------------------
void paint2d_line(float ax, float ay, float bx, float by, COLOR_SCALARS) {
    canvas_paint_line_v(Canvas2D, new_vec3(ax, ay, paint2d_depth), new_vec3(bx, by, paint2d_depth), new_vec4(cr, cg, cb, ca));
}
void paint2d_line_c(float ax, float ay, float bx, float by, char *color_str) {
    canvas_paint_line_v(Canvas2D, new_vec3(ax, ay, paint2d_depth), new_vec3(bx, by, paint2d_depth), str_to_color_key(color_str));
}

/*--------------------------------------------------------------------------------
    Chain painting
    (a chain of connected line segments).
--------------------------------------------------------------------------------*/
// Generic canvas chain painting
// -----------------------------
void canvas_paint_chain(int canvas, float vals[], int num_points, COLOR_SCALARS) // vals length: num_points * 3
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
    painting_add(canvas, gm_done(), mat);
}
void canvas_paint_chain_c(int canvas, float vals[], int num_points, char *color_str)
{
    vec4 color = str_to_color_key(color_str);
    canvas_paint_chain(canvas, vals, num_points, UNPACK_COLOR(color));
}
// Standard 3D canvas chain painting
// ---------------------------------
void paint_chain(float vals[], int num_points, COLOR_SCALARS) {
    canvas_paint_chain(Canvas3D, vals, num_points, cr,cg,cb,ca);
}
void paint_chain_c(float vals[], int num_points, char *color_str) {
    canvas_paint_chain_c(Canvas3D, vals, num_points, color_str);
}
// Standard 2D canvas chain painting
// ---------------------------------
void paint2d_chain(float vals[], int num_points, COLOR_SCALARS) // vals length: num_points * 2
{
    // Fill in the z values to pass to the 3D chain painter.
    float vals3d[num_points * 3];
    for (int i = 0; i < num_points; i++) {
        vals3d[3*i + 0] = vals[2*i + 0];
        vals3d[3*i + 1] = vals[2*i + 1];
        vals3d[3*i + 2] = paint2d_depth;
    }
    canvas_paint_chain(Canvas2D, vals3d, num_points, cr,cg,cb,ca);
}
void paint2d_chain_c(float vals[], int num_points, char *color_str) {
    vec4 color = str_to_color_key(color_str);
    paint2d_chain(vals, num_points, UNPACK_COLOR(color));
}
/*--------------------------------------------------------------------------------
    Loop painting
    (same as chain except the opposite ends are connected by a line segment.)
--------------------------------------------------------------------------------*/
// Generic canvas loop painting
// ----------------------------
void canvas_paint_loop(int canvas, float vals[], int num_points, COLOR_SCALARS) // vals length: num_points * 3
{
    gm_lines(VERTEX_FORMAT_3);
    for (int i = 0; i < num_points + 1; i++) {
        int j = (i + 1) % num_points;
        float x = vals[3*j + 0];
        float y = vals[3*j + 1];
        float z = vals[3*j + 2];
        attribute_3f(Position, x,y,z);
    }
    ResourceHandle mat = Material_create("Painting/Materials/flat_color");
    material_set_property_vec4(resource_data(Material, mat), "flat_color", new_vec4(cr,cg,cb,ca));
    painting_add(canvas, gm_done(), mat);
}
void canvas_paint_loop_c(int canvas, float vals[], int num_points, char *color_str) {
    vec4 color = str_to_color_key(color_str);
    canvas_paint_loop(canvas, vals, num_points, UNPACK_COLOR(color));
}
void canvas_paint_loop_v(int canvas, float vals[], int num_points, vec4 color) {
    canvas_paint_loop(canvas, vals, num_points, UNPACK_COLOR(color));
}
// Standard 3D canvas loop painting
// --------------------------------
void paint_loop(float vals[], int num_points, COLOR_SCALARS) {
    canvas_paint_loop(Canvas3D, vals, num_points, cr,cg,cb,ca);
}
void paint_loop_c(float vals[], int num_points, char *color_str) {
    canvas_paint_loop_c(Canvas3D, vals, num_points, color_str);
}
void paint_loop_v(float vals[], int num_points, vec4 color) {
    canvas_paint_loop_v(Canvas3D, vals, num_points, color);
}
// Standard 2D canvas loop painting
// --------------------------------
void paint2d_loop(float vals[], int num_points, COLOR_SCALARS) {
    // Fill in the z values to pass to the 3D loop painter.
    float vals3d[num_points * 3];
    for (int i = 0; i < num_points; i++) {
        vals3d[3*i + 0] = vals[2*i + 0];
        vals3d[3*i + 1] = vals[2*i + 1];
        vals3d[3*i + 2] = paint2d_depth;
    }
    canvas_paint_loop(Canvas2D, vals3d, num_points, cr,cg,cb,ca);
}
void paint2d_loop_c(float vals[], int num_points, char *color_str) {
    vec4 color = str_to_color_key(color_str);
    paint2d_loop(vals, num_points, UNPACK_COLOR(color));
}

/*--------------------------------------------------------------------------------
    Quad painting
    These are given by the four corners,
    instead of some encoding guaranteeing a correct quad.
--------------------------------------------------------------------------------*/
// Generic canvas quad painting
// ----------------------------
void canvas_paint_quad_vm(int canvas, vec3 p1, vec3 p2, vec3 p3, vec3 p4, ResourceHandle material_handle)
{
    gm_triangles(VERTEX_FORMAT_3);
    attribute_3f(Position, p1.vals[0], p1.vals[1], p1.vals[2]);
    attribute_3f(Position, p2.vals[0], p2.vals[1], p2.vals[2]);
    attribute_3f(Position, p3.vals[0], p3.vals[1], p3.vals[2]);
    attribute_3f(Position, p4.vals[0], p4.vals[1], p4.vals[2]);
    gm_index(0); gm_index(1); gm_index(2);
    gm_index(0); gm_index(2); gm_index(3);
    painting_add(canvas, gm_done(), material_handle);
}
void canvas_paint_quad(int canvas,
                       float p1x, float p1y, float p1z, 
                       float p2x, float p2y, float p2z, 
                       float p3x, float p3y, float p3z, 
                       float p4x, float p4y, float p4z,
                       COLOR_SCALARS)
{
    ResourceHandle mat = Material_create("Painting/Materials/flat_color");
    material_set_property_vec4(resource_data(Material, mat), "flat_color", new_vec4(cr,cg,cb,ca));
    canvas_paint_quad_vm(canvas, new_vec3(p1x,p1y,p1z), new_vec3(p2x,p2y,p2z), new_vec3(p3x,p3y,p3z), new_vec3(p4x,p4y,p4z), mat);
} 
void canvas_paint_quad_v(int canvas, vec3 p1, vec3 p2, vec3 p3, vec3 p4, vec4 color)
{
    canvas_paint_quad(canvas,
               p1.vals[0], p1.vals[1], p1.vals[2],
               p2.vals[0], p2.vals[1], p2.vals[2],
               p3.vals[0], p3.vals[1], p3.vals[2],
               p4.vals[0], p4.vals[1], p4.vals[2],
               UNPACK_COLOR(color));
}
void canvas_paint_quad_c(int canvas,
                         float p1x, float p1y, float p1z, 
                         float p2x, float p2y, float p2z, 
                         float p3x, float p3y, float p3z, 
                         float p4x, float p4y, float p4z,
                         char *color_str)
{
    vec4 color = str_to_color_key(color_str);
    canvas_paint_quad(canvas, p1x,p1y,p1z, p2x,p2y,p2z, p3x,p3y,p3z, p4x,p4y,p4z, UNPACK_COLOR(color));
}

// Standard 3D canvas quad painting
// --------------------------------
void paint_quad(float p1x, float p1y, float p1z, 
                float p2x, float p2y, float p2z, 
                float p3x, float p3y, float p3z, 
                float p4x, float p4y, float p4z,
                COLOR_SCALARS)
{
    canvas_paint_quad(Canvas3D, p1x,p1y,p1z, p2x,p2y,p2z, p3x,p3y,p3z, p4x,p4y,p4z, cr,cg,cb,ca);
}
void paint_quad_v(vec3 p1, vec3 p2, vec3 p3, vec3 p4, vec4 color)
{
    canvas_paint_quad(Canvas3D,
               p1.vals[0], p1.vals[1], p1.vals[2],
               p2.vals[0], p2.vals[1], p2.vals[2],
               p3.vals[0], p3.vals[1], p3.vals[2],
               p4.vals[0], p4.vals[1], p4.vals[2],
               UNPACK_COLOR(color));
}
void paint_quad_c(float p1x, float p1y, float p1z, 
                float p2x, float p2y, float p2z, 
                float p3x, float p3y, float p3z, 
                float p4x, float p4y, float p4z,
                char *color_str)
{
    vec4 color = str_to_color_key(color_str);
    paint_quad(p1x,p1y,p1z, p2x,p2y,p2z, p3x,p3y,p3z, p4x,p4y,p4z, UNPACK_COLOR(color));
}
void paint_quad_cv(vec3 p1, vec3 p2, vec3 p3, vec3 p4, char *color_str)
{
    paint_quad_v(p1, p2, p3, p4, str_to_color_key(color_str));
}

void paint_quad_vm(vec3 p1, vec3 p2, vec3 p3, vec3 p4, ResourceHandle material_handle)
{
    canvas_paint_quad_vm(Canvas3D, p1, p2, p3, p4, material_handle);
}


// Standard 2D canvas quad painting
// -------------------------------
void paint2d_quad(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y, float p4x, float p4y, COLOR_SCALARS) {
    canvas_paint_quad(Canvas2D,  p1x,p1y,paint2d_depth,  p2x,p2y,paint2d_depth,  p3x,p3y,paint2d_depth,  p4x,p4y,paint2d_depth,  cr,cg,cb,ca);
}
void paint2d_quad_c(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y, float p4x, float p4y, char *color_str) {
    canvas_paint_quad_c(Canvas2D,  p1x,p1y,paint2d_depth,  p2x,p2y,paint2d_depth,  p3x,p3y,paint2d_depth,  p4x,p4y,paint2d_depth,  color_str);
}
// Standard 2D canvas rectangle printing. This is more convenient formulated in 2D, so these functions are specific to 2D.
void paint2d_rect(float x, float y, float width, float height, COLOR_SCALARS) {
    paint2d_quad(x,y,  x+width,y,  x+width,y+height,  x,y+height,  cr,cg,cb,ca);
}
void paint2d_rect_c(float x, float y, float width, float height, char *color_str) {
    paint2d_quad_c(x,y,  x+width,y,  x+width,y+height,  x,y+height,  color_str);
}

// Standard 2D canvas triangle painting
// ------------------------------------
void paint2d_triangle_m(float ax, float ay, float bx, float by, float cx, float cy, ResourceHandle material_handle)
{
    gm_triangles(VERTEX_FORMAT_3);
    attribute_3f(Position, ax, ay, paint2d_depth);
    attribute_3f(Position, bx, by, paint2d_depth);
    attribute_3f(Position, cx, cy, paint2d_depth);
    gm_index(0); gm_index(1); gm_index(2);
    painting_add(Canvas2D, gm_done(), material_handle);
}

// Standard 2D canvas sprite painting
// ----------------------------------
// Variants:
// p: The texture is from a path
// h: Horizontal flip
// v: vertical flip

static void _paint2d_sprite_m(float blx, float bly, float width, float height, ResourceHandle material_handle, bool horiz_flip, int rotate)
{
    // horiz_flip and rotate generate D8, so variant functions can be based off of this.

    // Render the sprite using a custom material.
    gm_triangles(VERTEX_FORMAT_3U);
    attribute_3f(Position, blx, bly, paint2d_depth);
    attribute_3f(Position, blx+width, bly, paint2d_depth);
    attribute_3f(Position, blx+width, bly+height, paint2d_depth);
    attribute_3f(Position, blx, bly+height, paint2d_depth);

    // remember, flip * rotate = rotate^-1 * flip
    static float coords[2 * 8] = {
        0, 1,
        1, 1,
        1, 0,
        0, 0,
    };
    for (int i = 0; i < 4; i++) {
        int index = ((horiz_flip ? -i : i) + rotate) % 4;
        attribute_2f(TexCoord, coords[2*index], coords[2*index + 1]);
    }
    gm_index(0); gm_index(1); gm_index(2);
    gm_index(0); gm_index(2); gm_index(3);
    painting_add(Canvas2D, gm_done(), material_handle);
}
void paint2d_sprite_m(float blx, float bly, float width, float height, ResourceHandle material_handle)
{
    _paint2d_sprite_m(blx, bly, width, height, material_handle, false, 0);
}
void paint2d_sprite_mv(float blx, float bly, float width, float height, ResourceHandle material_handle)
{
    _paint2d_sprite_m(blx, bly, width, height, material_handle, true, 2);
}
void paint2d_sprite_mh(float blx, float bly, float width, float height, ResourceHandle material_handle)
{
    _paint2d_sprite_m(blx, bly, width, height, material_handle, true, 0);
}

void paint2d_sprite(float blx, float bly, float width, float height, ResourceHandle texture_handle)
{
    // Render a regular sprite with a single texture.
    ResourceHandle mat = Material_create("Painting/Materials/sprite");
    material_set_texture(resource_data(Material, mat), "sprite", texture_handle);
    paint2d_sprite_m(blx, bly, width, height, mat);
}
void paint2d_sprite_p(float blx, float bly, float width, float height, char *texture_path)
{
    // Render a regular sprite with a single texture, the texture being given by its resource path.
    paint2d_sprite(blx, bly, width, height, new_resource_handle(Texture, texture_path));
}
