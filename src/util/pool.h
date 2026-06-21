#ifndef POOL_H
#define POOL_H

#include "type.h"
#include <stddef.h>

typedef struct {
    char* buffer;
    i32* free_stack;
    size_t item_size;
    i32 num_items;
    i32 free_stack_head;
} MemoryPool;

void    mem_pool_init(MemoryPool* pool, i32 num_items, size_t item_size);
void    mem_pool_cleanup(MemoryPool* pool);

// allocates memory in pool, returns idx in pool if true else -1
i32     mem_pool_malloc(MemoryPool* pool);
i32     mem_pool_calloc(MemoryPool* pool);

void*   mem_pool_get(MemoryPool* pool, i32 idx);
void    mem_pool_free(MemoryPool* pool, i32 idx);

#endif
