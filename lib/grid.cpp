/*
 *
 */

#include <stdbool.h>
#include "grid.h"

#define GRID_DEBUG 0

static unsigned int GRID_HORIZ;
static unsigned int GRID_VERT;
static bool grid_defined = false;
static bool *GRID;

int grid_horiz(void)
{
    return GRID_HORIZ;
}
int grid_vert(void)
{
    return GRID_VERT;
}


double normalized_grid_pos_x(int i)
{
    return ((double) i) / GRID_HORIZ;
}
double normalized_grid_pos_y(int j)
{
    return ((double) j) / GRID_VERT;
}

void normalized_grid_pos(int i, int j, double &x, double &y)
{
    x = ((double) i) / GRID_HORIZ;
    y = ((double) j) / GRID_VERT;
}

void grid_init(int horiz, int vert)
{
    bool *mem_ptr = (bool *) malloc(sizeof(bool) * horiz * vert);
    if (!mem_ptr) {
        fprintf(stderr, "ERROR: failed to allocate memory for the grid.\n");
        exit(EXIT_FAILURE);
    }
    GRID_HORIZ = horiz;
    GRID_VERT = vert;
    GRID = mem_ptr;

    grid_defined = true;
}


bool col_in_grid_range(int i)
{
    return 0 <= i && i < GRID_HORIZ;
}
bool row_in_grid_range(int j)
{
    return 0 <= j && j < GRID_VERT;
}
bool in_grid_range(int i, int j)
{
    return 0 <= i && 0 <= j &&
        i < GRID_HORIZ && j < GRID_VERT;
}

void set_grid(int i, int j, bool val)
{
    GRID_DEFINED_CHECK();
#if GRID_DEBUG
    GRID_CHECK(i, j);
#endif
    GRID[i * GRID_VERT + j] = val;
}

bool grid_val(int i, int j)
{
    GRID_DEFINED_CHECK();
#if GRID_DEBUG
    GRID_CHECK(i, j);
#endif
    return GRID[i * GRID_VERT + j];
}

void grid_clear(void)
{
    FOR_GRID(i, j) {
        set_grid(i, j, false);
    }
}
