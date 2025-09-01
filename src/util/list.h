#ifndef LIST_H
#define LIST_H

#include "type.h"

#define LIST_RESIZE_LENGTH 16

typedef struct List {
    void** buffer;
    i32 length, capacity;
} List;

// create new list with empty buffer
List* list_create(void);

// copy an existing list
List* list_copy(List* list);

// add an item at the end of the list. if the list is full, 
// the function will automatically resize the list by LIST_RESIZE_LENGTH
void list_append(List* list, void* item);

// remove an item at idx in the list in O(1) time by replacing the item
// at idx with the item at the end. undefined if idx is out of bounds.
// the function will automatically resize the list modulo LIST_RESIZE_LENGTH
// returns the removed item
void* list_remove(List* list, i32 idx);

// remove an item at idx in the list in O(n) time by shifting everything past
// the idx left by one. undefined if idx is out of bounds.
// the function will automatically resize the list modulo LIST_RESIZE_LENGTH
// returns the removed item
void* list_remove_in_order(List* list, i32 idx);

// find and return idx of the item, returns -1 if not found
i32 list_search(List* list, void* item);

// returns true if list contains item, false otherwise
bool list_contains(List* list, void* item);

// returns true if an item in the list passes the query function
bool list_query(List* list, bool (*fptr)(void* item));

// resets the list's capacity to 0. does not alter any of the list's contents
void list_clear(List* list);

// returns the item at idx. undefined if idx is out of bounds
void* list_get(List* list, i32 idx);

// returns whether the list is empty or not
bool list_empty(List* list);

// randomly rearranges the elements in the list. O(n) time complexity
void list_shuffle(List* list);

// destroys the list. does not alter any of the list's contents
void  list_destroy(List* list);

#endif
