#ifndef QUADMASK_H
#define QUADMASK_H

#include "type.h"

// quadmask data structure is for efficiently querying information
// about a width x height area

typedef struct Quadmask {
    i32 width, length;
    u64* data;
} Quadmask;

Quadmask* quadmask_create(i32 width, i32 length);
void quadmask_destroy(Quadmask* qm);

// sets bit at (x, y)
void quadmask_set(Quadmask* qm, i32 x, i32 y);

// unsets bit at (x, y)
void quadmask_unset(Quadmask* qm, i32 x, i32 y);

// returns true if bit at (x, y) is set
bool quadmask_isset(Quadmask* qm, i32 x, i32 y);

// sets all bits to 1
void quadmask_setall(Quadmask* qm);

// sets all bits to 0
void quadmask_clear(Quadmask* qm);

// returns whether (x, y) is in the bounds of the quadmask
bool quadmask_in_bounds(Quadmask* qm, i32 x, i32 y);

// prints out the quadmask bits directly to stdout
void quadmask_print(Quadmask* qm);

#endif
