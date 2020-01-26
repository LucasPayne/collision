/*--------------------------------------------------------------------------------
Shaders component of the rendering module.
This consists of
    - The Shader resource, a compiled and shareable shader object of a certain type (Vertex, fragment, etc.)
    - Functions to work with GLSL shaders and GL shader-related functions,
      such as passing the source to the graphics driver.
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
  Shader resource
--------------------------------------------------------------------------------*/
ResourceType Shader_RTID;
void Shader_load(void *resource, char *path)
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
        shader_type = Geom;
    } else {
        return NULL;
    }
    GLenum gl_shader_type = gl_shader_type(shader_type);
    if (gl_shader_type == 0) return NULL;
    GLuint shader_id = glCreateShader(gl_shader_type);
    printf("created id: %d\n", shader_id);
    // Load the shader source from the physical path, and attempt to compile it.
    char shader_path_buffer[1024];
    int i;
    //////////////////////////////////////////////////////////////////////////////////
    // Awful hack for looking up a resource.
    for (i = 0; i < g_resource_path_count; i++) {
        if (resource_file_path(path, "", shader_path_buffer, 1024, i)) {
            FILE *fd = fopen(shader_path_buffer, "r");
            if (fd != NULL) {
                fclose(fd);
                break;
            }
        }
    }
    if (i == g_resource_path_count) {
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
    Shader *shader = (Shader *) resource;
    shader->shader_type = shader_type;
    shader->shader_id = shader_id;
}
bool Shader_reload(ResourceHandle handle)
{
    fprintf(stderr, "unimplemented\n");
    exit(EXIT_FAILURE);
    /* Shader *shader = resource_data(Shader, handle); */
    /* char shader_path_buffer[1024]; */
    /* if (!resource_file_path(handle.data.path, "", shader_path_buffer, 1024)) return false; */
    /* GLuint test_id = glCreateShader(gl_shader_type(shader->shader_type)); */
    /* if (!load_and_compile_shader(test_id, shader_path_buffer)) return false; */
    /* // The compilation was successful on the test ID, so give this to the resource handle. */
    /* shader->shader_id = test_id; */
    /* return true; */
}
