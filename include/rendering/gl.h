#ifndef HEADER_DEFINED_RENDERING_GL
#define HEADER_DEFINED_RENDERING_GL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdint.h>

typedef GLuint GraphicsID;
typedef GLint GraphicsInt;
typedef GLfloat GraphicsFloat;
typedef struct ___GraphicsMat4x4f_s {
    GLfloat vals[16];
} GraphicsMat4x4f;
typedef GLint GraphicsUniformID;

#define gl_shader_type(SHADER_TYPE)\
	((( SHADER_TYPE ) == Vertex) ? GL_VERTEX_SHADER\
	:((( SHADER_TYPE ) == Fragment) ? GL_FRAGMENT_SHADER\
	  :((( SHADER_TYPE ) == Geometry) ? GL_GEOMETRY_SHADER\
	    : 0)))

//----------Define more
#define gl_type_size(GL_TYPE)\
	((( GL_TYPE ) == GL_FLOAT) ? sizeof(float)\
	:((( GL_TYPE ) == GL_UNSIGNED_INT) ? sizeof(uint32_t)\
	  :((( GL_TYPE ) == GL_INT) ? sizeof(int32_t)\
	    : 0)))

#endif // HEADER_DEFINED_RENDERING_GL