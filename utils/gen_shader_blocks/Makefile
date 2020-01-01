#================================================================================
# Shader block generation utility
# -------------------------------
# This utility takes a file with "block" descriptions, which will be used to generate
# C and glsl files. This is to facilitate easy extension of the uniform data synchronized between
# an application and shaders, and direct usage of these in simple syntax for both the C and glsl code.
# 
# C:
#   For each block in the given file, a C header file is generated which contains
#   the information (global variable/s, struct definitions, etc.) required by the
#   "shader blocks" system, for synchronizing buffers between the application and
#   shaders.
#   The generated structs are padded to align to the std140-layout uniform blocks
#   generated for the glsl shaders. In this way, synchronization is a direct mapping
#   (or submapping) of the buffer to vram.
#
# glsl:
#   For each block in the given file, a glsl "header" file is generated. Since glsl
#   files are handed to the graphics driver, their preprocessing is limited. So, to
#   include these ".glh" files, it is intended for there to be a pre-preprocessor macro
#       #block "{name}.glh"
#
# Implementation
# --------------
# The file->IR is implemented with a flex scanner and bison parser. This takes the file
# and provides a coroutine for retrieving successive Block structures, along with std140 alignment,
# size, and offset information. These are then used to generate the C and glsl code.
#================================================================================
gen_shader_blocks: gen_shader_blocks.c gen_shader_blocks.h lexer.l parser.y
	bison -o parser.yy.c parser.y --defines=tokens.yy.h
	flex -o lexer.yy.c lexer.l
	gcc -c gen_shader_blocks.c
	gcc -o $@ gen_shader_blocks.o lexer.yy.c parser.yy.c
	
.PHONY: clean t1
clean:
	rm gen_shader_blocks *.o gen_shader_blocks.yy.c
t1: gen_shader_blocks
	./gen_shader_blocks tests/test1 -c test_C -g test_glsl

