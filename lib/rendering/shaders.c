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
#include "dictionary.h"

// glsl include-path. glsl header files are searched for in these paths, which must be added to the path at runtime.
static char *glsl_include_path = NULL;
void glsl_include_path_add(char *directory)
{
    if (glsl_include_path == NULL) {
        // Start the path up as "/path/to/directory".
        glsl_include_path = (char *) malloc(sizeof(char) * (strlen(directory) + 1));
        mem_check(glsl_include_path);
        strcpy(glsl_include_path, directory);
        return;
    }
    // Update the include path to be "/path/to/directory:/another/path", and so on.
    glsl_include_path = (char *) realloc(glsl_include_path, sizeof(char) * (strlen(glsl_include_path) + 1 + strlen(directory) + 1));
    glsl_include_path[strlen(glsl_include_path) - 1] = ':';
    strcpy(strchr(glsl_include_path, '\0'), directory);
}
FILE *glsl_include_path_open(char *name)
{
    if (glsl_include_path == NULL || *glsl_include_path == '\0') {
        fprintf(stderr, ERROR_ALERT "Attempted to open a glsl header file when no glsl include path has been initialized, or the path is empty.\n");
        exit(EXIT_FAILURE);
    }
    const int buf_size = 4096;
    char buf[buf_size];

    char *p = glsl_include_path;
    char *sep;
    do {
        sep = strchr(p + 1, ':');
        if (sep == NULL) sep = strchr(glsl_include_path, '\0');

        // Now p to sep is one entry in the glsl include path.
        if (strlen(name) + (sep - p) > buf_size - 2) { //-2 for the / and null-terminator
            fprintf(stderr, ERROR_ALERT "Directory in glsl include path is too long.\n");
            exit(EXIT_FAILURE);
        }
        memcpy(buf, p, sep - p);
        buf[sep - p] = '/';
        strcpy(buf + (sep - p) + 1, name);
        // Now buf contains /path/to/directory/header_file_name.glh
        FILE *fd = fopen(buf, "r");
        if (fd == NULL) continue;
        else return fd;
    } while (*sep != '\0');
    return NULL;
}


/*--------------------------------------------------------------------------------
  Shader resource
--------------------------------------------------------------------------------*/
ResourceType Shader_RTID;
void *Shader_load(char *path)
{
    /* printf("loading shader from path \"%s\" ...\n", path); */
    /* getchar(); */

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
    Shader *shader = (Shader *) calloc(1, sizeof(Shader));
    mem_check(shader);
    shader->shader_type = shader_type;
    /* printf("Setting shader_id: %u\n", shader_id); */
    shader->shader_id = shader_id;
    return (void *) shader;
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

/*--------------------------------------------------------------------------------
    Shader bookkeeping
--------------------------------------------------------------------------------*/
void read_shader_source(const char *name, char **lines_out[], size_t *num_lines)
{
    // Read the shader source, and process pre-preprocessing directives, which are
    //  - generated block inclusions, #block {blockname}.glh (see the gen_shader_blocks utility).
#define SHADER_SOURCE_LINE_MAX_LENGTH 500
#define MEM_SIZE_START 1024
#define LINES_MEM_SIZE_START 128
    FILE *fd = fopen(name, "r");
    if (fd == NULL) {
        fprintf(stderr, ERROR_ALERT "Shader %s does not exist.\n", name);
        exit(EXIT_FAILURE);
    }
    char line[SHADER_SOURCE_LINE_MAX_LENGTH];
    size_t mem_size = MEM_SIZE_START;
    size_t mem_used = 0;
    size_t lines_mem_size = LINES_MEM_SIZE_START;
    size_t lines_mem_used = 0;

    char *shader_source = (char *) malloc(mem_size * sizeof(char));
    char **lines = (char **) malloc(lines_mem_size * sizeof(char *));
    if (shader_source == NULL || lines == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory to start reading shader source %s.\n", name);
        exit(EXIT_FAILURE);
    }
    // One level of block-inclusion (this is all that is needed).
    bool in_block = false;
    FILE *reading_from_file = fd;
    while (1) {
        if (fgets(line, SHADER_SOURCE_LINE_MAX_LENGTH, reading_from_file) == NULL) {
            if (in_block) {
                // instead of breaking from the while loop, go back to processing the base file.
                in_block = false;
                reading_from_file = fd;
                continue;
            } else break;
        }
        if (strncmp(line, "#block ", 7) == 0) {
            // Source a block file here.
            in_block = true;
            char block_name[1024];
            strncpy(block_name, strchr(line, ' ') + 1, 1024 - 3);
            strcpy(strchr(block_name, '\0') - 1, ".glh"); // remove newline and add .glh.
            reading_from_file = glsl_include_path_open(block_name);
            if (reading_from_file == NULL) {
                fprintf(stderr, ERROR_ALERT "glsl pre-preprocessor could not find shader-block file \"%s\" in the glsl include path.\n", block_name);
                exit(EXIT_FAILURE);
            }
            continue;
        }
        size_t len = strlen(line);
        if (len + mem_used + 1 >= mem_size) {
            while (len + mem_used + 1 >= mem_size) {
                mem_size *= 2;
            }
            char *prev_location = shader_source;
            shader_source = (char *) realloc(shader_source, mem_size * sizeof(char));
            // Update position that the lines array points to.
            for (int i = 0; i < lines_mem_used; i++) {
                lines[i] = shader_source + (lines[i] - prev_location);
            }
            if (shader_source == NULL) {
                fprintf(stderr, ERROR_ALERT "Could not allocate memory when reading shader source %s.\n", name);
                exit(EXIT_FAILURE);
            }
        }
        if (lines_mem_used + 1 >= lines_mem_size) {
            lines_mem_size *= 2;
            lines = (char **) realloc(lines, lines_mem_size * sizeof(char *));
            if (lines == NULL) {
                fprintf(stderr, ERROR_ALERT "Could not allocate memory when reading shader source %s.\n", name);
                exit(EXIT_FAILURE);
            }
        }
        strncpy(shader_source + mem_used, line, len);
        lines[lines_mem_used] = shader_source + mem_used;
        mem_used += len;
        shader_source[mem_used] = '\0';
        mem_used += 1;
        lines_mem_used += 1;
    }
    if (num_lines == 0) {
        // ... only to make sure free(lines[0]) can free the source string on the callers side.
        fprintf(stderr, "ERROR: empty shader file %s\n", name);
        exit(EXIT_FAILURE);
    }

    // Write return values
    *lines_out = lines;
    *num_lines = lines_mem_used;
    fclose(fd);
#undef SHADER_SOURCE_LINE_MAX_LENGTH
#undef MEM_SIZE_START
#undef LINES_MEM_SIZE_START
}
bool load_and_compile_shader(GLuint shader_id, const char *shader_path)
{
#define DEBUG 1
    /* notes:
     * Error handling here should be done in a log system. So, something can still try to compile a shader
     * and fail, and handle that itself, while in testing a log can be checked, without causing a program exit.
     * Right now, this just returns whether it failed or not.
     */
    //=========================================================================================
    // glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length)
    //=========================================================================================
    // Get the source and associate it to the shader
    /* printf("Compiling %u, %s ...\n", shader_id, shader_path); */
    char **lines = NULL;
    size_t num_lines = 0;
    read_shader_source(shader_path, &lines, &num_lines);
#if DEBUG == 1
    for (int i = 0; i < num_lines; i++) {
        printf("%s", lines[i]);
    }
#endif
    glShaderSource(shader_id, num_lines, (const GLchar * const*) lines, NULL);
    free(lines[0]);
    free(lines);
    // Compile the shader and print error logs if needed
    glCompileShader(shader_id);
    GLint compiled;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled);
    // Successfully attempted compilation, checking errors ...
    if (compiled == GL_FALSE) {
	// Print out the log TODO: Print this to a logging system.
        printf(ERROR_ALERT "Shader %s failed to compile.\n", shader_path);
        GLint log_length;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
        char *log = (char *) malloc(log_length * sizeof(char));
        if (log == NULL) {
            fprintf(stderr, ERROR_ALERT "Oh no, could not assign memory to store shader compilation error log.\n");
            exit(EXIT_FAILURE);
        }
        glGetShaderInfoLog(shader_id, log_length, NULL, log);
        //!!!!---------- Really, really do need error logging system.
        printf(ERROR_ALERT "Failed shader compilation error log:\n");
        printf("%s", log);
        free(log);
        return false;
    }
    // Successfully compiled shader
    return true;
#undef DEBUG
}
bool link_shader_program(GLuint shader_program_id)
{
    /* Links the shader program and handles errors and error logs. */
    glLinkProgram(shader_program_id);
    GLint link_status;
    glGetProgramiv(shader_program_id, GL_LINK_STATUS, &link_status);
    if (link_status == GL_FALSE) {
        printf(ERROR_ALERT "Failed to link shader program.\n");
        GLint log_length; 
        glGetProgramiv(shader_program_id, GL_INFO_LOG_LENGTH, &log_length);
        char *log = (char *) malloc(log_length * sizeof(char));
        if (log == NULL) {
            fprintf(stderr, ERROR_ALERT "Oh no, failed to allocate memory for error log for failed link of shader program.\n");
            exit(EXIT_FAILURE);
        }
        glGetProgramInfoLog(shader_program_id, log_length, NULL, log);
        printf(ERROR_ALERT "Failed shader program link error log:\n");
        printf("%s", log);
        free(log);
        return false;
    }
    return true;
}
