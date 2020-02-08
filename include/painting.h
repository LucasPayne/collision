/*================================================================================
================================================================================*/
#ifndef HEADER_DEFINED_PAINTING
#define HEADER_DEFINED_PAINTING
#include "matrix_mathematics.h"
#include "resources.h"

// Helper in definitions (since there will be many).
#define COLOR_SCALARS float cr, float cg, float cb, float ca
#define UNPACK_COLOR(color) color.vals[0],color.vals[1],color.vals[2],color.vals[3]

typedef uint8_t PaintType;
enum PaintTypes {
    PAINT_NONE,
    PAINT_FLAT,
    PAINT_DASHED,
    PAINT_SPRITE,
    PAINT_CUSTOM_MATERIAL,
    NUM_PAINT_TYPES
};
typedef uint8_t PaintShapeType;
enum PaintShapeTypes {
    PAINT_SHAPE_NONE,
    PAINT_SHAPE_LINES,
    PAINT_SHAPE_TRIANGLES,
    NUM_PAINT_SHAPE_TYPES
};
typedef struct Paint_s {
    PaintType type;
    PaintShapeType shape;
    uint32_t offset; // location of the vertices in the vertex buffer.
    // Paint buffers should be filled correctly. The number of vertices, and the offsets of each attribute, are inferred
    // from a calculation with offsets (exception being the last paint, comparing against the current_offset in the canvas's vertex buffer).
    union {
        struct PaintPropertiesFlat {
            vec4 color;
        } flat;
        struct PaintPropertiesDashed {
            vec4 color;
            uint8_t on_amount; // 0: none, 127: half and half, 255: fully on.
            uint8_t length;    // Up to 1 metre length, stored as a byte.
        } dashed;
        struct PaintPropertiesSprite {
            ResourceHandle texture;
            bool destroy; // if this flag is set, the texture handle is destroyed when the canvas is flused.
        } sprite;
        struct PaintPropertiesCustomMaterial {
            ResourceHandle material;
            uint32_t vertex_format;
            bool destroy; // if this flag is set, the material handle is destroyed when the canvas is flused.
        } custom_material;
    } contents;
    union {
        struct PaintShapePropertiesLine {
            float width;
        } line;
    } shape_contents;
} Paint;

// right now, canvases have a fixed amount of paint.
#define g_paint_buffer_size 4096
#define g_paint_vertex_buffer_size ( 4096 * 4 * 3 * 16 )
typedef struct Canvas_s {
    int paint_count;
    int paint_buffer_size;
    Paint paint_buffer[g_paint_buffer_size];
    int current_offset; // current offset in the vertex buffer, where new paint will store its vertices.
    uint8_t vertex_buffer[g_paint_vertex_buffer_size];
} Canvas;

void painting_init(void);
void painting_draw(int canvas_id);
void painting_flush(int canvas_id);

vec4 str_to_color_key(char *color);

// Matrix used for 2d painting.
// x: < 0-1 >
// y: v 0-1 ^
mat4x4 matrix_paint2d(void);

// Standard canvases.
enum Canvases {
    Canvas2D,
    Canvas3D,
    NUM_CANVASES
};

//--------------------------------------------------------------------------------
void paint_line(int canvas_id, float ax, float ay, float az, float bx, float by, float bz, COLOR_SCALARS, float width);
void paint_chain(int canvas_id, float vals[], int num_points, COLOR_SCALARS, float width);
//--------------------------------------------------------------------------------

#if 0
/*--------------------------------------------------------------------------------
    Painting variants.
--------------------------------------------------------------------------------*/
// Lines
void canvas_paint_line_v(int canvas, vec3 a, vec3 b, vec4 color);
void canvas_paint_line(int canvas, float ax, float ay, float az, float bx, float by, float bz, COLOR_SCALARS);
void canvas_paint_line_c(int canvas, float ax, float ay, float az, float bx, float by, float bz, char *color_str);
void canvas_paint_line_cv(int canvas, vec3 a, vec3 b, char *color_str);
void paint_line(float ax, float ay, float az, float bx, float by, float bz, COLOR_SCALARS);
void paint_line_c(float ax, float ay, float az, float bx, float by, float bz, char *color_str);
void paint_line_v(vec3 a, vec3 b, vec4 color);
void paint_line_cv(vec3 a, vec3 b, char *color_str);
void paint2d_line(float ax, float ay, float bx, float by, COLOR_SCALARS);
void paint2d_line_c(float ax, float ay, float bx, float by, char *color_str);

// Chains
void canvas_paint_chain(int canvas, float vals[], int num_points, COLOR_SCALARS);
void canvas_paint_chain_c(int canvas, float vals[], int num_points, char *color_str);
void paint_chain(float vals[], int num_points, COLOR_SCALARS);
void paint_chain_c(float vals[], int num_points, char *color_str);
void paint2d_chain(float vals[], int num_points, COLOR_SCALARS);
void paint2d_chain_c(float vals[], int num_points, char *color_str);

// Loops
void canvas_paint_loop(int canvas, float vals[], int num_points, COLOR_SCALARS);
void canvas_paint_loop_c(int canvas, float vals[], int num_points, char *color_str);
void canvas_paint_loop_v(int canvas, float vals[], int num_points, vec4 color);
void paint_loop(float vals[], int num_points, COLOR_SCALARS);
void paint_loop_c(float vals[], int num_points, char *color_str);
void paint_loop_v(float vals[], int num_points, vec4 color);
void paint2d_loop(float vals[], int num_points, COLOR_SCALARS);
void paint2d_loop_c(float vals[], int num_points, char *color_str);

// Quads
void canvas_paint_quad(int canvas,
                       float p1x, float p1y, float p1z, 
                       float p2x, float p2y, float p2z, 
                       float p3x, float p3y, float p3z, 
                       float p4x, float p4y, float p4z,
                       COLOR_SCALARS);
void canvas_paint_quad_v(int canvas, vec3 p1, vec3 p2, vec3 p3, vec3 p4, vec4 color);
void canvas_paint_quad_c(int canvas,
                         float p1x, float p1y, float p1z, 
                         float p2x, float p2y, float p2z, 
                         float p3x, float p3y, float p3z, 
                         float p4x, float p4y, float p4z,
                         char *color_str);
void paint_quad(float p1x, float p1y, float p1z, 
                float p2x, float p2y, float p2z, 
                float p3x, float p3y, float p3z, 
                float p4x, float p4y, float p4z,
                COLOR_SCALARS);
void paint_quad_v(vec3 p1, vec3 p2, vec3 p3, vec3 p4, vec4 color);
void paint_quad_c(float p1x, float p1y, float p1z, 
                float p2x, float p2y, float p2z, 
                float p3x, float p3y, float p3z, 
                float p4x, float p4y, float p4z,
                char *color_str);
void paint_quad_cv(vec3 p1, vec3 p2, vec3 p3, vec3 p4, char *color_str);
void paint_quad_vm(vec3 p1, vec3 p2, vec3 p3, vec3 p4, ResourceHandle material_handle);
void paint2d_quad(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y, float p4x, float p4y, COLOR_SCALARS);
void paint2d_quad_c(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y, float p4x, float p4y, char *color_str);

// Rectangles
void paint2d_rect(float x, float y, float width, float height, COLOR_SCALARS);
void paint2d_rect_c(float x, float y, float width, float height, char *color_str);

// Sprites
void paint2d_sprite_m(float blx, float bly, float width, float height, ResourceHandle material_handle);
void paint2d_sprite_mh(float blx, float bly, float width, float height, ResourceHandle material_handle);
void paint2d_sprite_mv(float blx, float bly, float width, float height, ResourceHandle material_handle);
void paint2d_sprite(float blx, float bly, float width, float height, ResourceHandle texture_handle);
void paint2d_sprite_p(float blx, float bly, float width, float height, char *texture_path);
#endif

#endif // HEADER_DEFINED_PAINTING
