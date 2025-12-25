#include "internal.h"

void trigger_init(void)
{
    game_context.triggers = list_create();
}

Trigger* trigger_create(vec2 position, f32 radius, TriggerFunc func, void* args)
{
    Trigger* trigger = st_malloc(sizeof(Trigger));
    trigger->position = position;
    trigger->radius = radius;
    trigger->func = func;
    trigger->flags = 0;
    trigger->args = args;
    list_append(game_context.triggers, trigger);
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
    st_free(trigger);
}

void trigger_cleanup(void)
{
    if (game_context.triggers == NULL)
        return;
    for (i32 i = 0; i < game_context.triggers->length; i++)
        trigger_destroy(list_get(game_context.triggers, i));
    list_destroy(game_context.triggers);
}
