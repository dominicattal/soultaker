#include "pool.h"
#include "malloc.h"
#include <string.h>

void mem_pool_init(MemoryPool* pool, i32 num_items, size_t item_size)
{
    pool->num_items = num_items;
    pool->item_size = item_size;
    pool->buffer = st_malloc(num_items * item_size);
    pool->free_stack = st_malloc(num_items * sizeof(i32));
    for (i32 i = 0; i < num_items; i++)
        pool->free_stack[i] = i;
    pool->free_stack_head = 0;
}

void mem_pool_cleanup(MemoryPool* pool)
{
    st_free(pool->buffer);
    st_free(pool->free_stack);
}

i32 mem_pool_malloc(MemoryPool* pool)
{
    if (pool->free_stack_head == pool->num_items)
        return -1;
    return pool->free_stack[pool->free_stack_head++];
}

i32 mem_pool_calloc(MemoryPool* pool)
{
    if (pool->free_stack_head == pool->num_items)
        return -1;
    i32 idx = pool->free_stack[pool->free_stack_head++];
    memset(pool->buffer + idx * pool->item_size, 0, pool->item_size);
    return idx;
}

void* mem_pool_get(MemoryPool* pool, i32 idx)
{
    return pool->buffer + (idx * pool->item_size);
}

void mem_pool_free(MemoryPool* pool, i32 idx)
{
    pool->free_stack[--pool->free_stack_head] = idx;
}
