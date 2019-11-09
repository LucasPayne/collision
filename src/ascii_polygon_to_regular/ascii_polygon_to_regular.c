/*
 * Makes regular files for the ascii polygons.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "project_definitions.h"
#include "helper_definitions.h"
#include "data.h"
#include "shapes.h"

int main(int argc, char *argv[])
{
    print_data_directory();

    for (int i = 1; i <= 7; i++) {

        char polygon_name[50];
        sprintf(polygon_name, "%d.poly", i);

        char data_name[50];
        sprintf(data_name, DATA_DIR "polygons/ascii_poly%d.dat", i);

        Polygon poly;
        ascii_polygon(polygon_name, &poly);
        polygon_print(&poly);
        write_polygon(&poly, data_name);
        free(poly.vertices);
    }

    exit(EXIT_SUCCESS);
}
