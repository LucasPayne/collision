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
#include <stdint.h>
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

Material *flat_color_material;
GLint flat_color_material_uniform_location_flat_color;
Material *sprite_material;
Material *circle_material;
GLint circle_material_uniform_location_flat_color;
Material *sphere_material;
GLint sphere_material_uniform_location_tessellation_level;
GLint sphere_material_uniform_location_radius;
GLint sphere_material_uniform_location_flat_color;

Material *line_material;
GLint line_material_uniform_location_line_width;
GLint line_material_uniform_location_flat_color;

static mat4x4 g_painting_matrix; // The painting matrix multiplies acts on all generated positions.
static bool g_using_painting_matrix = false; // If this is true, the painting matrix is used to modify positions.

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


static const float paint2d_depth = 0.0; // The standard 2D wrapper functions fill in z values with this depth.
static Canvas g_canvases[NUM_CANVASES]; // Currently, there is a fixed set of canvases.
                                        // There chould be standard canvases with their own function calls,
                                        // which call the canvas-selecting functions.

#define painting_canvas(CANVAS_ID) ( &g_canvases[( CANVAS_ID )] )


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

    // Canvases are currently just selected globally, but there could be facilities for creating new ones without modifying painting functions.
    for (int i = 0; i < NUM_CANVASES; i++) { // initialize the canvases.
        g_canvases[i].paint_buffer_size = g_paint_buffer_size;
        glGenBuffers(1, &g_canvases[i].vbo);
        glGenVertexArrays(1, &g_canvases[i].position_vao);
        glBindVertexArray(g_canvases[i].position_vao);
        glBindBuffer(GL_ARRAY_BUFFER, g_canvases[i].vbo);
        glVertexAttribPointer(Position, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
        glEnableVertexAttribArray(Position);
    }
    // Cache the standard painting materials.
    ResourceHandle flat_color_material_handle = Material_create("Painting/Materials/flat_color");
    ResourceHandle sprite_material_handle = Material_create("Painting/Materials/sprite");
    ResourceHandle circle_material_handle = Material_create("Painting/Materials/circle");
    ResourceHandle sphere_material_handle = Material_create("Painting/Materials/sphere");
    ResourceHandle line_material_handle = Material_create("Painting/Materials/line");
    // Currently, the material properties system has too much overhead for setting and uploading properties, so this is accessed more directly.
    flat_color_material = resource_data(Material, flat_color_material_handle);
    MaterialType *mt = resource_data(MaterialType, flat_color_material->material_type);
    flat_color_material_uniform_location_flat_color = glGetUniformLocation(mt->program_id, "flat_color");

    // sprite_material = resource_data(Material, sprite_material_handle);

    circle_material = resource_data(Material, circle_material_handle);
    mt = resource_data(MaterialType, circle_material->material_type);
    circle_material_uniform_location_flat_color = glGetUniformLocation(mt->program_id, "flat_color");

    sphere_material = resource_data(Material, sphere_material_handle);
    mt = resource_data(MaterialType, sphere_material->material_type);
    sphere_material_uniform_location_tessellation_level = glGetUniformLocation(mt->program_id, "tessellation_level");
    sphere_material_uniform_location_radius = glGetUniformLocation(mt->program_id, "radius");
    sphere_material_uniform_location_flat_color = glGetUniformLocation(mt->program_id, "flat_color");

    // Line material.
    line_material = resource_data(Material, line_material_handle);
    mt = resource_data(MaterialType, line_material->material_type);
    line_material_uniform_location_line_width = glGetUniformLocation(mt->program_id, "line_width");
    line_material_uniform_location_flat_color = glGetUniformLocation(mt->program_id, "flat_color");
}

#define paint_buffer_check(CANVAS)\
    if (( CANVAS )->paint_count >= ( CANVAS )->paint_buffer_size) {\
        fprintf(stderr, ERROR_ALERT "paint error: The paint buffer of a canvas has been exceeded. paint_count: %d, paint_buffer_size: %d.\n", ( CANVAS )->paint_count, ( CANVAS )->paint_buffer_size);\
        exit(EXIT_FAILURE);\
    }


void painting_draw(int canvas_id)
{
    static int last_num_vertices = 0; // The buffer is invalidated calculating the size from this.
    Canvas *canvas = painting_canvas(canvas_id);

    glBindBuffer(GL_ARRAY_BUFFER, canvas->vbo);
    glBufferData(GL_ARRAY_BUFFER, last_num_vertices, NULL, GL_STREAM_DRAW);
    glBufferData(GL_ARRAY_BUFFER, canvas->current_index * sizeof(float) * 3, canvas->vertex_buffer, GL_STREAM_DRAW);
    last_num_vertices = canvas->current_index;
    // printf("%d\n", canvas->current_index);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw paint with Position-only vertex format.
    glBindVertexArray(canvas->position_vao);
    for (int i = 0; i < canvas->paint_count; i++) {
        Paint *paint = &canvas->paint_buffer[i];
        switch (paint->type) {
            case PAINT_FLAT_LINES:
#if 0
                material_prepare(flat_color_material);
                glUniform4f(flat_color_material_uniform_location_flat_color, UNPACK_COLOR(paint->contents.flat.color));
                glLineWidth(10);
                glDrawArrays(GL_LINES, paint->index, 2*(paint->shape.line.num_points - 1));
#else
                material_prepare(line_material);
                glUniform4f(line_material_uniform_location_flat_color, UNPACK_COLOR(paint->contents.flat.color));
                glUniform1f(line_material_uniform_location_line_width, paint->shape.line.width);
                glPatchParameteri(GL_PATCH_VERTICES, 2);
                glDrawArrays(GL_PATCHES, paint->index, 2*(paint->shape.line.num_points - 1)); //----Does this need to be multiplied by the number of vertices in a patch?
#endif
                break;
            case PAINT_FLAT_TRIANGLES:
                material_prepare(flat_color_material);
                glUniform4f(flat_color_material_uniform_location_flat_color, UNPACK_COLOR(paint->contents.flat.color));
                glDrawArrays(GL_TRIANGLES, paint->index, paint->shape.triangle.num_triangles * 3);
                break;
            case PAINT_FLAT_POINTS:
                material_prepare(circle_material);
                glUniform4f(circle_material_uniform_location_flat_color, UNPACK_COLOR(paint->contents.flat.color));
                glPointSize(paint->shape.point.size);
                glDrawArrays(GL_POINTS, paint->index, paint->shape.point.num_points);
                break;
            case PAINT_FLAT_SPHERES:
                material_prepare(sphere_material);
                glUniform4f(sphere_material_uniform_location_flat_color, UNPACK_COLOR(paint->contents.flat.color));
                glUniform1f(sphere_material_uniform_location_radius, paint->shape.sphere.radius);
                glUniform1f(sphere_material_uniform_location_tessellation_level, 50); //---
                glPatchParameteri(GL_PATCH_VERTICES, 1);
                glDrawArrays(GL_PATCHES, paint->index, paint->shape.sphere.num_spheres);
                break;
            case PAINT_DASHED_LINES:
                fprintf(stderr, "not implemented\n");
                exit(EXIT_FAILURE);
                break;
            // case PAINT_SPRITE:
            //     material_set_texture(sprite_material, "diffuse_map", paint->contents.sprite.texture);
            //     break;
            // case PAINT_CUSTOM_MATERIAL:
            //     break;
            default:
                fprintf(stderr, "Invalid paint type given.\n");
                exit(EXIT_FAILURE);
        }
    }
}
void painting_flush(int canvas_id)
{
    Canvas *canvas = painting_canvas(canvas_id);
    for (int i = 0; i < canvas->paint_count; i++) {
        Paint *paint = &canvas->paint_buffer[i];
        if (paint->type == PAINT_SPRITE && paint->contents.sprite.destroy) {
            destroy_resource_handle(&paint->contents.sprite.texture);
        }
        else if (paint->type == PAINT_CUSTOM_MATERIAL && paint->contents.custom_material.destroy) {
            destroy_resource_handle(&paint->contents.custom_material.material);
        }
    }
    canvas->paint_count = 0;
    canvas->current_index = 0;
}


/*--------------------------------------------------------------------------------
    Colors
--------------------------------------------------------------------------------*/
vec4 str_to_color_key(char *color)
{
    // transparent variant: t{g,} or transparent_{green,}.
    #define col(STR1,STR2,R,G,B,A)if (strcmp(color, ( STR1 )) == 0 || strcmp(color, ( STR2 )) == 0) return new_vec4((R),(G),(B),(A));\
                                  if (strcmp(color, ( "t" STR1 )) == 0 || strcmp(color, ( "transparent_" STR2 )) == 0) return new_vec4((R),(G),(B),(A) / 2.0);
    col("g", "green", 0,1,0,1); 
    col("r", "red", 1,0,0,1);
    col("y", "yellow", 1,1,0,1);
    col("b", "blue", 0,0,1,1);
    col("p", "pink", 0.95,0.7,0.85,1);
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

/*--------------------------------------------------------------------------------
    strokes_* functions are used by paint_* functions. These returns a pointer to the basic skeleton
    of the Paint struct, and put the vertices in the canvas's vertex buffer.
--------------------------------------------------------------------------------*/

// remember that this increments the paint count as well.
#define next_paint(CANVAS_POINTER) ( &( CANVAS_POINTER )->paint_buffer[( CANVAS_POINTER )->paint_count ++] )
#define index_buffer(CANVAS_POINTER) (((vec3 *) &canvas->vertex_buffer) + ( CANVAS_POINTER )->current_index)

static Paint *strokes_points(Canvas *canvas, vec3 *points, int num_points, float size)
{
    Paint *paint = next_paint(canvas);
    size_t vals_size = sizeof(float) * 3 * num_points;
    if (g_using_painting_matrix) {
        for (int i = 0; i < num_points; i++) {
            ((vec3 *) index_buffer(canvas))[i] = mat4x4_vec3(g_painting_matrix, points[i]);
        }
    } else {
        memcpy(index_buffer(canvas), points, vals_size);
    }
    paint->index = canvas->current_index;
    canvas->current_index += num_points;
    paint->shape.point.size = size;
    paint->shape.point.num_points = num_points;
    return paint;
}

static Paint *strokes_line(Canvas *canvas, vec3 a, vec3 b, float width)
{
    Paint *paint = next_paint(canvas);
    vec3 *vp = (vec3 *) index_buffer(canvas);
    if (g_using_painting_matrix) {
        vp[0] = mat4x4_vec3(g_painting_matrix, a);
        vp[1] = mat4x4_vec3(g_painting_matrix, b);
    } else {
        vp[0] = a;
        vp[1] = b;
    }
    paint->index = canvas->current_index;
    canvas->current_index += 2;
    paint->shape.line.width = 0.005 * width; //---
    paint->shape.line.num_points = 2;
    return paint;
}
#if 0
//---To use this, canvases may need to have separate vertex arrays for different vertex formats.
static Paint *strokes_sprite(Canvas *canvas, float blx, float bly, float width, float height, bool horiz_flip, int rotate)
{
    // 3U: 2D rectangle with UV coordinates.
    Paint *paint = next_paint(canvas);
    float vertices[3 * 4 + 2 * 4] = {
        // Position
        blx, bly, paint2d_depth,
        blx + width, bly, paint2d_depth,
        blx + width, bly + height, paint2d_depth,
        blx, bly + height, paint2d_depth,
        // TexCoord
        0, 1,
        1, 1,
        1, 0,
        0, 0,
    };
    memcpy(index_buffer(canvas), vertices, sizeof(vertices));
    paint->index = canvas->current_index;
    canvas->current_index += 
    return paint;
}
#endif
static Paint *strokes_chain(Canvas *canvas, float vals[], int num_points, float width)
{
    Paint *paint = next_paint(canvas);
    size_t vals_size = sizeof(float) * 3 * num_points;
    if (g_using_painting_matrix) {
        vec3 *points = (vec3 *) vals;
        for (int i = 0; i < num_points; i++) {
            ((vec3 *) index_buffer(canvas))[i] = mat4x4_vec3(g_painting_matrix, points[i]);
        }
    } else {
        memcpy(index_buffer(canvas), vals, vals_size);
    }
    paint->index = canvas->current_index;
    canvas->current_index += num_points;
    paint->shape.line.width = width;
    paint->shape.line.num_points = num_points;
    return paint;
}
static Paint *strokes_loop(Canvas *canvas, float vals[], int num_points, float width)
{
    //-Can't create a loop of length 0.
    Paint *paint = next_paint(canvas);
    size_t vals_size = sizeof(float) * 3 * num_points;

    if (g_using_painting_matrix) {
        vec3 *points = (vec3 *) vals;
        for (int i = 0; i < num_points; i++) {
            ((vec3 *) index_buffer(canvas))[i] = mat4x4_vec3(g_painting_matrix, points[i]);
        }
        ((vec3 *) index_buffer(canvas))[num_points] = mat4x4_vec3(g_painting_matrix, points[0]);
    } else {
        memcpy(index_buffer(canvas), vals, vals_size);
        memcpy(index_buffer(canvas) + vals_size, &vals[0], sizeof(float) * 3);
    }
    paint->index = canvas->current_index;
    canvas->current_index += num_points + 1;
    paint->shape.line.width = width;
    paint->shape.line.num_points = num_points + 1;
    return paint;
}
static Paint *strokes_quad(Canvas *canvas, vec3 a, vec3 b, vec3 c, vec3 d)
{
    Paint *paint = next_paint(canvas);
    vec3 *vp = (vec3 *) index_buffer(canvas);
    if (g_using_painting_matrix) {
        vp[0] = mat4x4_vec3(g_painting_matrix, a);
        vp[1] = mat4x4_vec3(g_painting_matrix, b);
        vp[2] = mat4x4_vec3(g_painting_matrix, c);
        vp[3] = mat4x4_vec3(g_painting_matrix, a);
        vp[4] = mat4x4_vec3(g_painting_matrix, c);
        vp[5] = mat4x4_vec3(g_painting_matrix, d);
    } else {
        vp[0] = a;
        vp[1] = b;
        vp[2] = c;
        vp[3] = a;
        vp[4] = c;
        vp[5] = d;
    }
    paint->index = canvas->current_index;
    canvas->current_index += 6;
    paint->shape.triangle.num_triangles = 2;
    return paint;
}
static Paint *strokes_triangle(Canvas *canvas, vec3 a, vec3 b, vec3 c)
{
    Paint *paint = next_paint(canvas);
    vec3 *vp = (vec3 *) index_buffer(canvas);
    if (g_using_painting_matrix) {
        vp[0] = mat4x4_vec3(g_painting_matrix, a);
        vp[1] = mat4x4_vec3(g_painting_matrix, b);
        vp[2] = mat4x4_vec3(g_painting_matrix, c);
    } else {
        vp[0] = a;
        vp[1] = b;
        vp[2] = c;
    }
    paint->index = canvas->current_index;
    canvas->current_index += 3;
    paint->shape.triangle.num_triangles = 1;
    return paint;
}
static Paint *strokes_sphere(Canvas *canvas, vec3 center, float radius)
{
    Paint *paint = next_paint(canvas);
    vec3 *vp = (vec3 *) index_buffer(canvas);
    if (g_using_painting_matrix) {
        vp[0] = mat4x4_vec3(g_painting_matrix, center);
    } else {
        vp[0] = center;
    }
    paint->index = canvas->current_index;
    canvas->current_index += 1;
    paint->shape.sphere.radius = radius;
    paint->shape.sphere.num_spheres = 1;
    return paint;
}


/*--------------------------------------------------------------------------------
void paint2d_quad(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y, float p4x, float p4y, COLOR_SCALARS) {
void paint2d_rect(float x, float y, float width, float height, COLOR_SCALARS) {
void paint_quad(float p1x, float p1y, float p1z, 
void paint_loop(float vals[], int num_points, COLOR_SCALARS) {
void paint2d_loop(float vals[], int num_points, COLOR_SCALARS) {
--------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/

void paint_line(int canvas_id, float ax, float ay, float az, float bx, float by, float bz, COLOR_SCALARS, float width)
{
    Paint *paint = strokes_line(painting_canvas(canvas_id), new_vec3(ax, ay, az), new_vec3(bx, by, bz), width);
    paint->type = PAINT_FLAT_LINES;
    paint->contents.flat.color = new_vec4(cr, cg, cb, ca);
}
// void paint_arrow_v(int canvas_id, vec3 a, vec3 b, vec4 color, float width, float head_size)
// {
//     paint_line_v(canvas_id, a, b, color, width);
//     vec3 c = vec3_sub(b, vec3_mul(vec3_normalize(vec3_sub(a, b)), head_size));
//     vec3 ba = vec3_sub(b, a);
//     vec3 v1 = vec3_normalize(vec3_cross(ba, new_vec3(-ba.vals[1], ba.vals[0], ba.vals[2])));
//     vec3 v2 = vec3_normalize(vec3_cross(v1, ba));
//     int n = 25;
//     for (int i = 0; i < n; i++) {
//         int j = (i + 1) % n;
//         vec3 circle_p = vec3_add(c, vec3_add(vec3_mul(v1, sin(2*M_PI*i*1.0/n) * head_size/2.0),
//                                     vec3_add(vec3_mul(v2, cos(2*M_PI*i*1.0/n) * head_size/2.0))));
//         vec3 circle_pp = vec3_add(c, vec3_add(vec3_mul(v1, sin(2*M_PI*j*1.0/n) * head_size/2.0),
//                                     vec3_add(vec3_mul(v2, cos(2*M_PI*j*1.0/n) * head_size/2.0))));
//         paint_triangle_v(canvas_id, b, circle_p, circle_pp, color);
//     }
//}

void paint_points(int canvas_id, vec3 *points, int num_points, COLOR_SCALARS, float size)
{
    Paint *paint = strokes_points(painting_canvas(canvas_id), points, num_points, size);
    paint->type = PAINT_FLAT_POINTS;
    paint->contents.flat.color = new_vec4(cr, cg, cb, ca);
}

void paint_chain(int canvas_id, float vals[], int num_points, COLOR_SCALARS, float width)
{
    Paint *paint = strokes_chain(painting_canvas(canvas_id), vals, num_points, width);
    paint->type = PAINT_FLAT_LINES;
    paint->contents.flat.color = new_vec4(cr, cg, cb, ca);
}
void paint_quad_v(int canvas_id, vec3 a, vec3 b, vec3 c, vec3 d, vec4 color)
{
    Paint *paint = strokes_quad(painting_canvas(canvas_id), a, b, c, d);
    paint->type = PAINT_FLAT_TRIANGLES;
    paint->contents.flat.color = color;
}
void paint_grid_v(int canvas_id, vec3 a, vec3 b, vec3 c, vec3 d, vec4 color, int w, int h, float line_width)
{
    for (int i = 0; i < w + 1; i++) {
        paint_line_v(canvas_id, vec3_add(a, vec3_mul(vec3_sub(b, a), i * 1.0 / w)),
                                vec3_add(d, vec3_mul(vec3_sub(c, d), i * 1.0 / w)), color, line_width);
    }
    for (int i = 0; i < h + 1; i++) {
        paint_line_v(canvas_id, vec3_add(a, vec3_mul(vec3_sub(d, a), i * 1.0 / h)),
                                vec3_add(b, vec3_mul(vec3_sub(c, b), i * 1.0 / h)), color, line_width);
    }
}


void paint_triangle_v(int canvas_id, vec3 a, vec3 b, vec3 c, vec4 color)
{
    Paint *paint = strokes_triangle(painting_canvas(canvas_id), a, b, c);
    paint->type = PAINT_FLAT_TRIANGLES;
    paint->contents.flat.color = color;
}

void paint_loop(int canvas_id, float vals[], int num_points, COLOR_SCALARS, float width)
{
    Paint *paint = strokes_loop(painting_canvas(canvas_id), vals, num_points, width);
    paint->type = PAINT_FLAT_LINES;
    paint->contents.flat.color = new_vec4(cr, cg, cb, ca);
}
void paint_sphere_v(int canvas_id, vec3 center, float radius, vec4 color)
{
    Paint *paint = strokes_sphere(painting_canvas(canvas_id), center, radius);
    paint->type = PAINT_FLAT_SPHERES;
    paint->contents.flat.color = color;
}

void paint_box_v(int canvas_id, vec3 corners[], vec4 color)
{
    // can also be used to paint a frustum, parallelepiped, etc.
    paint_quad_v(canvas_id, corners[0], corners[1], corners[2], corners[3], color);
    paint_quad_v(canvas_id, corners[4], corners[5], corners[6], corners[7], color);
    paint_quad_v(canvas_id, corners[0], corners[1], corners[5], corners[4], color);
    paint_quad_v(canvas_id, corners[2], corners[3], corners[7], corners[6], color);
    paint_quad_v(canvas_id, corners[1], corners[2], corners[6], corners[5], color);
    paint_quad_v(canvas_id, corners[0], corners[3], corners[7], corners[4], color);
}

void paint_wireframe_box_v(int canvas_id, vec3 corners[], vec4 color, float line_width)
{
    // Box should be given as two opposite quads, with joined points having the same relative index.
    for (int i = 0; i < 4; i++) {
        paint_line_v(canvas_id, corners[i], corners[(i+1)%4], color, line_width);
        paint_line_v(canvas_id, corners[4+i], corners[4+(i+1)%4], color, line_width);
        paint_line_v(canvas_id, corners[i], corners[4+i], color, line_width);
    }
}

#define num_layers 256
#define layer2d(LAYER) ( -( LAYER ) * 1.0 / num_layers)

void paint2d_rect(int canvas_id, float x, float y, float width, float height, vec4 color, int layer)
{
    paint_quad_v(canvas_id, new_vec3(x,y,layer2d(layer)), new_vec3(x+width,y,layer2d(layer)), new_vec3(x+width,y+height,layer2d(layer)), new_vec3(x,y+height,layer2d(layer)), color);
}
void paint2d_rect_bordered(int canvas_id, float x, float y, float width, float height, vec4 color, float line_width, vec4 line_color, int layer)
{
    paint_line(canvas_id, x, y, layer2d(layer), x + width, y, layer2d(layer), UNPACK_COLOR(line_color), line_width);
    paint_line(canvas_id, x, y, layer2d(layer), x, y + height, layer2d(layer), UNPACK_COLOR(line_color), line_width);
    paint_line(canvas_id, x + width, y + height, layer2d(layer), x + width, y, layer2d(layer), UNPACK_COLOR(line_color), line_width);
    paint_line(canvas_id, x + width, y + height, layer2d(layer), x, y + height, layer2d(layer), UNPACK_COLOR(line_color), line_width);
    paint2d_rect(canvas_id, x, y, width, height, color, layer);
}

// Painting matrix. This multiplies all incoming paint positions.
void painting_matrix(mat4x4 matrix)
{
    g_using_painting_matrix = true;
    g_painting_matrix = matrix;
}
void painting_matrix_reset(void)
{
    // Just set this flag to false. If a matrix isn't being used, just use the incoming positions, rather than multiplying them
    // by the identity matrix.
    g_using_painting_matrix = false;
}
