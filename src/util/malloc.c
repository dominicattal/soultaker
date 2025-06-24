#include "malloc.h"
#include "log.h"

void* _st_malloc(size_t size, const char* file, int line)
{
    void* ptr = malloc(size);
    if (ptr == NULL)
        log_write(MEMCTRL, "%s:%d\nmalloc failed", file, line);
    else {
        log_write(MEMCTRL, "%s:%d\nmemory allocation\naddr: %p\nsize: %lld",
                file, line, ptr, size);
    }
    return ptr;
}
void* _st_realloc(void* ptr, size_t size, const char* file, int line)
{
    union {
        void* ptr;
        long long num;
    } old;
    old.ptr = ptr;
    void* new_ptr = realloc(ptr, size);
    if (new_ptr == NULL)
        log_write(MEMCTRL, "%s:%d\nrealloc failed", file, line);
    else {
        log_write(MEMCTRL, "%s:%d\nmemory reallocation\nold_addr: %p\nnew_addr: %p\nsize: %lld",
                file, line, old.num, new_ptr, size);
    }
    return new_ptr;
}
void* _st_calloc(int cnt, size_t size, const char* file, int line)
{
    void* ptr = calloc(cnt, size);
    if (ptr == NULL)
        log_write(MEMCTRL, "%s:%d\ncalloc failed", file, line);
    else {
        log_write(MEMCTRL, "%s:%d\nmemory 0 allocation\naddr: %p\nsize: %lld", 
                file, line, ptr, size);
    }
    return ptr;
}
void _st_free(void* ptr, const char* file, int line)
{
    if (ptr == NULL)
        log_write(MEMCTRL, "%s:%d\nfreed NULL", file, line, ptr);
    else
        log_write(MEMCTRL, "%s:%d\nfreed memory %p", file, line, ptr);
    free(ptr);
}
