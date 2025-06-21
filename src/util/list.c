#include "list.h"
#include "malloc.h"
#include <stdlib.h>
#include <assert.h>

#define RESIZE_LENGTH 10

List* list_create(void)
{
    List* list = st_malloc(sizeof(List));
    list->buffer = NULL;
    list->length = list->capacity = 0;
    return list;
}

void  list_append(List* list, void* item)
{
    if (list->length >= list->capacity) {
        list->capacity += RESIZE_LENGTH;
        if (list->buffer == NULL)
            list->buffer = st_malloc(list->capacity * sizeof(void*));
        else
            list->buffer = st_realloc(list->buffer, list->capacity * sizeof(void*));
        assert(list->buffer != NULL);
    }
    list->buffer[list->length++] = item;
}

void  list_remove(List* list, i32 idx)
{
    list->buffer[idx] = list->buffer[--list->length];
    if (list->length % RESIZE_LENGTH == 0) {
        list->capacity = list->length;
        if (list->capacity == 0) {
            free(list->buffer);
            list->buffer = NULL;
        } else
            list->buffer = st_realloc(list->buffer, list->capacity * sizeof(void*));
    }
}

void* list_pop(List* list, i32 idx)
{
    void* data = list->buffer[idx];
    list->buffer[idx] = list->buffer[--list->length];
    if (list->length % RESIZE_LENGTH == 0) {
        list->capacity = list->length;
        if (list->capacity == 0) {
            free(list->buffer);
            list->buffer = NULL;
        } else
            list->buffer = st_realloc(list->buffer, list->capacity * sizeof(void*));
    }
    return data;
}

void  list_clear(List* list)
{
    free(list->buffer);
    list->buffer = NULL;
    list->length = list->capacity = 0;
}

void* list_get(List* list, i32 idx)
{
    return list->buffer[idx];
}

bool  list_empty(List* list)
{
    return list->length == 0;
}

void  list_destroy(List* list)
{
    free(list->buffer);
    free(list);
}
