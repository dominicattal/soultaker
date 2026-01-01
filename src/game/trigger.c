#include "internal.h"

Trigger* trigger_create(vec2 position, f32 radius, TriggerFunc func, TriggerDestroyFunc destroy, void* args)
{
    Trigger* trigger = st_malloc(sizeof(Trigger));
    trigger->position = position;
    trigger->radius = radius;
    trigger->func = func;
    trigger->flags = 0;
    trigger->args = args;
    trigger->destroy = destroy;
    return trigger;
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
        st_free(trigger->args);
    else
        trigger->destroy(&game_api, trigger->args);
    st_free(trigger);
}
