/*--------------------------------------------------------------------------------
   Rendering module.
Dependencies:
	resources:  Basic graphics objects are formed as resources, special kinds of shared objects tied to assets.
	dictionary: A basic key-value-pair file format used here for configuring graphics objects.
--------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "helper_definitions.h"
#include "resources.h"
#include "rendering.h"
#include "dictionary.h"
#include "ply.h" // should remove this dependency?


// Rendering resources
//================================================================================
ResourceType Shader_RTID;
static void *Shader_load(char *path)
{
    // Path format:
    //      Drive/path/to/shader.{vert,geom,frag}
    // Corresponds directly to a GLSL shader source file.
    ShaderType shader_type;
    char *suffix;
    // Want to keep the shader asset as a simple file, so infer the shader type from the path. 
    if ((suffix = strrchr(path, '.')) == NULL) return NULL;
    if (strcmp(suffix + 1, "vert") == 0) {
        shader_type = Vertex;
    } else if (strcmp(suffix + 1, "frag") == 0) {
        shader_type = Fragment;
    } else if (strcmp(suffix + 1, "geom") == 0) {
        shader_type = Geometry;
    } else {
        return NULL;
    }
    GLenum gl_shader_type = gl_shader_type(shader_type);
    if (gl_shader_type == 0) return NULL;
    GraphicsID shader_id = glCreateShader(gl_shader_type);
    printf("created id: %d\n", shader_id);
    // Load the shader source from the physical path, and attempt to compile it.
    char shader_path_buffer[1024];
    if (!resource_file_path(path, "", shader_path_buffer, 1024)) {
        fprintf(stderr, "FAILED TO GET SHADER FILE PATH\n");
        exit(EXIT_FAILURE);
        return NULL;
    }
    printf("Shader path: \"%s\"\n", shader_path_buffer);
    if (!load_and_compile_shader(shader_id, shader_path_buffer)) {
        fprintf(stderr, "FAILED TO COMPILE SHADER\n");
        exit(EXIT_FAILURE);
        return NULL;
    }
    // The shader has been compiled and associated to the created shader ID.
    // Create the Shader resource and return it.
    Shader *shader = (Shader *) calloc(1, sizeof(Shader));
    mem_check(shader);
    shader->shader_type = shader_type;
    printf("Setting shader_id: %u\n", shader_id);
    shader->shader_id = shader_id;
    return (void *) shader;
}
ResourceType GraphicsProgram_RTID;
static void *GraphicsProgram_load(char *path)
{
    // Path format:
    //      Drive/path/to/graphicsprogram
    // Associated build files:
    //      .../path/to/graphicsprogram.Program
    // This is a standard key-value pairs file with graphics program information.
    return NULL;
#if 0
    FILE *file = resource_file_open(path, ".Program", "r");
    if (file == NULL) return NULL;

    Dictionary *dic = read_dictionary(file);
    if (dic == NULL) return NULL;
    
    const int buf_size = 64;
    char buf[buf_size];
    if (!dictionary_get("vertex_format", buf, buf_size)) return NULL;
    int i;
    if ((i = string_in(buf, "3,C,3C")) == -1) return NULL;
    free(dic);
#endif
}
ResourceType Mesh_RTID;
static void *Mesh_load(char *path)
{
    return NULL;
}

void init_resources_rendering(void)
{
    add_resource_type(Shader);
    add_resource_type(GraphicsProgram);
    add_resource_type(Mesh);
}
