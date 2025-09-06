#include "quadmask.h"
#include "malloc.h"
#include <stdio.h>

Quadmask* quadmask_create(i32 width, i32 length)
{
    Quadmask* qm = st_malloc(sizeof(Quadmask));
    qm->width = width;
    qm->length = length;
    qm->data = st_calloc((width * length + 63) / 64, sizeof(u64));
    return qm;
}

void quadmask_destroy(Quadmask* qm)
{
    st_free(qm->data);
    st_free(qm);
}

void quadmask_set(Quadmask* qm, i32 x, i32 y)
{
    i32 idx = y * qm->width + x;
    qm->data[idx / 64] |= 1ULL << (63 - (idx % 64));
}

void quadmask_unset(Quadmask* qm, i32 x, i32 y)
{
    i32 idx = y * qm->width + x;
    qm->data[idx / 64] &= ~(1ULL << (63 - (idx % 64)));
}

bool quadmask_isset(Quadmask* qm, i32 x, i32 y)
{
    i32 idx = y * qm->width + x;
    return (qm->data[idx / 64] >> (63 - (idx % 64))) & 1;
}

void quadmask_setall(Quadmask* qm)
{
    for (i32 i = 0; 64 * i < qm->width * qm->length; i++)
        qm->data[i] = ~0LL;
}

void quadmask_clear(Quadmask* qm)
{
    for (i32 i = 0; 64 * i < qm->width * qm->length; i++)
        qm->data[i] = 0;
}

bool quadmask_in_bounds(Quadmask* qm, i32 x, i32 y)
{
    return x >= 0 && x < qm->width && y >= 0 && y < qm->length;
}

void quadmask_print(Quadmask* qm)
{
    for (i32 y = 0; y < qm->length; y++) {
        for (i32 x = 0; x < qm->length; x++)
            printf("%1d", quadmask_isset(qm, x, y));
        puts("");
    }
    puts("");
}
