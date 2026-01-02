#ifndef BITSET_H
#define BITSET_H

#include "type.h"

typedef struct Bitset Bitset;

typedef struct Bitset {
    i32 numset;
    i32 length;
    char* buf;
} Bitset;

Bitset* bitset_create(i32 length);
void    bitset_set(Bitset* bs, i32 i);
void    bitset_setall(Bitset* bs);
void    bitset_unset(Bitset* bs, i32 i);
void    bitset_unsetall(Bitset* bs);
i32     bitset_isset(Bitset* bs, i32 i);
void    bitset_destroy(Bitset* bs);
void    bitset_print(Bitset* bs);

#endif
