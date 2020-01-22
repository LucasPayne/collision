/*================================================================================
    This module provides a simple wrapper to a single call to malloc, to allocate
    initial program memory (the "root" allocator).

Usage example:
    mem_init(bytes_GB(2));
    int frame_stack = mem_create_allocator(bytes_MB(512));
        ... initialization
    int small_allocators = mem_create_allocator(bytes_GB(1));
        ... initialization
    int string_heap = mem_create_allocator(bytes_MB(64));
        ... initialization
Creating allocators:
    Allocator implementations use this in the background when for example, a new double-ended stack is created.
    


    
current root allocator (subject to change):
    The root allocator is a stack with a maximal size, and with no pop.
    This is just to provide memory regions intended to be fixed for the lifetime of the program,
    each to be controlled in whatever way the user wants.
================================================================================*/
#ifndef HEADER_DEFINED_MEMORY
#define HEADER_DEFINED_MEMORY
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Initial program memory allocation with malloc.

// Some macros for helping the user define region sizes.
#define bytes_KB(NUM_KILOBYTES) (1024*( NUM_KILOBYTES ))
#define bytes_MB(NUM_MEGABYTES) (1024*1024*( NUM_MEGABYTES ))
#define bytes_GB(NUM_GIGABYTES) (1024*1024*1024*( NUM_GIGABYTES ))

// The user must call this to initialize the root block.
void mem_init(size_t size);

// This root block is used by creating "allocators", which are memory regions usable for any purpouse, but most likely
// as an allocator such as a heap, stack, or pool.
typedef struct MemAllocator_s {
    void *location;
    size_t size;
} MemAllocator;
#define mem_get_allocator(INDEX) g_mem_allocators[( INDEX )]
extern MemAllocator g_mem_allocators[];
int mem_create_allocator(size_t size);

#endif // HEADER_DEFINED_MEMORY
