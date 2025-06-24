#include "malloc.h"
#include "log.h"

static size_t heap_size;

void* _st_malloc(size_t size, const char* file, int line)
{
    void* ptr = malloc(size + sizeof(size_t));
    size_t* sptr = (size_t*)ptr;
    if (ptr == NULL) {
        log_write(MEMCTRL, "%s:%d\nmalloc failed", file, line);
        return NULL;
    }
    log_write(MEMCTRL, "%s:%d\nmemory allocation\naddr: %p\nsize: %lld", file, line, sptr+1, size);
    heap_size += size;
    sptr[0] = size;
    return (void*)(sptr+1);
}
void* _st_realloc(void* ptr, size_t size, const char* file, int line)
{
    size_t* sptr = (size_t*)(ptr) - 1;
    union {
        void* ptr;
        long long num;
    } old;
    old.ptr = sptr;
    void* new_ptr = realloc((void*)sptr, size + sizeof(size_t));
    size_t* new_sptr = (size_t*)new_ptr;
    if (new_ptr == NULL) {
        log_write(MEMCTRL, "%s:%d\nrealloc failed", file, line);
        return NULL;
    }
    log_write(MEMCTRL, "%s:%d\nmemory reallocation\nold_addr: %p\nnew_addr: %p\nsize: %lld", file, line, old.num+sizeof(size_t), new_sptr+1, size);
    heap_size += size - new_sptr[0];
    new_sptr[0] = size;
    return (void*)(new_sptr+1);
}
void* _st_calloc(int cnt, size_t size, const char* file, int line)
{
    void* ptr = calloc(cnt, size + sizeof(size_t));
    size_t* sptr = (size_t*)ptr;
    if (ptr == NULL) {
        log_write(MEMCTRL, "%s:%d\ncalloc failed", file, line);
        return NULL;
    }
    log_write(MEMCTRL, "%s:%d\nmemory 0 allocation\naddr: %p\nsize: %lld", file, line, sptr+1, size);
    heap_size += size;
    sptr[0] = size;
    return (void*)(sptr+1);
}
void _st_free(void* ptr, const char* file, int line)
{
    size_t* sptr = (size_t*)(ptr) - 1;
    if (ptr == NULL) {
        log_write(MEMCTRL, "%s:%d\nfreed NULL", file, line, ptr);
        return;
    }
    size_t size = sptr[0];
    heap_size -= size;
    log_write(MEMCTRL, "%s:%d\nfreed memory %p with size %lld", file, line, ptr, size);
    free(sptr);
}
size_t get_heap_size(void)
{
    return heap_size;
}
