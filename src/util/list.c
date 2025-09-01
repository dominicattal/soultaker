#include "list.h"
#include "malloc.h"
#include <stdlib.h>
#include <assert.h>

List* list_create(void)
{
    List* list = st_malloc(sizeof(List));
    list->buffer = NULL;
    list->length = list->capacity = 0;
    return list;
}

List* list_copy(List* list)
{
    List* copy = st_malloc(sizeof(List));
    if (list->capacity != 0)
        copy->buffer = st_malloc(list->capacity * sizeof(void*));
    else
        copy->buffer = NULL;
    for (i32 i = 0; i < list->length; i++)
        copy->buffer[i] = list->buffer[i];
    copy->length = list->length;
    copy->capacity = list->capacity;
    return copy;
}

void list_append(List* list, void* item)
{
    if (list->length >= list->capacity) {
        list->capacity += LIST_RESIZE_LENGTH;
        if (list->buffer == NULL)
            list->buffer = st_malloc(list->capacity * sizeof(void*));
        else
            list->buffer = st_realloc(list->buffer, list->capacity * sizeof(void*));
    }
    list->buffer[list->length++] = item;
}

static void downsize_list(List* list)
{
    if (list->length % LIST_RESIZE_LENGTH != 0)
        return;
    list->capacity = list->length;
    if (list->capacity == 0) {
        st_free(list->buffer);
        list->buffer = NULL;
    } else
        list->buffer = st_realloc(list->buffer, list->capacity * sizeof(void*));
}

void* list_remove(List* list, i32 idx)
{
    void* data = list->buffer[idx];
    list->buffer[idx] = list->buffer[--list->length];
    downsize_list(list);
    return data;
}

void* list_remove_in_order(List* list, i32 idx)
{
    void* data = list->buffer[idx];
    for (i32 i = idx; i < list->length-1; i++)
        list->buffer[i] = list->buffer[i+1];
    list->length--;
    downsize_list(list);
    return data;
}

i32 list_search(List* list, void* item)
{
    for (i32 i = 0; i < list->length; i++)
        if (list->buffer[i] == item)
            return i;
    return -1;
}

bool list_contains(List* list, void* item)
{
    for (i32 i = 0; i < list->length; i++)
        if (list->buffer[i] == item)
            return true;
    return false;
}

bool list_query(List* list, bool (*fptr)(void* item))
{
    for (i32 i = 0; i < list->length; i++)
        if (fptr(list->buffer[i]))
            return true;
    return false;
}

void list_clear(List* list)
{
    st_free(list->buffer);
    list->buffer = NULL;
    list->length = list->capacity = 0;
}

void* list_get(List* list, i32 idx)
{
    return list->buffer[idx];
}

bool list_empty(List* list)
{
    return list->length == 0;
}

void list_shuffle(List* list)
{
    // modern fisher-yates
    i32 i, j;
    void* tmp;
    for (i = 1; i < list->length; i++) {
        j = rand() % (i+1);
        tmp = list->buffer[i];
        list->buffer[i] = list->buffer[j];
        list->buffer[j] = tmp;
    }
}

void  list_destroy(List* list)
{
    st_free(list->buffer);
    st_free(list);
}
