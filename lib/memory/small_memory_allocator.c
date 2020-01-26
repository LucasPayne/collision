#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Initial program memory allocation with malloc.
#include <stdbool.h>
#include "helper_definitions.h"
#include "memory.h"

// Memory debugging rendering is built into this module. Maybe it would be better to define a separate header for graphical memory debugging utilities, and
// have a separate module.
#include "painting.h"

/*--------------------------------------------------------------------------------
    Debugging
--------------------------------------------------------------------------------*/
static bool small_memory_allocator_initialized = false;
#define SMA_DEBUG_LEVEL 2
#if SMA_DEBUG_LEVEL >= 2
#define trace(STR) { printf("sma allocator trace: " STR "\n"); }
#else
#define trace(STR) { }
#endif

#if SMA_DEBUG_LEVEL >= 1
#define sma_func_debug() {\
    if (!small_memory_allocator_initialized) {\
        fprintf(stderr, ERROR_ALERT "The small memory allocator has not been initialized.\n");\
        exit(EXIT_FAILURE);\
    }\
}
#else
#define sma_func_debug() { }
#endif

/*================================================================================
notes:
    Current maximums:
        2^2 = 4 byte cells minimum
        2^12 = 4 KB cells maximum
        2^16 - 1 entries per pool maximum
================================================================================*/


static MemAllocator g_small_memory_allocator;
// Define the cell-sizes powers of two of each pool in the small memory allocator.
// This complication in definition is here so that these sizes are easily editable, while
// allowing a bitmask to be created on initialization which is to be used to calculate
// the pool, if any, available for a certain size.
typedef struct FreeList_s {
    uint16_t next; //-1 as max 16-bit int: null.
    uint16_t prev;
} FreeList;
static struct Pool {
    int power;
    int count;
    void *location;
    int16_t free_list;
} sma_pools[32] = { 0 };
static uint32_t sma_pool_powers_mask = 0;

void init_small_memory_allocator(const SMAPoolInfo sma_pool_info[], const int num_sma_pools)
{
    /*--------------------------------------------------------------------------------
        Usage: sma_pool_info is an array containing counts and powers of each pool wanted
               in the small memory allocator.
    --------------------------------------------------------------------------------*/
    mem_func_debug();
    trace("Initializing.");

    // Initialize the free lists now, since the sma memory has been initialized. Two loops are needed
    // since the first calculated the size of the pool so it could be created.
    size_t pool_size = 0;
    for (int i = 0; i < num_sma_pools; i++) {
        pool_size += sma_pool_info[i].count * (1 << sma_pool_info[i].power);
    }
    // pool_offset is now the total size of the pools. Create a global block of memory for the sma pools.
    printf("Creating sma pool of %zu bytes.\n", pool_size);
    g_small_memory_allocator = mem_create_allocator(pool_size);

    sma_pool_powers_mask = 0;
    size_t pool_offset = 0;
    for (int i = 0; i < num_sma_pools; i++) {
        trace("Creating pool.");
        printf("pool offset: %zu\n", pool_offset);
        struct Pool *new_sma_pool = &sma_pools[sma_pool_info[i].power];
        new_sma_pool->power = sma_pool_info[i].power;
        sma_pool_powers_mask |= 1 << sma_pool_info[i].power;
        new_sma_pool->count = sma_pool_info[i].count;
        // The location of this pool is calculated.
        new_sma_pool->location = g_small_memory_allocator.location + pool_offset;
        printf("---- %zu\n", new_sma_pool->location - g_small_memory_allocator.location);
        // The free list is initially at the beginning of the pool.
        new_sma_pool->free_list = 0;
        // Calculating the total pools size from the powers and counts of each pool below.
        pool_offset += new_sma_pool->count << new_sma_pool->power;
        // Make it so that that each cell's lower 4 bytes contain the previous and next in the free list, -1=2^16-1 being the
        // "null" index.
        printf("Initializing %zu-pool ...\n", sma_pool_info[i].power);
        for (int j = 0; j < new_sma_pool->count; j++) {
            FreeList *fl = (FreeList *) (new_sma_pool->location + (j << new_sma_pool->power));
            fl->prev = j - 1;
            fl->next = j == new_sma_pool->count - 1 ? -1 : j + 1; // end the free list if at the end.
        }
        printf("pool:\n\tpower: %zu\n\tcount: %d\n\tfree_list: %d\n\toffset: %zu\n",
                new_sma_pool->power, new_sma_pool->count, new_sma_pool->free_list, new_sma_pool->location - g_small_memory_allocator.location);
    }

    small_memory_allocator_initialized = true;
}
void *sma_alloc(size_t size)
{
    mem_func_debug();
    sma_func_debug();
    // trace("Allocating.");
    //---implement var args for tracing
    printf("Allocating: %zu.\n", size);
    //////////////////////////////////////////////////////////////////////////////////
    //---- Fix this
    //////////////////////////////////////////////////////////////////////////////////
    // To find the pool which this size fits into, calculate the position of the most significant bit.
    // Let s be the number of left shifts needed so that the left-hand bit is 1. p = sizeof(size_t) - 1 - s is then the optimal power of the pool.
    // However, this may not be in the sma pool powers bitmask, so this value will be increased to the position of the first 1 bit in the powers mask.
    // int s;
    // for (s = 0; s < sizeof(size_t); s++) {
    //     if (((size << s) & (1 << (sizeof(size_t)-1))) != 0) break;
    // }
    // int p = sizeof(size_t) - s;
    // while (p < 32) {
    //     if ((sma_pool_powers_mask & (1 << p)) == 0) p++;
    //     else break;
    // }
    int p = 12; // Allocate 4 KB ////////////////
    printf("Pool %d chosen, cell size: %d\n", p, 1 << p);
    // p is now the power of the sma pool being used for allocation.
    // ------------------------------------------------------------------
    // Allocate the free cell and update the free list.
    // =bug note=
    //    There was a bug here with the allocator only shifting by one byte instead of the cell sizes. This clobbered the previous entry.
    //    Note to self: be careful when writing custom memory allocators.
    FreeList *free_list = (FreeList *) (sma_pools[p].location + (sma_pools[p].free_list << sma_pools[p].power));
    sma_pools[p].free_list = free_list->next;
    free_list->prev = -1;
    trace("Allocated succesfully.");
    return (void *) free_list;
}
// void sma_free(void *cell)
// {
//     mem_func_debug();
//     sma_func_debug();
//     // Infer from the pointer which sma pool this is in (this assumes that it is a correct pointer to a cell).
//     int p;
//     for (int i = 0; i < num_sma_pool_powers; i++) {
//         if (cell - g_small_memory_allocator.location > 
//     }
// 
//     FreeList *fl = (FreeList *) cell;
//     // Add this cell to the head of the free list.
//     fl->prev = -1;
//     fl->next = sma_pools[p].free_list;
//     sma_pools[p].free_list = 
// }

/*--------------------------------------------------------------------------------
    Graphical debugging.
note: the memory allocators need to work somewhat already to be able to use this.
--------------------------------------------------------------------------------*/
void small_memory_allocator_debug_overlay(void)
{
    float blx = 0.1;
    float bly = 0.6;
    float trx = 0.9;
    float try = 0.9;
    float width = trx - blx;
    float height = try - bly;
    
    int num_sma_pools = 0;
    for (int i = 0; i < 32; i++) {
        if ((sma_pool_powers_mask & (1 << i)) != 0) num_sma_pools ++;
    }
    float pool_width = width / num_sma_pools;

    int index = 0;
    for (int i = 0; i < 32; i++) {
        if ((sma_pool_powers_mask & (1 << i)) == 0) continue;
        float pool_blx = blx+pool_width*index;
        float pool_bly = bly;
        struct Pool *pool = &sma_pools[i];
        int square_side = 1;
        while (square_side * square_side < pool->count) square_side += 1; // Find the root of the smallest square number which can contain the number of cells.
        float cell_width = pool_width / square_side;
        float cell_height = height / square_side;

        bool cell_states[square_side * square_side];
        memset(cell_states, true, square_side * square_side);
        // Deactivate the cells in the free list.
        FreeList *free_list = (FreeList *) (pool->location + (pool->free_list << pool->power));
        printf("next: %d\n", free_list->next);
        // //---first in free list
        while (free_list->next != -1) {
            printf("next: %d\n", free_list->next);
            cell_states[free_list->next] = false;
            free_list = ((FreeList*) (pool->location + (free_list->next << pool->power)));
        }

        for (int j = 0; j < square_side; j++) {
            for (int k = 0; k < square_side; k++) {
                int cell_index = j*square_side + k;
                if (cell_index >= pool->count) continue;
                if (cell_states[cell_index])
                    paint2d_rect(pool_blx+cell_width*j,pool_bly+cell_height*k,  cell_width,cell_height,  0,j*1.0/square_side,k*1.0/square_side,1);
                // else
                //     paint2d_loop(pool_blx,pool_bly,  pool_blx+cell_width,pool_bly,  pool_blx+cell_width,pool_blx+cell_height,  pool_blx,pool_bly+cell_height,  0,0,1,1);
            }
        }
        index ++;
    }
        

// static struct Pool {
//     int power;
//     int count;
//     void *location;
//     uint16_t free_list;
// } sma_pools[32] = { 0 };

        // paint2d_rect(blx+pool_width*i,bly,  pool_width,height,  0,0,i*1.0/num_sma_pools,1);
}



