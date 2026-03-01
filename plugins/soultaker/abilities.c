#include "../../src/api.h"

static void other_part_update(GameApi* api, Particle* part, f32 dt)
{
    part->size -= 0.4*dt;
    if (part->size < 0) 
        part->size = 0;
}

static void create_aoe(GameApi* api, vec2 position)
{
    Particle* part;
    vec2 dir;
    vec3 pos3;
    i32 i;
    f32 rad, hue, lifetime;
    AOE* aoe = api->map_create_aoe(position, 1.0f);
    if (aoe == NULL)
        return;
    i32 n = 100;
    lifetime = 0.4;
    pos3 = api->vec3_create(position.x, 0.5, position.z);
    for (i = 0; i < n; i++) {
        rad = (2*PI*i)/n;
        dir = api->vec2_direction(rad);
        part = api->map_create_particle(pos3);
        if (part == NULL)
            return;
        part->velocity.x = 5*dir.x;
        part->velocity.z = 5*dir.z;
        part->update = other_part_update;
        part->lifetime = lifetime;
        hue = api->randf();
        part->color.x = hue;
        part->color.y = hue;
        part->color.z = hue;
        part = api->map_create_particle(pos3);
        if (part == NULL)
            return;
        part->velocity.x = 7*dir.x;
        part->velocity.z = 7*dir.z;
        part->update = other_part_update;
        part->lifetime = lifetime;
        hue = api->randf();
        part->color.x = hue;
        part->color.y = hue;
        part->color.z = hue;
        part = api->map_create_particle(pos3);
        if (part == NULL)
            return;
        part->velocity.x = 9*dir.x;
        part->velocity.z = 9*dir.z;
        part->lifetime = lifetime;
        part->update = other_part_update;
        hue = api->randf();
        part->color.x = hue;
        part->color.y = hue;
        part->color.z = hue;
    }
}

static void update_lob(GameApi* api, Particle* part, f32 dt)
{
    Particle* new_part;
    f32* timer = part->data;
    *timer -= dt;
    if (*timer < 0) {
        new_part = api->map_create_particle(part->position);
        new_part->color.x = new_part->color.y = new_part->color.z = api->randf();
        new_part->lifetime = 0.1;
        *timer += 0.01;
    }
}

static void destroy_lob(GameApi* api, Particle* part)
{
    create_aoe(api, api->vec2_create(part->position.x, part->position.z));
    api->st_free(part->data);
}

st_export void spelltome_cast(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    // https://www.desmos.com/calculator/lybiehprmk
    Particle* part;
    vec2 origin = player->entity->position;
    vec2 offset = api->vec2_sub(target, origin);
    vec2 part_velocity = api->vec2_normalize(offset);
    f32 distance = api->vec2_mag(offset);
    vec3 origin3 = api->vec3_create(origin.x, 0.5, origin.z);
    part = api->map_create_particle(origin3);
    if (part == NULL)
        return;

    f32 g, h, y1, y2, t1, t2, speed;
    g = GRAVITY;
    h = 3.0f;
    y2 = 0.5;
    t1 = sqrt(2*(y2-h)/g);
    y1 = -g*t1;
    t2 = (-y1-sqrt(y1*y1-2*g*y2))/g;
    part->lifetime = t2;
    speed = distance / t2;
    part_velocity = api->vec2_scale(part_velocity, speed);
    part->velocity.x = part_velocity.x;
    part->velocity.y = y1;
    part->velocity.z = part_velocity.z;
    part->acceleration.y = GRAVITY;
    part->data = api->st_malloc(sizeof(f32));
    part->size = 0.2f;
    *((f32*)part->data) = 0.0f;
    part->update = update_lob;
    part->destroy = destroy_lob;
}
