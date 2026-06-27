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

void weapon_pointer_primary(Player* player, vec2 direction, vec2 target)
{
    Projectile* proj = map_create_projectile(PROJECTILE_CREATE(
                .position = player->entity->position,
                .direction = direction,
                .size = 0.5,
                .speed = 20,
                .lifetime = 0.15,
                .facing = vec2_radians(direction),
                .tex = texture_get_id("bullet"),
                ));
    projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
}

void weapon_pointer_secondary(Player* player, vec2 direction, vec2 target)
{
    vec2 player_pos = player->entity->position;
    for (size_t i = 0; i < sizeof(pointer_offsets) / sizeof(vec2); i++) {
        vec2 offset = pointer_offsets[i];
        offset.x -= 0.5;
        Projectile* proj = map_create_projectile(PROJECTILE_CREATE(
                    .position = vec2_add(player_pos, vec2_rotate(offset, vec2_radians(direction) - PI/2)),
                    .direction = direction,
                    .size = 0.5,
                    .speed = 20,
                    .lifetime = 0.15,
                    .facing = vec2_radians(direction),
                    .tex = texture_get_id("bullet"),
                    ));
        projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
    }
}

void weapon_null_pointer_primary(Player* player, vec2 direction, vec2 target)
{
    Projectile* proj = map_create_projectile(PROJECTILE_CREATE(
                .position = player->entity->position,
                .direction = direction,
                .size = 0.5,
                .speed = 20,
                .lifetime = 0.15,
                .facing = vec2_radians(direction),
                .tex = texture_get_id("null_ptr"),
                ));
    projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
}

void weapon_null_pointer_secondary(Player* player, vec2 direction, vec2 target)
{
    vec2 player_pos = player->entity->position;
    for (size_t i = 0; i < sizeof(pointer_offsets) / sizeof(vec2); i++) {
        vec2 offset = pointer_offsets[i];
        offset.x += 0.5;
        Projectile* proj = map_create_projectile(PROJECTILE_CREATE(
                    .position = vec2_add(player_pos, vec2_rotate(offset, vec2_radians(direction) - PI/2)),
                    .direction = direction,
                    .size = 0.5,
                    .speed = 20,
                    .lifetime = 0.15,
                    .facing = vec2_radians(direction),
                    .tex = texture_get_id("null_ptr"),
                    ));
        projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
    }
}

void super_pointer_primary(Player* player, vec2 direction, vec2 target)
{
    Projectile* proj = map_create_projectile(PROJECTILE_CREATE(
                .position = player->entity->position,
                .direction = direction,
                .size = 0.65,
                .speed = 40,
                .lifetime = 0.1,
                .facing = vec2_radians(direction),
                .tex = texture_get_id("purp_bullet"),
                ));
    projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
}

void super_pointer_secondary(Player* player, vec2 direction, vec2 target)
{
    vec2 player_pos = player->entity->position;
    for (size_t i = 0; i < sizeof(pointer_offsets) / sizeof(vec2); i++) {
        vec2 offset = pointer_offsets[i];
        Projectile* proj = map_create_projectile(PROJECTILE_CREATE(
                    .position = vec2_add(player_pos, vec2_rotate(offset, vec2_radians(direction) - PI/2)),
                    .direction = direction,
                    .size = 0.65,
                    .speed = 40,
                    .lifetime = 0.1,
                    .facing = vec2_radians(direction),
                    .tex = texture_get_id("purp_bullet"),
                    ));
        projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
    }
}

void other_part_update(Particle* part, f32 dt)
{
    part->size -= 0.4*dt;
    if (part->size < 0) 
        part->size = 0;
}

static void create_aoe(vec2 position)
{
    Particle* part;
    vec2 dir;
    vec3 pos3;
    i32 i;
    f32 rad, hue;
    bool success;

    AOE* aoe = map_create_aoe(position, 1.0f);
    if (aoe == NULL)
        return;

    i32 part_id = particle_get_id("other_part");

    i32 n = 100;
    pos3 = vec3_create(position.x, 0.5, position.z);
    for (i = 0; i < n; i++) {
        rad = (2*PI*i)/n;
        dir = vec2_direction(rad);

        for (i32 j = 5; j <= 9; j += 2) {
            hue = randf();
            success = map_create_particle(PARTICLE_CREATE(
                        .position = pos3,
                        .velocity = vec3_create(j*dir.x, 0, j*dir.z),
                        .color = vec3_create(hue, hue, hue),
                        .lifetime = 0.4,
                        .id = part_id
                        ));
            if (!success)
                return;
        }
    }
}

void spelltome_update_lob(Particle* part, f32 dt)
{
    Particle* new_part;
    f32* timer = part->data;
    *timer -= dt;
    if (*timer < 0) {
        map_create_particle(PARTICLE_CREATE(
                    .position = part->position,
                    .color = vec3_create(randf(), randf(), randf()),
                    .lifetime = 0.1
                    ));
        *timer += 0.01;
    }
}

void spelltome_destroy_lob(Particle* part)
{
    create_aoe(vec2_create(part->position.x, part->position.z));
    st_free(part->data);
}

void pointer_spelltome_update_lob(Particle* part, f32 dt)
{
    Particle* new_part;
    f32* timer = part->data;
    *timer -= dt;
    if (*timer < 0) {
        f32 hue = randf();
        map_create_particle(PARTICLE_CREATE(
                    .position = part->position,
                    .color = vec3_create(hue, hue, 255),
                    .lifetime = 0.1
                    ));
        *timer += 0.01;
    }
}

static void pointer_spelltome_create_projectiles(vec2 origin)
{
    Projectile* proj;
    i32 tex_id = texture_get_id("bullet");
    for (i32 i = 0; i < 12; i++) {
        f32 facing = (PI / 6) * i;
        proj = map_create_projectile(PROJECTILE_CREATE(
                    .position = origin,
                    .speed = 4.0f,
                    .size = 0.6f,
                    .lifetime = 6.0f,
                    .tex = tex_id,
                    .facing = facing,
                    .direction = vec2_direction(facing),
                    ));
    }
}

void pointer_spelltome_destroy_lob(Particle* part)
{
    pointer_spelltome_create_projectiles(vec2_create(part->position.x, part->position.z));
    st_free(part->data);
}

void null_pointer_spelltome_update_lob(Particle* part, f32 dt)
{
    Particle* new_part;
    f32* timer = part->data;
    *timer -= dt;
    if (*timer < 0) {
        f32 hue = randf();
        map_create_particle(PARTICLE_CREATE(
                    .position = part->position,
                    .color = vec3_create(hue, hue, 255),
                    .lifetime = 0.1
                    ));
        *timer += 0.01;
    }
}

void null_pointer_spelltome_create_projectiles(vec2 origin)
{
    Projectile* proj;
    i32 tex_id = texture_get_id("null_ptr");
    for (i32 i = 0; i < 12; i++) {
        f32 facing = PI / 6 * i + PI / 12;
        proj = map_create_projectile(PROJECTILE_CREATE(
                    .position = origin, 
                    .speed = 4.0f,
                    .size = 0.6f,
                    .lifetime = 6.0f,
                    .tex = tex_id,
                    .facing = facing,
                    .direction = vec2_direction(facing)
                    ));
        //projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
    }
}

void null_pointer_spelltome_destroy_lob(Particle* part)
{
    null_pointer_spelltome_create_projectiles(vec2_create(part->position.x, part->position.z));
    st_free(part->data);
}

void pointer_null_pointer_spelltome_update_lob(Particle* part, f32 dt)
{
    Particle* new_part;
    f32* timer = part->data;
    *timer -= dt;
    if (*timer < 0) {
        map_create_particle(PARTICLE_CREATE(
                    .position = part->position,
                    .color = vec3_create(randf(), randf(), randf()),
                    .lifetime = 0.1
                    ));
        *timer += 0.01;
    }
}

void pointer_null_pointer_spelltome_create_projectiles(vec2 origin)
{
    Projectile* proj;
    i32 tex_id = texture_get_id("purp_bullet");
    for (i32 i = 0; i < 12; i++) {
        f32 facing = PI / 6 * i + PI / 24;
        proj = map_create_projectile(PROJECTILE_CREATE(
                    .position = origin,
                    .speed = 4.0f,
                    .size = 0.6f,
                    .lifetime = 6.0f,
                    .tex = tex_id,
                    .facing = facing,
                    .direction = vec2_direction(facing),
                    ));
        //projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
    }
}

void pointer_null_pointer_spelltome_destroy_lob(Particle* part)
{
    pointer_null_pointer_spelltome_create_projectiles(vec2_create(part->position.x, part->position.z));
    st_free(part->data);
}

void spelltome_create_lob(Particle* particle)
{
    particle->data = st_malloc(sizeof(f32));
    *((f32*)particle->data) = 0.0f;
}

static void create_lob(Player* player, vec2 direction, vec2 target, i32 part_id)
{
    // https://www.desmos.com/calculator/lybiehprmk
    Particle* part;
    vec2 origin = player->entity->position;
    vec2 offset = vec2_sub(target, origin);
    vec2 part_velocity = vec2_normalize(offset);
    vec3 velocity;
    f32 distance = vec2_mag(offset);
    f32 g, h, y1, y2, t1, t2, speed;
    g = GRAVITY;
    h = 3.0f;
    y2 = 0.5;
    t1 = sqrt(2*(y2-h)/g);
    y1 = -g*t1;
    t2 = (-y1-sqrt(y1*y1-2*g*y2))/g;
    speed = distance / t2;
    part_velocity = vec2_scale(part_velocity, speed);
    velocity.x = part_velocity.x;
    velocity.y = y1;
    velocity.z = part_velocity.z;
    bool success = map_create_particle(PARTICLE_CREATE(
                .position = vec3_create(origin.x, 0.5, origin.z),
                .velocity = velocity,
                .acceleration = vec3_create(0, GRAVITY, 0),
                .lifetime = t2,
                .size = 0.2,
                .id = part_id
                ));
}

void spelltome_cast(Player* player, vec2 direction, vec2 target)
{
    i32 part_id = particle_get_id("spelltome_lob");
    create_lob(player, direction, target, part_id);
}

void pointer_spelltome_cast(Player* player, vec2 direction, vec2 target)
{
    i32 part_id = particle_get_id("pointer_spelltome_lob");
    create_lob(player, direction, target, part_id);
}

void null_pointer_spelltome_cast(Player* player, vec2 direction, vec2 target)
{
    i32 part_id = particle_get_id("null_pointer_spelltome_lob");
    create_lob(player, direction, target, part_id);
}

void pointer_null_pointer_spelltome_cast(Player* player, vec2 direction, vec2 target)
{
    i32 part_id = particle_get_id("pointer_null_pointer_spelltome_lob");
    create_lob(player, direction, target, part_id);
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

void shiv_primary(Player* player, vec2 direction, vec2 target)
{
    vec2 player_pos = player->entity->position;
    for (size_t i = 0; i < sizeof(sword_offsets) / sizeof(vec2); i++) {
        vec2 offset = sword_offsets[i];
        Projectile* proj = map_create_projectile(PROJECTILE_CREATE(
                    .position = vec2_add(player_pos, vec2_rotate(offset, vec2_radians(direction) - PI/2)),
                    .direction = direction,
                    .size = 0.5,
                    .speed = 20,
                    .lifetime = 0.15,
                    .facing = vec2_radians(direction),
                    .tex = texture_get_id("bullet"),
                    ));
        projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
    }
}

void wand_primary(Player* player, vec2 direction, vec2 target)
{
    Projectile* proj = map_create_projectile(PROJECTILE_CREATE(
                .position = player->entity->position,
                .direction = vec2_create(1, 0),
                .size = 0.5,
                .speed = 20,
                .lifetime = 0.15,
                .facing = vec2_radians(direction),
                .tex = texture_get_id("bullet"),
                ));
    projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);

    proj = map_create_projectile(PROJECTILE_CREATE(
                .position = player->entity->position,
                .direction = vec2_create(0, 1),
                .size = 0.5,
                .speed = 20,
                .lifetime = 0.15,
                .facing = vec2_radians(direction),
                .tex = texture_get_id("null_ptr"),
                ));
    projectile_set_flag(proj, PROJECTILE_FLAG_FRIENDLY, true);
}

void bow_primary(Player* player, vec2 direction, vec2 target)
{
}
