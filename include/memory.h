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
#include <stdbool.h>
/*--------------------------------------------------------------------------------
    Debugging 
--------------------------------------------------------------------------------*/
// 0: Minimal checking.
// 1: Check for correct usage.
#define MEM_DEBUG_LEVEL 1
extern bool g_memory_initialized;
#if MEM_DEBUG_LEVEL >= 1
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
/*--------------------------------------------------------------------------------
    Some macros for helping the user define region sizes.
--------------------------------------------------------------------------------*/
#define bytes_KB(NUM_KILOBYTES) (1024*( NUM_KILOBYTES ))
#define bytes_MB(NUM_MEGABYTES) (1024*1024*( NUM_MEGABYTES ))
#define bytes_GB(NUM_GIGABYTES) (1024*1024*1024*( NUM_GIGABYTES ))
//--------------------------------------------------------------------------------

#define MEM_MAX_NUM_ALLOCATORS 32

// The user must call this to initialize the root block.
void mem_init(size_t size);

// This root block is used by creating "allocators", which are memory regions usable for any purpouse, but most likely
// as an allocator such as a heap, stack, or pool.
typedef struct MemAllocator_s {
    void *location;
    size_t size;
} MemAllocator;
extern MemAllocator g_mem_allocators[];
MemAllocator mem_create_allocator(size_t size);
/*================================================================================
    Allocators
================================================================================*/

// Small memory allocator
typedef struct SMAPoolInfo_s {
    uint8_t power;
    uint16_t count;
} SMAPoolInfo;
void init_small_memory_allocator(const SMAPoolInfo sma_pool_info[], const int num_sma_pools);
void *sma_alloc(size_t size);
// todo: sma_free

/*================================================================================
    Debugging
================================================================================*/
void small_memory_allocator_debug_overlay(void);

#endif // HEADER_DEFINED_MEMORY
