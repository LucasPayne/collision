
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "helper_definitions.h"
#include "shapes.h"
#include "collision.h"

bool intersection_convex_polygons(Polygon *_polyA, Polygon *_polyB)
{
    // Test symmetric over segments of A and B.
    for (int trial = 0; trial < 2; trial++) {
        Polygon *polyA;
        Polygon *polyB;
        if (trial == 0) {
            Polygon *polyA = _polyA;
            Polygon *polyB = _polyB;
        } else {
            Polygon *polyA = _polyB;
            Polygon *polyB = _polyA;
        }
        // Iterate over segments of polyA.
        for (int i = 0; i < polyA->num_vertices; i++) {
            int j = (i + 1) % polyA->num_vertices;

            // Get vector direction normal to segment.
            Vec2f v = vec_to(polyA->vertices[i], polyA->vertices[j]);
            double temp = v.x;
            v.x = -v.y;
            v.y = temp;

            // Get the extrema of both polygons in this direction.

            // Project A points onto this vector.
            double min_A_on_v = 0.0;
            double max_A_on_v = 0.0;
            bool A_set = false;
            for (int ip = 0; ip < polyA->num_vertices; ip++) {
                double on_v = dot(vec_to(polyA->vertices[i], polyA->vertices[ip]), v);
                if (!A_set || on_v < min_A_on_v) {
                    min_A_on_v = on_v;
                }
                if (!A_set || on_v > max_A_on_v) {
                    max_A_on_v = on_v;
                }
                A_set = true;
            }

            // Project B points onto this vector.
            double min_B_on_v = 0.0;
            double max_B_on_v = 0.0;
            bool B_set = false;
            for (int ip = 0; ip < polyB->num_vertices; ip++) {
                double on_v = dot(vec_to(polyA->vertices[i], polyB->vertices[ip]), v);
                if (!B_set || on_v < min_B_on_v) {
                    min_B_on_v = on_v;
                }
                if (!B_set || on_v > max_B_on_v) {
                    max_B_on_v = on_v;
                }
                B_set = true;
            }
            // Test overlap of these intervals for early out.
            if (max_A_on_v < min_B_on_v || min_A_on_v > max_B_on_v) {
                return false;
            }
        }
    }
    return true;
}


void collision_system_update(void)
{

}
