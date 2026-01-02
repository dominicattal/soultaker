#include "bitset.h"
#include "malloc.h"
#include <stdio.h>
#include <stdlib.h>

#define SIZE sizeof(u8)

Bitset* bitset_create(i32 length)
{
    Bitset* bs = st_malloc(sizeof(Bitset));
    bs->numset = 0;
    bs->length = length;
    if (length != 0)
        bs->buf = st_calloc((length + SIZE - 1) / SIZE, SIZE);
    else
        bs->buf = NULL;
    return bs;
}

void bitset_set(Bitset* bs, i32 i)
{
    bs->numset++;
    bs->buf[i/SIZE] |= 1 << (i % SIZE);
}

void bitset_setall(Bitset* bs)
{
    bs->numset = bs->length;
    for (i32 i = 0; i < (i32)((bs->length + SIZE - 1) / SIZE); i++)
        bs->buf[i] = ~0;
}

void bitset_unset(Bitset* bs, i32 i)
{
    bs->numset--;
    bs->buf[i/SIZE] &= ~(1<<(i % SIZE));
}

void bitset_unsetall(Bitset* bs)
{
    bs->numset = 0;
    for (i32 i = 0; i < (i32)((bs->length + SIZE - 1) / SIZE); i++)
        bs->buf[i] = 0;
}

i32 bitset_isset(Bitset* bs, i32 i)
{
    return (bs->buf[i / SIZE] >> (i % SIZE)) & 1;
}

void bitset_destroy(Bitset* bs)
{
    st_free(bs->buf);
    st_free(bs);
}

void bitset_print(Bitset* bs)
{
    for (i32 i = 0; i < bs->length; i++)
        printf("%d", (bs->buf[i / SIZE] >> (i%SIZE)) & 1);
    puts("");
}
