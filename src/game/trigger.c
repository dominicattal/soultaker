#include "internal.h"

Trigger* trigger_create(vec2 position, f32 radius)
{
    Trigger* trigger = st_malloc(sizeof(Trigger));
    trigger->position = position;
    trigger->radius = radius;
    trigger->entities = list_create();
    trigger->bitset = bitset_create(0);
    trigger->enter = NULL;
    trigger->stay = NULL;
    trigger->leave = NULL;
    trigger->destroy = NULL;
    trigger->data = NULL;
    trigger->flags = 0;
    return trigger;
}

void trigger_update(Trigger* trigger)
{
    i32 i, j;
    Bitset* bitset = trigger->bitset;
    List* entities = trigger->entities;
    Entity* entity;
    for (i = j = 0; i < bitset->length; i++) {
        if (bitset_isset(bitset, i)) {
            j++;
            continue;
        }
        entity = list_remove_in_order(entities, j);
        map_handle_trigger_leave(trigger, entity);
    }
    bitset_destroy(trigger->bitset);
    trigger->bitset = bitset_create(trigger->entities->length);
}

void trigger_set_flag(Trigger* trigger, TriggerFlagEnum flag, bool val)
{
    trigger->flags = (trigger->flags & ~(1<<flag)) | (val<<flag);
}

bool trigger_get_flag(Trigger* trigger, TriggerFlagEnum flag)
{
    return (trigger->flags >> flag) & 1;
}

void trigger_destroy(Trigger* trigger)
{
    if (trigger->destroy == NULL)
        st_free(trigger->data);
    else
        trigger->destroy(&game_api, trigger);
    bitset_destroy(trigger->bitset);
    list_destroy(trigger->entities);
    st_free(trigger);
}
