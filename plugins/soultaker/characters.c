#include "../../../src/api.h"
#include <math.h>

#define NUM_STATES 3

st_export void entity_knight_init(GlobalApi* api)
{
}

st_export void knight_walking(GlobalApi* api, Entity* entity, f32 dt)
{
}

st_export void knight_shooting(GlobalApi* api, Entity* entity, f32 dt)
{
}

st_export void entity_knight_update(GlobalApi* api, Entity* entity, f32 dt)
{
}

st_export void entity_knight_create(GlobalApi* api, Entity* entity)
{
    entity->stats.health = 100;
    entity->stats.max_health = 100;
    entity->stats.mana = 50;
    entity->stats.max_mana = 100;
    entity->stats.souls = 50;
    entity->stats.max_souls = 100;
}

st_export void entity_knight_destroy(GlobalApi* api, Entity* entity)
{
}

st_export void entity_knight_cleanup(GlobalApi* api)
{
}
