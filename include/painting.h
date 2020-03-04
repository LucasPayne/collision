/*================================================================================
================================================================================*/
#ifndef HEADER_DEFINED_PAINTING
#define HEADER_DEFINED_PAINTING
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "matrix_mathematics.h"
#include "resources.h"

// Helper in definitions (since there will be many).
#define COLOR_SCALARS float cr, float cg, float cb, float ca
#define UNPACK_COLOR(color) color.vals[0],color.vals[1],color.vals[2],color.vals[3]
#define UNPACK_VEC3(vector) vector.vals[0],vector.vals[1],vector.vals[2]

typedef uint8_t PaintType;

enum PaintTypes {
    PAINT_NONE,
    PAINT_FLAT_LINES,
    PAINT_FLAT_TRIANGLES,
    PAINT_DASHED_LINES,
    PAINT_SPRITE,
    PAINT_CUSTOM_MATERIAL,
    NUM_PAINT_TYPES
};
typedef struct Paint_s {
    PaintType type;
    uint32_t index; // the index of the vertex in the vertex buffer.
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
        struct PaintShapeLine {
            float width;
        } line;
    } shape;
} Paint;

// right now, canvases have a fixed amount of paint.
#define g_paint_buffer_size (1 << 15)
#define g_paint_vertex_buffer_size ( g_paint_buffer_size * 4 * 3 * 16 )
typedef struct Canvas_s {
    int paint_count;
    int paint_buffer_size;
    Paint paint_buffer[g_paint_buffer_size];
    int current_index;
    uint8_t vertex_buffer[g_paint_vertex_buffer_size];
    // OpenGL stuff
    GLuint vbo; //--rename position_vbo. There should be a buffer for each vertex format. (?)
    GLuint position_vao;
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
#define paint_line_c(CANVAS_ID,AX,AY,AZ,BX,BY,BZ,COLOR_STR,WIDTH)\
{\
    vec4 color = str_to_color_key(COLOR_STR);\
    paint_line(CANVAS_ID,AX,AY,AZ,BX,BY,BZ,UNPACK_COLOR(color),WIDTH);\
}
#define paint_line_v(CANVAS_ID,A,B,COLOR,WIDTH)\
    paint_line(CANVAS_ID,UNPACK_VEC3(( A )),UNPACK_VEC3(( B )),UNPACK_COLOR(( COLOR )),WIDTH)
#define paint_line_cv(CANVAS_ID,A,B,COLOR_STR,WIDTH)\
{\
    vec4 color = str_to_color_key(COLOR_STR);\
    paint_line(CANVAS_ID,UNPACK_VEC3(( A )),UNPACK_VEC3(( B )),UNPACK_COLOR(color),WIDTH);\
}
void paint_loop(int canvas_id, float vals[], int num_points, COLOR_SCALARS, float width);
#define paint_loop_c(CANVAS_ID,VALS,NUM_POINTS,COLOR_STR,WIDTH)\
{\
    vec4 color = str_to_color_key(( COLOR_STR ));\
    paint_loop(CANVAS_ID,VALS,NUM_POINTS,UNPACK_COLOR(color),WIDTH);\
}
#define paint_loop_v(CANVAS_ID,VALS,NUM_POINTS,COLOR,WIDTH)\
    paint_loop(CANVAS_ID,VALS,NUM_POINTS,COLOR,WIDTH);


void paint_chain(int canvas_id, float vals[], int num_points, COLOR_SCALARS, float width);
#define paint_chain_c(CANVAS_ID,VALS,NUM_POINTS,COLOR_STR,WIDTH)\
{\
    vec4 color = str_to_color_key(( COLOR_STR ));\
    paint_chain(CANVAS_ID,VALS,NUM_POINTS,UNPACK_COLOR(color),WIDTH);\
}

void paint_quad_v(int canvas_id, vec3 a, vec3 b, vec3 c, vec3 d, vec4 color);
#define paint_quad_cv(CANVAS_ID,A,B,C,D,COLOR_STR)\
{\
    vec4 color = str_to_color_key(( COLOR_STR ));\
    paint_quad_v(CANVAS_ID,A,B,C,D,color);\
}

void paint_grid_v(int canvas_id, vec3 a, vec3 b, vec3 c, vec3 d, vec4 color, int w, int h, float line_width);
#define paint_grid_cv(CANVAS_ID,A,B,C,D,COLOR_STR,WIDTH,HEIGHT,LINE_WIDTH)\
{\
    vec4 color = str_to_color_key(( COLOR_STR ));\
    paint_grid_v(CANVAS_ID,A,B,C,D,color,WIDTH,HEIGHT,LINE_WIDTH);\
}

void paint_arrow_v(int canvas_id, vec3 a, vec3 b, vec4 color, float width, float head_size);

void paint_triangle_v(int canvas_id, vec3 a, vec3 b, vec3 c, vec4 color);
#define paint_triangle_cv(CANVAS_ID,A,B,C,COLOR_STR)\
{\
    vec4 color = str_to_color_key(( COLOR_STR ));\
    paint_triangle_v(CANVAS_ID,A,B,C,color);\
}

void paint_box_v(int canvas_id, vec3 corners[], vec4 color);
#define paint_box_c(CANVAS_ID,CORNERS,COLOR_STR)\
{\
    vec4 color = str_to_color_key(( COLOR_STR ));\
    paint_box_v(CANVAS_ID,CORNERS,color);\
}

void paint2d_rect(int canvas_id, float x, float y, float width, float height, vec4 color, int layer);
#define paint2d_rect_c(CANVAS_ID,X,Y,WIDTH,HEIGHT,COLOR_STR,LAYER)\
{\
    vec4 color = str_to_color_key(( COLOR_STR ));\
    paint2d_rect(CANVAS_ID,X,Y,WIDTH,HEIGHT,color,LAYER);\
}
void paint2d_rect_bordered(int canvas_id, float x, float y, float width, float height, vec4 color, float line_width, vec4 line_color, int layer);

//--------------------------------------------------------------------------------


#endif // HEADER_DEFINED_PAINTING
