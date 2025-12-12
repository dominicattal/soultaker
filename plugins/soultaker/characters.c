#include "../../src/api.h"
#include <math.h>

#define NUM_STATES 3

st_export void entity_knight_init(GameApi* api)
{
}

st_export void knight_walking(GameApi* api, Entity* entity, f32 dt)
{
}

st_export void knight_shooting(GameApi* api, Entity* entity, f32 dt)
{
}

st_export void entity_knight_update(GameApi* api, Entity* entity, f32 dt)
{
}

st_export void entity_knight_create(GameApi* api, Entity* entity)
{
    entity->health = 100;
    entity->max_health = 100;
}

st_export void entity_knight_destroy(GameApi* api, Entity* entity)
{
}

st_export void entity_knight_cleanup(GameApi* api)
{
}
