#ifndef ST_MALLOC_H
#define ST_MALLOC_H

#include <stdlib.h>
#include "type.h"

void* _st_malloc(size_t size, const char* file, int line);
void* _st_realloc(void* ptr, size_t size, const char* file, int line);
void* _st_calloc(int cnt, size_t size, const char* file, int line);
void  _st_free(void* ptr, const char* file, int line);
size_t get_heap_size(void);
void   print_heap_info(void);

#ifdef DEBUG_BUILD
    #define st_malloc(size) _st_malloc(size, __FILE__, __LINE__)
    #define st_realloc(ptr, size) _st_realloc(ptr, size, __FILE__, __LINE__)
    #define st_calloc(cnt, size) _st_calloc(cnt, size, __FILE__, __LINE__)
    #define st_free(ptr) _st_free(ptr, __FILE__, __LINE__)
#else
    #define st_malloc(size) malloc(size)
    #define st_realloc(ptr, size) realloc(ptr, size)
    #define st_calloc(cnt, size) calloc(cnt, size)
    #define st_free(ptr) free(ptr)
#endif

#endif
