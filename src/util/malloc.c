#include "malloc.h"
#include "log.h"
#include <pthread.h>
#include <string.h>

static size_t heap_size;

#ifdef TRACK_POINTERS

typedef struct {
    const char* file;
    int line;
    size_t size;
    void* addr;
} Pointer;

typedef struct {
    Pointer** buffer;
    int length, capacity;
} PointerList;

static PointerList pointers;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#define RESIZE_LENGTH 10
static void list_append(PointerList* list, void* item)
{
    if (list->length >= list->capacity) {
        list->capacity += RESIZE_LENGTH;
        if (list->buffer == NULL)
            list->buffer = malloc(list->capacity * sizeof(void*));
        else
            list->buffer = realloc(list->buffer, list->capacity * sizeof(void*));
        log_assert(list->buffer, "FUCK");
    }
    list->buffer[list->length++] = item;
}

static void list_remove(PointerList* list, int idx)
{
    list->buffer[idx] = list->buffer[--list->length];
    if (list->length % RESIZE_LENGTH == 0) {
        list->capacity = list->length;
        if (list->capacity == 0) {
            free(list->buffer);
            list->buffer = NULL;
        } else {
            list->buffer = realloc(list->buffer, list->capacity * sizeof(void*));
            log_assert(list->buffer, "FUCK");
        }
    }
}

static void _add_pointer(const char* file, int line, size_t size, void* addr)
{
    Pointer* ptr = malloc(sizeof(Pointer));
    ptr->file = file;
    ptr->line = line;
    ptr->addr = addr;
    ptr->size = size;
    pthread_mutex_lock(&mutex);
    list_append(&pointers, ptr);
    pthread_mutex_unlock(&mutex);
}

static void _remove_pointer(void* addr)
{
    Pointer* pointer;
    pthread_mutex_lock(&mutex);
    for (i32 i = 0; i < pointers.capacity; i++) {
        pointer = pointers.buffer[i];
        if (pointer->addr == addr) { 
            list_remove(&pointers, i);
            goto done;
        }
    }
    log_write(CRITICAL, "Unaccounted pointer %p", addr);
done:
    pthread_mutex_unlock(&mutex);
}

static void _print_pointers(void)
{
    char* string = malloc(200 * pointers.length * sizeof(char));
    strncpy(string, "All allocated pointers\n", 24);
    int idx = 23;
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < pointers.length; i++) {
        Pointer* ptr = pointers.buffer[i];
        sprintf(string+idx, "%p %-16llx %s:%-8d\n", ptr->addr, ptr->size, ptr->file, ptr->line); 
        idx += 16 + 1 + 16 + 1 + strlen(ptr->file) + 1 + 8 + 1;
    }
    pthread_mutex_unlock(&mutex);
    log_write(DEBUG, string);
    free(string);
}

#define print_pointers() _print_pointers();
#define add_pointer(file, line, size, addr) _add_pointer(file, line, size, addr)
#define remove_pointer(addr) _remove_pointer(addr)

#else

#define print_pointers()
#define add_pointer(file, line, size, addr)
#define remove_pointer(addr)

#endif

void* _st_malloc(size_t size, const char* file, int line)
{
    if (size == 0)
        log_write(FATAL, "%s:%d\nalloc 0 bytes", file, line);
    void* ptr = malloc(size + sizeof(size_t));
    size_t* sptr = (size_t*)ptr;
    if (ptr == NULL) {
        log_write(CRITICAL, "%s:%d\nmalloc failed", file, line);
        return NULL;
    }
    log_write(MEMORY, "%s:%d\nmemory allocation\naddr: %p\nsize: %llx", file, line, sptr+1, size);
    __atomic_fetch_add(&heap_size, size, __ATOMIC_SEQ_CST);
    sptr[0] = size;
    add_pointer(file, line, size, (void*)(sptr+1));
    return (void*)(sptr+1);
}
void* _st_realloc(void* ptr, size_t size, const char* file, int line)
{
    if (size == 0)
        log_write(FATAL, "%s:%d\nrealloc 0 bytes", file, line);
    size_t* sptr = (size_t*)(ptr) - 1;
    long long old_addr = (long long)(void*)(sptr);
    void* new_ptr = realloc((void*)sptr, size + sizeof(size_t));
    size_t* new_sptr = (size_t*)new_ptr;
    if (new_ptr == NULL) {
        log_write(CRITICAL, "%s:%d\nrealloc failed", file, line);
        return NULL;
    }
    log_write(MEMORY, "%s:%d\nmemory reallocation\nold_addr: %p\nnew_addr: %p\nsize: %llx", file, line, old_addr+sizeof(size_t), new_sptr+1, size);
    __atomic_fetch_add(&heap_size, size - new_sptr[0], __ATOMIC_SEQ_CST);
    new_sptr[0] = size;
    remove_pointer(ptr);
    add_pointer(file, line, size, (void*)(new_sptr+1));
    return (void*)(new_sptr+1);
}
void* _st_calloc(int cnt, size_t size, const char* file, int line)
{
    if (cnt == 0 || size == 0)
        log_write(FATAL, "%s:%d\ncalloc 0 bytes", file, line);
    void* ptr = calloc(cnt, size + sizeof(size_t));
    size_t* sptr = (size_t*)ptr;
    if (ptr == NULL) {
        log_write(CRITICAL, "%s:%d\ncalloc failed", file, line);
        return NULL;
    }
    log_write(MEMORY, "%s:%d\nmemory callocation\naddr: %p\nsize: %llx", file, line, sptr+1, size);
    __atomic_fetch_add(&heap_size, size, __ATOMIC_SEQ_CST);
    sptr[0] = size;
    add_pointer(file, line, size, (void*)(sptr+1));
    return (void*)(sptr+1);
}
void _st_free(void* ptr, const char* file, int line)
{
    size_t* sptr = (size_t*)(ptr) - 1;
    if (ptr == NULL) {
        log_write(MEMORY, "%s:%d\nfreed NULL", file, line, ptr);
        return;
    }
    size_t size = sptr[0];
    __atomic_fetch_sub(&heap_size, size, __ATOMIC_SEQ_CST);
    log_write(MEMORY, "%s:%d\nfreed memory %p with size %llx", file, line, ptr, size);
    free(sptr);
    remove_pointer(ptr);
}
size_t get_heap_size(void)
{
    return heap_size;
}
void print_heap_info(void)
{
    print_pointers();
    log_write(DEBUG, "Current heap size: %lld", heap_size);
}
