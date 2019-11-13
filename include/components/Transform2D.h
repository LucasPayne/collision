/*================================================================================
   2D transform component and related functions.
================================================================================*/
#ifndef HEADER_DEFINED_TRANSFORM2D
#define HEADER_DEFINED_TRANSFORM2D

#include "entity.h"

#define Transform2D_TYPE_ID -1

typedef struct Transform2D_s {
    Component component;
    double x;
    double y;
    double theta;
    double scale_x;
    double scale_y;
} Transform2D;
void get_global_transform(Transform2D *transform, Transform2D *to);

#endif // HEADER_DEFINED_TRANSFORM2D
