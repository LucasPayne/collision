/*================================================================================
   Mesh generation module.
================================================================================*/
#ifndef HEADER_DEFINED_MESH_GEN
#define HEADER_DEFINED_MESH_GEN
#include "mesh.h"

void create_cube_mesh(Mesh *mesh, float size);
void make_sphere(Mesh *mesh, float radius, int tess);

#endif // HEADER_DEFINED_MESH_GEN
