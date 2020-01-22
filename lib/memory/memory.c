#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Initial program memory allocation with malloc.
#include "helper_definitions.h"
#include "memory.h"

// 0: Minimal checking.
// 1: Check for correct usage.
#define DEBUG_LEVEL 1

// g_memory is the block allocated exclusively for the application. It is intended for all allocations to be made in allocators which are
// themselves allocated memory in this block.
static void *g_memory = NULL;
static bool g_memory_initialized = false;

#if DEBUG_LEVEL >= 1
#define mem_init_check() { \
    if (!g_memory_initialized) {\
        fprintf(stderr, ERROR_ALERT "mem debug: Initial program memory has not been initialized.\n");\
        exit(EXIT_FAILURE);\
    }\
}
#else
#define mem_init_check() { }
#endif
// All mem_* functions except mem_init must start with this macro.
#define mem_func_debug() {\
    mem_init_check();\
}

void mem_init(size_t size)
{
    // Allocate the program global memory.
    // The only thing you can do with this memory is create allocators. These treat g_memory as a stack. Allocators cannot be deleted.
    if (g_memory_initialized) {
        fprintf(stderr, ERROR_ALERT "Attempted to initialize the initial global program memory while it has already been initialized.\n");
        exit(EXIT_FAILURE);
    }
    g_memory = calloc(1, size);
    if (g_memory == NULL) {
        fprintf(stderr, ERROR_ALERT "Failed to allocate initial program memory block of %zu bytes.\n", size);
        exit(EXIT_FAILURE);
    }
    g_memory_initialized = true;
}
MemAllocator g_mem_allocators[MEM_MAX_NUM_ALLOCATORS] = { 0 };
int mem_create_allocator(size_t size)
{
    mem_func_debug();
    static int num_allocators = 0;
    if (num_allocators >= MEM_MAX_NUM_ALLOCATORS) {
        fprintf(stderr, ERROR_ALERT "mem error: More than the maximum number of memory allocators (%zu) have been created. This maximum can be increased.\n", MEM_MAX_NUM_ALLOCATORS);
        exit(EXIT_FAILURE);
    }
    MemAllocator new_allocator = &g_mem_allocators[num_allocators];
    if (num_allocators == 0) // Start the allocators at the bottom of the stack.
        new_allocator->location = g_memory;
    else                     // Put the next allocator on the top of the stack.
        new_allocator->location = g_mem_allocators[num_allocators - 1].location + g_mem_allocators[num_allocators - 1].size;
    new_allocator->size = size;

    // Check to see that the bounds of the program memory have not been exceeded.
    if ((new_allocator->location + new_allocator->size) - g_memory > size) {
        fprintf(stderr, ERROR_ALERT "mem error: A newly created allocator has exceeded the bounds of the initial program memory.\n");
        exit(EXIT_FAILURE);
    }
    num_allocators ++;
    return num_allocators - 1; // Return the index of the new memory allocator block.
}
