/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <sys/types.h>
#include <dirent.h>

#include "project_definitions.h"
#include "helper_definitions.h"
#include "data.h"
#include "shapes.h"

#define MAX_PATH_LENGTH 400
#define MAX_LINE_LENGTH 500
#define MAX_FILENAME_LENGTH 100

void print_data_directory(void)
{
    opendir_handle(data_dir, DATA_DIR);
    struct dirent *entry;
    while ((entry = readdir(data_dir)) != NULL) {
        puts(entry->d_name);
    }
    closedir(data_dir);
}


void write_polygon(Polygon *poly, char *filename)
{
    fopen_handle(file, filename, "w+");
    for (int i = 0; i < poly->num_vertices; i++) {
        fprintf(file, "%.6lf %.6lf\n", poly->vertices[i].x, poly->vertices[i].y);
    }
}

void read_polygon(char *relative_filename, Polygon *poly)
{
    char filename[MAX_FILENAME_LENGTH];
    strncpy(filename, DATA_DIR "polygons/", MAX_FILENAME_LENGTH);
    strncat(filename, relative_filename, MAX_FILENAME_LENGTH);
    fopen_handle(file, filename, "r");

    int num_vertices = 0;
    int mem_length = 64;
    Point2f *vertices = (Point2f *) malloc(sizeof(Point2f) * mem_length);

    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        Point2f point;
        if (sscanf(line, "%lf %lf", &point.x, &point.y) == EOF) {
            fprintf(stderr, "Error in reading polygon file\n");
            exit(EXIT_FAILURE);
        }
        if (num_vertices >= mem_length) {
            mem_length *= 2;
            vertices = (Point2f *) realloc(vertices, sizeof(Point2f) * mem_length);
        }
        vertices[num_vertices] = point;
        num_vertices ++;
    }
    poly->num_vertices = num_vertices;
    poly->vertices = vertices;
}

#define ASCII_POLYGONS_DIR DATA_DIR "polygons/ascii/"
#define ASCII_NUM_POINTS 36
    // 10 numbers + 26 letters
/*
   File format example
    6_______
    _______5
    ________
    ____3___
    1_______
    ________
    __2____4
*/
#define TRACE 0
void ascii_polygon(char *name, Polygon *polygon)
{
    opendir_handle(dir, ASCII_POLYGONS_DIR);
    char poly_path[MAX_PATH_LENGTH];
    strncpy(poly_path, ASCII_POLYGONS_DIR, MAX_PATH_LENGTH);
    strncat(poly_path, name, MAX_PATH_LENGTH);
    fopen_handle(file, poly_path, "r");

#if TRACE
    printf("Opened ascii polygon file %s.\n", poly_path);
#endif

    // Fill the points up.
    Point2f points[ASCII_NUM_POINTS] = { 0 };
    int line_num = 0;
    int num_points = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] >= '0' && line[i] <= '9') {
                points[line[i] - '0'].x = (double) i;
                points[line[i] - '0'].y = (double) line_num;
                num_points ++;
            }
            else if (line[i] >= 'A' && line[i] <= 'Z') {
                points[10 + (line[i] - 'A')].x = (double) i;
                points[10 + (line[i] - 'A')].y = (double) line_num;
                num_points ++;
            }
	}
        line_num ++;
    }

    // Points read from top to bottom, but point positions have y increasing upwards, so invert these.
    for (int i = 0; i < ASCII_NUM_POINTS; i++) {
        points[i].y = line_num - 1 - points[i].y;
        // --- for now, make it smaller
        points[i].x *= 0.05;
        points[i].y *= 0.05;
    }

#if TRACE
    printf("Got %d points:\n", num_points);
    for (int i = 0; i < num_points; i++) {
        printf("(%.2lf %.2lf)\n", points[i].x, points[i].y);
    }
#endif

    Point2f *polygon_vertices = (Point2f *) malloc(sizeof(Point2f) * num_points);
    mem_check(polygon_vertices);
    for (int i = 0 ; i < num_points; i++) {
        polygon_vertices[i] = points[i];
    }
    
    polygon->vertices = polygon_vertices;
    polygon->num_vertices = num_points;

    closedir(dir);
}
#undef TRACE
