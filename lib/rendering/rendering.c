
#include "resources.h"
#include "rendering.h"
#include "string_processing.h"

// Rendering resources
//================================================================================
void init_resources_rendering(void)
{
    add_resource_type(Shader);
    add_resource_type(GraphicsProgram);
    add_resource_type(Mesh);
}
ResourceType Shader_RTID;
static void *Shader_load(char *path)
{
    // Path format:
    //      Drive/path/to/shader.format_suffix
    // Corresponds directly to a GLSL shader source file.
    ShaderType shader_type;
    char *suffix;
    // Want to keep the shader asset as a simple file, so infer the shader type from the path. 
    if (suffix = strrchr(path, '.') == NULL) return NULL;
    if (strcmp(suffix + 1, "vert") == 0) {
        shader_type = Vertex;
    } else if (strcmp(suffix + 1, "frag") == 0) {
        shader_type = Fragment;
    } else if (strcmp(suffix + 1, "geom") == 0) {
        shader_type = Geometry;
    } else {
        return NULL;
    }
    GraphicsID shader_id = glCreateShader(shader_type);

    // Load the shader source from the physical path, and attempt to compile it.
    char *shader_path_buffer[1024];
    if (!resource_file_path(path, "", shader_path_buffer, 1024)) return NULL;
    if (!load_and_compile_shader(shader_id, shader_path_buffer)) return NULL;

    // The shader has been compiled and associated to the created shader ID.
    // Create the Shader resource and return it.
    Shader *shader = (Shader *) calloc(1, sizeof(Shader));
    mem_check(shader);
    shader->shader_type = shader_type;
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

    FILE *file = resource_file_open(path, ".Program", "r");
    if (file == NULL) return NULL;

    Dictionary *dic = read_dictionary(file);
    if (dic == NULL) return NULL;
    
    const int buf_size = 64;
    char buf[buf_size];
    if (!key_dictionary("vertex_format", buf, buf_size)) return NULL;
    int i;
    if ((i = string_in(buf, "3,C,3C")) == -1) return NULL;


    free(dic);
}
ResourceType Mesh_RTID;
static void *Mesh_load(char *path)
{

}


void read_shader_source(const char *name, char **lines_out[], size_t *num_lines)
{
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
    while (fgets(line, SHADER_SOURCE_LINE_MAX_LENGTH, fd) != NULL) {
        size_t len = strlen(line);
        if (len + mem_used + 1 >= mem_size) {
            while (len + mem_used + 1 >= mem_size) {
                mem_size *= 2;
            }
            char *prev_location = shader_source;
            shader_source = (char *) realloc(shader_source, mem_size * sizeof(char));
            // Update position that the lines array points to
            for (int i = 0; i < lines_mem_used; i++) {
                lines[i] = shader_source + (lines[i] - prev_location);
            }
            if (shader_source == NULL) {
                fprintf(stderr, "ERROR: Could not allocate memory when reading shader source %s.\n", name);
                exit(EXIT_FAILURE);
            }
        }
        if (lines_mem_used + 1 >= lines_mem_size) {
            lines_mem_size *= 2;
            lines = (char **) realloc(lines, lines_mem_size * sizeof(char *));
            if (lines == NULL) {
                fprintf(stderr, "ERROR: Could not allocate memory when reading shader source %s.\n", name);
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
bool load_and_compile_shader(GraphicsID shader_id, const char *shader_path)
{
    /* notes:
     * Error handling here should be done in a log system. So, something can still try to compile a shader
     * and fail, and handle that itself, without causing a program exit.
     * Right now, this just returns whether it failed or not.
     */
    //=========================================================================================
    // glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length)
    //=========================================================================================
    // Get the source and associate it to the shader
    char **lines = NULL;
    size_t num_lines = 0;
    read_shader_source(shader_path, &lines, &num_lines);
    glShaderSource(shader_id, num_lines, (const GLchar * const*) lines, NULL);
    free(lines[0]);
    free(lines);
    // Compile the shader and print error logs if needed
    glCompileShader(shader_id);
    GLint compiled;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled);
    // Successfully attempted compilation, checking errors ...
    if (compiled == GL_FALSE) {
        return false;
#if 0
        fprintf(stderr, ERROR_ALERT "Shader %s failed to compile.\n", shader_path);
        GLint log_length;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
        char *log = (char *) malloc(log_length * sizeof(char));
        if (log == NULL) {
            fprintf(stderr, ERROR_ALERT "Oh no, could not assign memory to store shader compilation error log.\n");
            exit(EXIT_FAILURE);
        }
        glGetShaderInfoLog(shader_id, log_length, NULL, log);
        fprintf(stderr, ERROR_ALERT "Failed shader compilation error log:\n");
        fprintf(stderr, "%s", log);
        free(log);
        exit(EXIT_FAILURE);
#endif
    }
    // Successfully compiled shader
    return true;
}
void link_shader_program(GraphicsID shader_program_id)
{
    /* Links the shader program and handles errors and error logs. */
    glLinkProgram(shader_program_id);
    GLint link_status;
    glGetProgramiv(shader_program_id, GL_LINK_STATUS, &link_status);
    if (link_status == GL_FALSE) {
        fprintf(stderr, ERROR_ALERT "Failed to link shader program.\n");
        GLint log_length; 
        glGetProgramiv(shader_program_id, GL_INFO_LOG_LENGTH, &log_length);
        char *log = (char *) malloc(log_length * sizeof(char));
        if (log == NULL) {
            fprintf(stderr, ERROR_ALERT "Oh no, failed to allocate memory for error log for failed link of shader program.\n");
            exit(EXIT_FAILURE);
        }
        glGetProgramInfoLog(shader_program_id, log_length, NULL, log);
        fprintf(stderr, ERROR_ALERT "Failed shader program link error log:\n");
        fprintf(stderr, "%s", log);
        free(log);
        exit(EXIT_FAILURE);
    }
}
