#ifndef HEADER_DEFINED_INTERACTIVE_3D
#define HEADER_DEFINED_INTERACTIVE_3D
#include "matrix_mathematics.h"

/*--------------------------------------------------------------------------------
  Shader blocks useful for materials in an interactive 3D scene
--------------------------------------------------------------------------------*/
ShaderBlockID ShaderBlockID_StandardLoopWindow;
typedef struct ShaderBlock_StandardLoopWindow_s {
    // padding bytes would be in here if neccessary.
    float aspect_ratio;
    float time;
} ShaderBlock_StandardLoopWindow;

ShaderBlockID ShaderBlockID_Standard3D;
typedef struct ShaderBlock_Standard3D_s {
    Matrix4x4f mvp_matrix;
} ShaderBlock_Standard3D;

ShaderBlockID ShaderBlockID_DirectionalLights;
typedef struct ShaderBlock_DirectionalLights_s {
    // todo
} ShaderBlock_DirectionalLights;


#endif // HEADER_DEFINED_INTERACTIVE_3D
