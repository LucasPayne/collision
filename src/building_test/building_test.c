/*--------------------------------------------------------------------------------
PROJECT_LIBS:
	+ glad
	+ text_processing
	+ matrix_mathematics
        - testing/lextest
        + ply
--------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "project_definitions.h"
#include "text_processing.h"
#include "matrix_mathematics.h"
#include "testing/lextest.h"
#include "ply.h"

int main(int argc, char *argv[])
{
    Matrix4x4f mat;
    identity_matrix4x4f(&mat);
    print_matrix4x4f(&mat);

    PLYStats stats;
    lex_ply_header(&stats, DATA_DIR "models/plytest2.ply");
    print_ply_stats(&stats);
}
