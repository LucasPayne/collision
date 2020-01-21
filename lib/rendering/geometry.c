/*--------------------------------------------------------------------------------
    "Geometry" resource and geometry-specification component of the rendering module.
    The Geometry resource, immediate-mode-style geometry definition, and attribute buffer uploading is in this module.
=notes=
This module is not for geometric math, only the "Geometry" resource.
--------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "helper_definitions.h"
#include "resources.h"
#include "rendering.h"

/*--------------------------------------------------------------------------------
Information about available vertex attributes.
=notes=
Remember to add here when new vertex attributes are used!
--------------------------------------------------------------------------------*/
const AttributeInfo g_attribute_info[NUM_ATTRIBUTE_TYPES] = {
    { ATTRIBUTE_TYPE_POSITION, "vPosition", GL_FLOAT, 3 },
    { ATTRIBUTE_TYPE_COLOR, "vColor", GL_FLOAT, 3 },
    { ATTRIBUTE_TYPE_NORMAL, "vNormal", GL_FLOAT, 3},
    { ATTRIBUTE_TYPE_UV, "vTexCoord", GL_FLOAT, 2},
    { ATTRIBUTE_TYPE_INDEX, "vIndex", GL_UNSIGNED_INT, 1},
};

/*--------------------------------------------------------------------------------
The Geometry resource: A collection of primitives over a set of vertices with attributes.
Uses:
    - Geometry+Material pair rendering is a general way to render an object, be it a complex loaded mesh or a single line.
--------------------------------------------------------------------------------*/
ResourceType Geometry_RTID;
/*--------------------------------------------------------------------------------
Loading file-backed Geometry:
  - Triangle mesh described in a .Mesh file
      - PLY, stanford triangle format
--------------------------------------------------------------------------------*/
void *Geometry_load(char *path)
{
    #define load_error(STRING) { fprintf(stderr, ERROR_ALERT "Error loading geometry: %s\n", ( STRING )); exit(EXIT_FAILURE); }
    #define manifest_error(str) load_error("Geometry manifest file has missing or malformed " str " entry.\n")
    DataDictionary *dd = dd_open(g_resource_dictionary, path);
    if (dd == NULL) load_error("Could not open dictionary for geometry.");
    char *vertex_format_string;
    if (!dd_get(dd, "vertex_format", "string", &vertex_format_string)) manifest_error("vertex_format_string");
    VertexFormat vertex_format = string_to_VertexFormat(vertex_format_string);
    if (vertex_format == VERTEX_FORMAT_NONE) load_error("Invalid vertex format given.");
    char *type;
    if (!dd_get(dd, "type", "string", &type)) manifest_error("type"); //- if not doing a fatal error, remember to free the queried strings.

    Geometry geometry;
    if (strcmp(type, "ply") == 0) {
        // Load geometry from a PLY file.
        char *ply_path;
        if (!dd_get(dd, "path", "string", &ply_path)) manifest_error("path");
        FILE *ply_file = resource_file_open(ply_path, "", "r");
        if (ply_file == NULL) load_error("Cannot open resource PLY file.");
        MeshData mesh_data = {0};
        load_mesh_ply(&mesh_data, vertex_format, ply_file);
        geometry = upload_mesh(&mesh_data);
        // Destroy the mesh data.
        for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
            if (mesh_data.attribute_data[i] != NULL) free(mesh_data.attribute_data[i]);
        }
        if (mesh_data.triangles != NULL) free(mesh_data.triangles);
    } else load_error("Invalid geometry-loading type given.");

    Geometry *geometry_out = (Geometry *) calloc(1, sizeof(Geometry));
    mem_check(geometry_out);
    memcpy(geometry_out, &geometry, sizeof(Geometry));
    return geometry_out;
    #undef load_error
    #undef manifest_error
}

/*---Usage details----------------------------------------------------------------
------Geometry specification---
Geometry is specified in the same way whether it is for procedural geometry intended to last for a frame,
or whether the specification is done in a dedicated loading function for meshes to be stored in vram.

------Defining geometry--------
Geometry is specified in a similar way to compatability-mode OpenGL's immediate mode vertex definitions.
However, the product of these calls is a Geometry object (not required to be used as a resource). Checks at the end
determine whether a correct number of attributes for each declared attribute have been specified.

--- Attributes
gm_triangles(VERTEX_FORMAT_3N) begins a geometry specification with attributes "3", 3-dimensional position, and "N", normals.
The attribute_{1f,2f,3f,1i,etc.} functions are for specifying these attributes. Alternatively and sometimes prefered is the attribute_buf function
for uploading a number of the given attribute.

--- Indices
gm_index(3) adds index 3 to the index list. (note: depending on the call used to start the geometry specifcation, the geometry may not be indexed.)

--- Completion and getting the Geometry object
Once the calls have been made,
Geometry g = gm_done();
will complete the specification, check whether it is a correct definition, and return a Geometry object.
The Geometry object does not store vertices or indices, as these are now only in vram. 

--- Rendering and destruction
The Geometry is ready to be paired with a Material for rendering, with gm_draw(geometry, material).
!! IMPORTANT: gm_free(geometry) must be used when the geometry is no longer needed. A Geometry owns vram buffers, which
must be freed. This is especially important with per-frame geometry, as memory leaks to vram will build up fast.
(and also very important with static geometry, as memory leaks to vram will build up slowly.)

------ Example, rendering a colored quad.
gm_triangles(VERTEX_FORMAT_3C);
int a = attribute_3f(Position, 0, 0, 0);
int b = attribute_3f(Position, 1, 0, 0);
int c = attribute_3f(Position, 1, 1, 0);
int d = attribute_3f(Position, 0, 1, 0);
attribute_4f(Color, 0,0,0,1);
attribute_4f(Color, 1,0,0,1);
attribute_4f(Color, 0,1,0,1);
attribute_4f(Color, 0,0,1,1);
gm_index(a); gm_index(b); gm_index(c);
gm_index(a); gm_index(c); gm_index(d);
Geometry g = gm_done();
gm_draw(g, some color-interpolating material);
gm_free(g);
--------------------------------------------------------------------------------*/

/*---Implementation details-------------------------------------------------------
--- Global state
Below is the global state used by the geometry specification system.
=notes=
Some sort of global state is required due to the nature of the call interface (I don't want to change this),
but this could be improved to allow state per-specification, if multithreading is ever used.
--------------------------------------------------------------------------------*/
static bool g_gm_initialized = false;
static bool g_gm_specifying = false;
static Geometry g_geometry;
static uint32_t *g_gm_index_buffer;
static int g_gm_index_count;
static void *g_gm_attribute_buffers[NUM_ATTRIBUTE_TYPES];
static size_t g_gm_attribute_positions[NUM_ATTRIBUTE_TYPES];
static int g_gm_attribute_counts[NUM_ATTRIBUTE_TYPES];

// Initialize the state for the first geometry-specification.
static void gm_init(void)
{
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        g_gm_attribute_buffers[i] = calloc(1, GM_ATTRIBUTE_BUFFER_SIZE);
        mem_check(g_gm_attribute_buffers[i]);
        g_gm_attribute_positions[i] = 0;
    }
    g_gm_index_buffer = calloc(1, GM_INDEX_BUFFER_SIZE);
    mem_check(g_gm_index_buffer);
    g_gm_initialized = true;
}
// Reset the state for a new geometry-specification.
static void gm_reset(void)
{
    memset(&g_geometry, 0, sizeof(Geometry));
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        g_gm_attribute_positions[i] = 0;
        g_gm_attribute_counts[i] = 0;
    }
    g_gm_index_count = 0;
}
#define gm_size_error()\
{\
    fprintf(stderr, ERROR_ALERT "Attempted to specify too-large geometry.\n");\
    exit(EXIT_FAILURE);\
}
#define gm_attribute_error()\
{\
    fprintf(stderr, ERROR_ALERT "Attribute given in geometry upload which does not match the declared vertex format.\n");\
    exit(EXIT_FAILURE);\
}
#define gm_init_check()\
{\
    if (!g_gm_initialized) {\
        gm_init();\
    }\
}

// Functions for starting a geometry-specification.
void gm_triangles(VertexFormat vertex_format)
{
    gm_init_check();
    gm_reset();
    g_gm_specifying = true;
    g_geometry.vertex_format = vertex_format;
    g_geometry.primitive_type = Triangles;
    g_geometry.is_indexed = true;
    g_geometry.dynamic = true;
}
void gm_lines(VertexFormat vertex_format)
{
    gm_init_check();
    gm_reset();
    g_gm_specifying = true;
    g_geometry.vertex_format = vertex_format;
    g_geometry.primitive_type = Lines;
    g_geometry.is_indexed = false;
    g_geometry.dynamic = true;
}

// One-by-one and buffered vertex-attribute and indices definition.
#define attribute_check(SIZE,TYPE)\
{\
    if ((g_geometry.vertex_format & (1 << attribute_type)) == 0) gm_attribute_error();\
    if (g_attribute_info[attribute_type].gl_size != ( SIZE ) || g_attribute_info[attribute_type].gl_type != ( TYPE )) {\
        fprintf(stderr, ERROR_ALERT "Type mismatch when specifying attribute value. Make sure that the right attribute_* function is used.\n");\
        exit(EXIT_FAILURE);\
    }\
}
uint32_t attribute_1u(AttributeType attribute_type, uint32_t u)
{
    attribute_check(1, GL_UNSIGNED_INT);
    size_t pos = g_gm_attribute_positions[attribute_type];
    if (pos + sizeof(uint32_t) > GM_ATTRIBUTE_BUFFER_SIZE) gm_size_error();
    memcpy(g_gm_attribute_buffers[attribute_type] + pos, &u, sizeof(uint32_t));
    // Returns the index, for convenience in defining index lists.
    return g_gm_attribute_counts[attribute_type] ++;
}
uint32_t attribute_2f(AttributeType attribute_type, float a, float b)
{
    attribute_check(2, GL_FLOAT);
    size_t pos = g_gm_attribute_positions[attribute_type];
    if (pos + 3*sizeof(float) > GM_ATTRIBUTE_BUFFER_SIZE) gm_size_error();
    memcpy(g_gm_attribute_buffers[attribute_type] + pos, &a, sizeof(float));
    memcpy(g_gm_attribute_buffers[attribute_type] + pos + sizeof(float), &b, sizeof(float));
    g_gm_attribute_positions[attribute_type] += 2*sizeof(float);
    // Returns the index, for convenience in defining index lists.
    return g_gm_attribute_counts[attribute_type] ++;
}
uint32_t attribute_3f(AttributeType attribute_type, float a, float b, float c)
{
    attribute_check(3, GL_FLOAT);
    size_t pos = g_gm_attribute_positions[attribute_type];
    if (pos + 3*sizeof(float) > GM_ATTRIBUTE_BUFFER_SIZE) gm_size_error();
    memcpy(g_gm_attribute_buffers[attribute_type] + pos, &a, sizeof(float));
    memcpy(g_gm_attribute_buffers[attribute_type] + pos + sizeof(float), &b, sizeof(float));
    memcpy(g_gm_attribute_buffers[attribute_type] + pos + 2*sizeof(float), &c, sizeof(float));
    g_gm_attribute_positions[attribute_type] += 3*sizeof(float);
    // Returns the index, for convenience in defining index lists.
    return g_gm_attribute_counts[attribute_type] ++;
}
#undef attribute_check
void attribute_buf(AttributeType attribute_type, void *buf, int count)
{
    /* Specify the attribute data from a buffer. This will be faster than streaming through, if you already
     * have the data in the format needed for the attribute.
     */
    size_t width = g_attribute_info[attribute_type].gl_size * gl_type_size(g_attribute_info[attribute_type].gl_type); // the width of a single one of these attributes.
    size_t pos = g_gm_attribute_positions[attribute_type];
    if ((g_geometry.vertex_format & (1 << attribute_type)) == 0) gm_attribute_error();
    if (pos + width * count > GM_ATTRIBUTE_BUFFER_SIZE) gm_size_error();
    memcpy(g_gm_attribute_buffers[attribute_type] + pos, buf, width * count);
    g_gm_attribute_positions[attribute_type] += width * count;
    g_gm_attribute_counts[attribute_type] += count;
}
void gm_index(uint32_t index)
{
    if (g_gm_index_count >= GM_INDEX_BUFFER_SIZE) gm_size_error();

    g_gm_index_buffer[g_gm_index_count] = index;
    g_gm_index_count ++;
}
void gm_index_buf(uint32_t *indices, int count)
{
    if (g_gm_index_count + count > GM_INDEX_BUFFER_SIZE) gm_size_error();
    memcpy(g_gm_index_buffer + g_gm_index_count, indices, count * sizeof(uint32_t));
    g_gm_index_count += count;
}

// Completing the geometry specification.
Geometry gm_done(void)
{
    if (!g_gm_specifying) {
        fprintf(stderr, ERROR_ALERT "Cannot call gm_done when no geometry is being specified.\n");
        exit(EXIT_FAILURE);
    }
    // Check that all of the relevant attributes have had the same number of values specified.
    int last_count = -1;
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        if (g_geometry.vertex_format & (1 << i)) {
            if (last_count != -1 && last_count != g_gm_attribute_counts[i]) {
                fprintf(stderr, ERROR_ALERT "Unequal numbers of vertex attributes have been specified in a geometry upload. %d != %d.\n", last_count, g_gm_attribute_counts[i]); 
                exit(EXIT_FAILURE);
            }
            last_count = g_gm_attribute_counts[i];
        }
    }

    glGenVertexArrays(1, &g_geometry.vao_id);
    glBindVertexArray(g_geometry.vao_id);

    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        if (g_geometry.vertex_format & (1 << i)) {
            glGenBuffers(1, &g_geometry.buffer_ids[i]);
            glBindBuffer(GL_ARRAY_BUFFER, g_geometry.buffer_ids[i]);
            glBufferData(GL_ARRAY_BUFFER, g_gm_attribute_positions[i], g_gm_attribute_buffers[i], g_geometry.dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
            glVertexAttribPointer(i,
                                  g_attribute_info[i].gl_size,
                                  g_attribute_info[i].gl_type,
                                  GL_FALSE, // not normalized
                                  0, // no stride
                                  (void *) 0);
            glEnableVertexAttribArray(i);
        }
    }

    if (g_geometry.is_indexed) {
        glGenBuffers(1, &g_geometry.indices_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_geometry.indices_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t)*g_gm_index_count, g_gm_index_buffer, g_geometry.dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    g_gm_specifying = false;

    g_geometry.num_indices = g_gm_index_count;
    g_geometry.num_vertices = last_count;

    return g_geometry;
}

// Rendering a Geometry+Material pair.
void gm_draw(Geometry geometry, Material *material)
{
    MaterialType *mt = resource_data(MaterialType, material->material_type);
    glUseProgram(mt->program_id);
    // Bind the textures
    for (int i = 0; i < mt->num_textures; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, resource_data(Texture, material->textures[i])->texture_id);
    }
    // Prepare the material properties, if there are any.
    if (material->properties != NULL) {
        memcpy(g_shader_blocks[ShaderBlockID_MaterialProperties].shader_block, material->properties, mt->properties_size);
        g_shader_blocks[ShaderBlockID_MaterialProperties].dirty = true; //setting this explicitly since this is not using the macro syntax for accessing entries of the block.
    }
    synchronize_shader_blocks();

    //---- do this map elsewhere.
    GLenum gl_primitive_type;
    switch(geometry.primitive_type) {
        case Triangles: gl_primitive_type = GL_TRIANGLES; break;
        case Lines: gl_primitive_type = GL_LINES; break;
        default:
            fprintf(stderr, ERROR_ALERT "Have not accounted for the primitive type of a piece of geometry passed into gm_draw.\n");
            exit(EXIT_FAILURE);
    }

    glBindVertexArray(geometry.vao_id);
    if (geometry.is_indexed) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.indices_id);
        glDrawElements(gl_primitive_type, geometry.num_indices, GL_UNSIGNED_INT, (void *) 0);
    } else {
        glDrawArrays(gl_primitive_type, 0, geometry.num_vertices);
    }
}

// Destroying the geometry (freeing the vram buffers).
void gm_free(Geometry geometry)
{
    // --- Since this is called for each transient draw, maybe it would be better
    // --- to just allocate an ID for every attribute type, and free them all at once, to reduce
    // --- the number of API calls.
    if (geometry.is_indexed) {
        glDeleteBuffers(1, &geometry.indices_id);
    }
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        if (geometry.vertex_format & (1 << i)) {
            glDeleteBuffers(1, &geometry.buffer_ids[i]);
        }
    }
}
#undef gm_size_error
