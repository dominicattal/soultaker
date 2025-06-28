#include "../../../src/api.h"
#include <math.h>

#define NUM_STATES 3

typedef struct {
    f32 state_timer;
} Data;

st_export void entity_knight_init(GlobalApi* api)
{
}

st_export void entity_knight_update(GlobalApi* api, Entity* entity, f32 dt)
{
    f32 frame_length;
    switch (entity->state) {
        case 1:
            frame_length = 2 / (entity->speed + EPSILON);
            entity->frame = fmod(entity->state_timer, frame_length) > frame_length / 2;
            break;
        case 2:
            frame_length = (1 + entity->haste) / 2;
            entity->frame = fmod(entity->state_timer, frame_length) > frame_length / 2;
            break;
        default:
            break;
    }
}

st_export void entity_knight_create(GlobalApi* api, Entity* entity)
{
    entity->data = api->st_malloc(sizeof(Data));
}

st_export void entity_knight_destroy(GlobalApi* api, Entity* entity)
{
    api->st_free(entity->data);
}

st_export void entity_knight_cleanup(GlobalApi* api)
{
}
