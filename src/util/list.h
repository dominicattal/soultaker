#ifndef LIST_H
#define LIST_H

#include "type.h"

typedef struct List {
    void** buffer;
    i32 length, capacity;
} List;

List* list_create(void);
void  list_append(List* list, void* item);
void  list_remove(List* list, i32 idx);
void* list_pop(List* list, i32 idx);
void  list_clear(List* list);
void* list_get(List* list, i32 idx);
bool  list_empty(List* list);
void  list_destroy(List* list);

#endif
