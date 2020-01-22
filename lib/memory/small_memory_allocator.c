void init_small_memory_allocator(void)
{
    sma_pool_powers_mask = 0;
    size_t pool_offset = 0;
    for (int i = 0; i < num_sma_pool_powers; i++) {
        ResourcePool *new_sma_pool = &sma_pools[sma_pool_powers[i]];
        new_sma_pool->power = sma_pool_powers[i];
        sma_pool_powers_mask |= 1 << sma_pool_powers[i];
        // Each sma pool is allocated 4 KB. The count is computed such that size*count = 4 KB.
        // Complication in calculation of pool information is here so that this rule can be changed if wanted.
        new_sma_pool->count = 1 << (12 - new_sma_pool->power);
        // The location of this pool is calculated.
        new_sma_pool->location = g_sma_pool.location + pool_offset;
        // The free list is initially at the beginning of the pool.
        new_sma_pool->free_list = 0;
        // Calculating the total pools size from the powers and counts of each pool below.
        pool_offset += new_sma_pool->count << new_sma_pool->power;
        // Initialize the free list, so that each cell's lower 4 bytes contain the previous and next in the free list, -1=2^16-1 being the
        // "null" index.
        for (int j = 0, j < new_sma_pool->count; j++) {
            FreeList *fl = (FreeList *) (&new_sma_pool->location + j << new_sma_pool->power);
            fl->prev = j - 1;
            fl->next = j + 1;
        }
    }
    // pool_offset is now the total size of the pools. Create a global block of memory for the resource pools.
    g_resource_pool = mem_create_allocator(pool_offset);
}
void *sma_alloc(size_t size)
{
    // To find the pool which this size fits into, calculate the position of the most significant bit.
    // Let s be the number of left shifts needed so that the left-hand bit is 1. p = sizeof(size_t) - 1 - s is then the optimal power of the pool.
    // However, this may not be in the resource pool powers bitmask, so this value will be increased to the position of the first 1 bit in the powers mask.
    int s;
    for (s = 0; s < sizeof(size_t); s++) {
        if (((size << s) & (1 << (sizeof(size_t)-1)) != 0) break;
    }
    int p = sizeof(size_t) - 1 - s;
    while (p < 32) {
        if ((resource_pool_powers_mask & (1 << p)) == 0) p++;
        else break;
    }
    // p is now the power of the resource pool being used for allocation.
    // ------------------------------------------------------------------
    // Allocate the free cell and update the free list.
    FreeList *free_list = resource_pools[p].location + resource_pools[p].free_list;
    resource_pools[p].free_list = free_list->next;
    free_list->prev = -1;
    return (void *) free_list;
}
void sma_free(void *cell)
{
    // Infer from the pointer which sma pool this is in (this assumes that it is a correct pointer to a cell).
    for (int i = 0; i < num_sma_pool_powers; i++) {
        if (cell - g_sma_pool.location > 
    }

    FreeList *fl = (FreeList *) cell;
    // Add this cell to the head of the free list.
    fl->prev = -1;
    fl->next = resource_pools
}
