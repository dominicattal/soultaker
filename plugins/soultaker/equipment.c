#include "../../src/api.h"

static vec2 pointer_offsets[] = {
    { .x = -0.5, .z = 3.5 },
    { .x = -1, .z = 3 },
    { .x = -1.5, .z = 2.5 },
    { .x = -1, .z = 2.5 },
    { .x = -0.5, .z = 2.5 },
    { .x = -0.5, .z = 2 },
    { .x = -0.5, .z = 1.5 },
    { .x = -0.5, .z = 1 },
    { .x = -0.5, .z = 0.5 },
    { .x = -0.5, .z = 0 },
    { .x = 0, .z = 0 },
    { .x = 0.5, .z = 0 },
    { .x = 0.5, .z = 0.5 },
    { .x = 0.5, .z = 1 },
    { .x = 0.5, .z = 1.5 },
    { .x = 0.5, .z = 2 },
    { .x = 0.5, .z = 2.5 },
    { .x = 1, .z = 2.5 },
    { .x = 1.5, .z = 2.5 },
    { .x = 1.0, .z = 3 },
    { .x = 0.5, .z = 3.5 },
    { .x = 0, .z = 4 },
};

st_export void weapon_pointer_primary(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    vec2 pos = player->entity->position;
    Projectile* proj = api->map_create_projectile(pos);
    proj->direction = direction;
    proj->size = 0.5;
    proj->speed = 20;
    proj->lifetime = 0.15;
    proj->facing = api->vec2_radians(direction);
    proj->tex = api->texture_get_id("bullet");
    api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, 1);
}

st_export void weapon_pointer_secondary(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    vec2 player_pos = player->entity->position;
    for (size_t i = 0; i < sizeof(pointer_offsets) / sizeof(vec2); i++) {
        vec2 offset = pointer_offsets[i];
        offset.x -= 0.5;
        vec2 pos = api->vec2_add(player_pos, api->vec2_rotate(offset, api->vec2_radians(direction) - PI/2));
        Projectile* proj = api->map_create_projectile(pos);
        proj->direction = direction;
        proj->size = 0.5;
        proj->speed = 20;
        proj->lifetime = 0.15;
        proj->facing = api->vec2_radians(direction);
        proj->tex = api->texture_get_id("bullet");
        api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, 1);
    }
}

st_export void weapon_null_pointer_primary(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    vec2 pos = player->entity->position;
    Projectile* proj = api->map_create_projectile(pos);
    proj->direction = direction;
    proj->size = 0.5;
    proj->speed = 20;
    proj->lifetime = 0.15;
    proj->facing = api->vec2_radians(direction);
    proj->tex = api->texture_get_id("null_ptr");
    api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, 1);
}

st_export void weapon_null_pointer_secondary(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    vec2 player_pos = player->entity->position;
    for (size_t i = 0; i < sizeof(pointer_offsets) / sizeof(vec2); i++) {
        vec2 offset = pointer_offsets[i];
        offset.x += 0.5;
        vec2 pos = api->vec2_add(player_pos, api->vec2_rotate(offset, api->vec2_radians(direction) - PI/2));
        Projectile* proj = api->map_create_projectile(pos);
        proj->direction = direction;
        proj->size = 0.5;
        proj->speed = 20;
        proj->lifetime = 0.15;
        proj->facing = api->vec2_radians(direction);
        proj->tex = api->texture_get_id("null_ptr");
        api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, 1);
    }
}

st_export void super_pointer_primary(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    vec2 pos = player->entity->position;
    Projectile* proj = api->map_create_projectile(pos);
    proj->direction = direction;
    proj->size = 0.65;
    proj->speed = 40;
    proj->lifetime = 0.1;
    proj->facing = api->vec2_radians(direction);
    proj->tex = api->texture_get_id("purp_bullet");
    api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, 1);
}

st_export void super_pointer_secondary(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    vec2 player_pos = player->entity->position;
    for (size_t i = 0; i < sizeof(pointer_offsets) / sizeof(vec2); i++) {
        vec2 offset = pointer_offsets[i];
        vec2 pos = api->vec2_add(player_pos, api->vec2_rotate(offset, api->vec2_radians(direction) - PI/2));
        Projectile* proj = api->map_create_projectile(pos);
        proj->direction = direction;
        proj->size = 0.65;
        proj->speed = 40;
        proj->lifetime = 0.1;
        proj->facing = api->vec2_radians(direction);
        proj->tex = api->texture_get_id("purp_bullet");
        api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, 1);
    }
}

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

static void spelltome_update_lob(GameApi* api, Particle* part, f32 dt)
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

static void spelltome_destroy_lob(GameApi* api, Particle* part)
{
    create_aoe(api, api->vec2_create(part->position.x, part->position.z));
    api->st_free(part->data);
}

static void pointer_spelltome_update_lob(GameApi* api, Particle* part, f32 dt)
{
    Particle* new_part;
    f32* timer = part->data;
    *timer -= dt;
    if (*timer < 0) {
        new_part = api->map_create_particle(part->position);
        new_part->color.z = 255;
        new_part->color.x = new_part->color.y = api->randf();
        new_part->lifetime = 0.1;
        *timer += 0.01;
    }
}

static void pointer_spelltome_create_projectiles(GameApi* api, vec2 origin)
{
    Projectile* proj;
    i32 tex_id = api->texture_get_id("bullet");
    for (i32 i = 0; i < 12; i++) {
        proj = api->map_create_projectile(origin);
        if (proj == NULL) return;
        proj->speed = 4.0f;
        proj->size = 0.6f;
        proj->lifetime = 6.0f;
        proj->tex = tex_id;
        proj->facing = PI / 6 * i;
        proj->direction = api->vec2_direction(proj->facing);
        //api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
    }
}

static void pointer_spelltome_destroy_lob(GameApi* api, Particle* part)
{
    pointer_spelltome_create_projectiles(api, api->vec2_create(part->position.x, part->position.z));
    api->st_free(part->data);
}

static void null_pointer_spelltome_update_lob(GameApi* api, Particle* part, f32 dt)
{
    Particle* new_part;
    f32* timer = part->data;
    *timer -= dt;
    if (*timer < 0) {
        new_part = api->map_create_particle(part->position);
        new_part->color.x = 255;
        new_part->color.y = new_part->color.z = api->randf();
        new_part->lifetime = 0.1;
        *timer += 0.01;
    }
}

static void null_pointer_spelltome_create_projectiles(GameApi* api, vec2 origin)
{
    Projectile* proj;
    i32 tex_id = api->texture_get_id("null_ptr");
    for (i32 i = 0; i < 12; i++) {
        proj = api->map_create_projectile(origin);
        if (proj == NULL) return;
        proj->speed = 4.0f;
        proj->size = 0.6f;
        proj->lifetime = 6.0f;
        proj->tex = tex_id;
        proj->facing = PI / 6 * i + PI / 12;
        proj->direction = api->vec2_direction(proj->facing);
        //api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
    }
}

static void null_pointer_spelltome_destroy_lob(GameApi* api, Particle* part)
{
    null_pointer_spelltome_create_projectiles(api, api->vec2_create(part->position.x, part->position.z));
    api->st_free(part->data);
}

static void pointer_null_pointer_spelltome_update_lob(GameApi* api, Particle* part, f32 dt)
{
    Particle* new_part;
    f32* timer = part->data;
    *timer -= dt;
    if (*timer < 0) {
        new_part = api->map_create_particle(part->position);
        new_part->color.x = 255;
        new_part->color.z = 255;
        new_part->color.y = api->randf();
        new_part->lifetime = 0.1;
        *timer += 0.01;
    }
}

static void pointer_null_pointer_spelltome_create_projectiles(GameApi* api, vec2 origin)
{
    Projectile* proj;
    i32 tex_id = api->texture_get_id("purp_bullet");
    for (i32 i = 0; i < 12; i++) {
        proj = api->map_create_projectile(origin);
        if (proj == NULL) return;
        proj->speed = 4.0f;
        proj->size = 0.6f;
        proj->lifetime = 6.0f;
        proj->tex = tex_id;
        proj->facing = PI / 6 * i + PI / 24;
        proj->direction = api->vec2_direction(proj->facing);
        //api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
    }
}

static void pointer_null_pointer_spelltome_destroy_lob(GameApi* api, Particle* part)
{
    pointer_null_pointer_spelltome_create_projectiles(api, api->vec2_create(part->position.x, part->position.z));
    api->st_free(part->data);
}

static void create_lob(GameApi* api, Player* player, vec2 direction, vec2 target, void (*update_lob)(GameApi*, Particle*, f32), void (*destroy_lob)(GameApi*, Particle*))
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

st_export void spelltome_cast(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    create_lob(api, player, direction, target, spelltome_update_lob, spelltome_destroy_lob);
}

st_export void pointer_spelltome_cast(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    create_lob(api, player, direction, target, pointer_spelltome_update_lob, pointer_spelltome_destroy_lob);
}

st_export void null_pointer_spelltome_cast(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    create_lob(api, player, direction, target, null_pointer_spelltome_update_lob, null_pointer_spelltome_destroy_lob);
}

st_export void pointer_null_pointer_spelltome_cast(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    create_lob(api, player, direction, target, pointer_null_pointer_spelltome_update_lob, pointer_null_pointer_spelltome_destroy_lob);
}

static vec2 sword_offsets[] = {
    { .x = 0, .z = 0 },
    { .x = 0, .z = 0.5 },
    { .x = 0.5, .z = 0.5 },
    { .x = 0.5, .z = 1.0 },
    { .x = 0.5, .z = 1.5 },
    { .x = 0.5, .z = 2.0 },
    { .x = 1.0, .z = 2.0 },
    { .x = 1.5, .z = 2.0 },
    { .x = 2.0, .z = 2.0 },
    { .x = 2.0, .z = 2.5 },
    { .x = 2.0, .z = 3.0 },
    { .x = 1.5, .z = 3.0 },
    { .x = 1.0, .z = 3.0 },
    { .x = 0.5, .z = 3.0 },
    { .x = 0.0, .z = 3.0 },
    { .x = -0.5, .z = 3.0 },
    { .x = -1.0, .z = 3.0 },
    { .x = -1.5, .z = 3.0 },
    { .x = -2.0, .z = 3.0 },
    { .x = -2.0, .z = 2.5 },
    { .x = -2.0, .z = 2.0 },
    { .x = -1.5, .z = 2.0 },
    { .x = -1.0, .z = 2.0 },
    { .x = -0.5, .z = 2.0 },
    { .x = 0.0, .z = 2.0 },
    { .x = -0.5, .z = 1.5 },
    { .x = -0.5, .z = 1.0 },
    { .x = -0.5, .z = 0.5 },
    { .x = -0.5, .z = 0.0 },
    { .x = 1.0, .z = 3.5 },
    { .x = 1.0, .z = 4.0 },
    { .x = 1.0, .z = 4.5 },
    { .x = 1.0, .z = 5.0 },
    { .x = 1.0, .z = 5.5 },
    { .x = 1.0, .z = 6.0 },
    { .x = 1.0, .z = 6.5 },
    { .x = 1.0, .z = 7.0 },
    { .x = 0.5, .z = 7.5 },
    { .x = 0.0, .z = 8.0 },
    { .x = -0.5, .z = 7.5 },
    { .x = -1.0, .z = 7.0 },
    { .x = -1.0, .z = 6.5 },
    { .x = -1.0, .z = 6.0 },
    { .x = -1.0, .z = 5.5 },
    { .x = -1.0, .z = 5.0 },
    { .x = -1.0, .z = 4.5 },
    { .x = -1.0, .z = 4.0 },
    { .x = -1.0, .z = 3.5 },
};

st_export void shiv_primary(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    vec2 player_pos = player->entity->position;
    for (size_t i = 0; i < sizeof(sword_offsets) / sizeof(vec2); i++) {
        vec2 offset = sword_offsets[i];
        vec2 pos = api->vec2_add(player_pos, api->vec2_rotate(offset, api->vec2_radians(direction) - PI/2));
        Projectile* proj = api->map_create_projectile(pos);
        proj->direction = direction;
        proj->size = 0.5;
        proj->speed = 20;
        proj->lifetime = 0.15;
        proj->facing = api->vec2_radians(direction);
        proj->tex = api->texture_get_id("bullet");
        api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, 1);
    }
}

st_export void wand_primary(GameApi* api, Player* player, vec2 direction, vec2 target)
{
    vec2 pos = player->entity->position;
    Projectile* proj = api->map_create_projectile(pos);
    proj->direction = api->vec2_create(1, 0);
    proj->size = 0.5;
    proj->speed = 20;
    proj->lifetime = 0.15;
    proj->facing = api->vec2_radians(direction);
    proj->tex = api->texture_get_id("bullet");
    api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, 1);

    proj = api->map_create_projectile(pos);
    proj->direction = api->vec2_create(0, 1);
    proj->size = 0.5;
    proj->speed = 20;
    proj->lifetime = 0.15;
    proj->facing = api->vec2_radians(direction);
    proj->tex = api->texture_get_id("null_ptr");
    api->projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, 1);
}
