#ifndef QUADMASK_H
#define QUADMASK_H

#include "type.h"

// quadmask data structure is for efficiently querying information
// about a width x height area

typedef struct QuadMask {
    i32 width, length;
    u64* data;
} QuadMask;

QuadMask* quadmask_create(i32 width, i32 length);
void quadmask_destroy(QuadMask* qm);

// sets bit at (x, y)
void quadmask_set(QuadMask* qm, i32 x, i32 y);

// unsets bit at (x, y)
void quadmask_unset(QuadMask* qm, i32 x, i32 y);

// returns true if bit at (x, y) is set
bool quadmask_isset(QuadMask* qm, i32 x, i32 y);

// sets all bits to 0
void quadmask_clear(QuadMask* qm);

// returns whether (x, y) is in the bounds of the quadmask
bool quadmask_in_bounds(QuadMask* qm, i32 x, i32 y);

// prints out the quadmask bits directly to stdout
void quadmask_print(QuadMask* qm);

#endif
