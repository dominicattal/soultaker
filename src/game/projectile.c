#include "../game.h"
#include <string.h>

extern GameContext game_context;

Projectile* projectile_create(vec2 position)
{
    Projectile* proj = st_malloc(sizeof(Projectile));
    proj->position = position;
    proj->direction = vec2_create(0, 0);
    proj->elevation = 0.5;
    proj->facing = 0;
    proj->rotation = 0;
    proj->speed = 1;
    proj->size = 0.5;
    proj->lifetime = 1;
    proj->flags = 0;
    proj->pierce_timer = 0;
    proj->owner_uid = -1;
    proj->update = NULL;
    proj->destroy = NULL;
    proj->data = NULL;
    proj->uid = game_map_uid(proj, GAME_OBJ_PROJECTILE);
    return proj;
}

void projectile_update(Projectile* proj, f32 dt)
{
    proj->position = vec2_add(proj->position, vec2_scale(proj->direction, proj->speed * dt));
    if (!projectile_get_flag(proj, PROJECTILE_FLAG_IGNORE_LIFETIME))
        proj->lifetime -= dt;
    if (projectile_get_flag(proj, PROJECTILE_FLAG_PIERCE) && proj->pierce_timer >= 0)
        proj->pierce_timer -= dt;
    if (proj->update != NULL)
        proj->update(proj, dt);
}

void projectile_set_flag(Projectile* proj, ProjectileFlagEnum flag, bool val)
{
    proj->flags = (proj->flags & ~(1<<flag)) | (val<<flag);
}

bool projectile_get_flag(Projectile* proj, ProjectileFlagEnum flag)
{
    return (proj->flags >> flag) & 1;
}

void projectile_destroy(Projectile* proj)
{
    game_free_uid(proj->uid);
    if (proj->destroy != NULL)
        proj->destroy(proj);
    if (projectile_get_flag(proj, PROJECTILE_FLAG_AUTO_FREE_DATA))
        st_free(proj->data);
    st_free(proj);
}

size_t projectile_sizeof(void)
{
    Projectile proj;
    return sizeof(proj.position)
         + sizeof(proj.direction)
         + sizeof(proj.elevation)
         + sizeof(proj.facing)
         + sizeof(proj.rotation)
         + sizeof(proj.speed)
         + sizeof(proj.size)
         + sizeof(proj.lifetime)
         + sizeof(proj.flags)
         + sizeof(proj.tex)
         + sizeof(proj.uid);
}

void projectile_write(Projectile* proj, char* buffer)
{
    memcpy(buffer, &proj->position, sizeof(proj->position));
    buffer += sizeof(proj->position);
    memcpy(buffer, &proj->direction, sizeof(proj->direction));
    buffer += sizeof(proj->direction);
    memcpy(buffer, &proj->elevation, sizeof(proj->elevation));
    buffer += sizeof(proj->elevation);
    memcpy(buffer, &proj->facing, sizeof(proj->facing));
    buffer += sizeof(proj->facing);
    memcpy(buffer, &proj->rotation, sizeof(proj->rotation));
    buffer += sizeof(proj->rotation);
    memcpy(buffer, &proj->speed, sizeof(proj->speed));
    buffer += sizeof(proj->speed);
    memcpy(buffer, &proj->size, sizeof(proj->size));
    buffer += sizeof(proj->size);
    memcpy(buffer, &proj->lifetime, sizeof(proj->lifetime));
    buffer += sizeof(proj->lifetime);
    memcpy(buffer, &proj->flags, sizeof(proj->flags));
    buffer += sizeof(proj->flags);
    memcpy(buffer, &proj->tex, sizeof(proj->tex));
    buffer += sizeof(proj->tex);
    memcpy(buffer, &proj->uid, sizeof(proj->uid));
    buffer += sizeof(proj->uid);
}

Projectile* projectile_read(char* buffer)
{
    Projectile* proj = st_calloc(1, sizeof(Projectile));
    memcpy(&proj->position, buffer, sizeof(proj->position));
    buffer += sizeof(proj->position);
    memcpy(&proj->direction, buffer, sizeof(proj->direction));
    buffer += sizeof(proj->direction);
    memcpy(&proj->elevation, buffer, sizeof(proj->elevation));
    buffer += sizeof(proj->elevation);
    memcpy(&proj->facing, buffer, sizeof(proj->facing));
    buffer += sizeof(proj->facing);
    memcpy(&proj->rotation, buffer, sizeof(proj->rotation));
    buffer += sizeof(proj->rotation);
    memcpy(&proj->speed, buffer, sizeof(proj->speed));
    buffer += sizeof(proj->speed);
    memcpy(&proj->size, buffer, sizeof(proj->size));
    buffer += sizeof(proj->size);
    memcpy(&proj->lifetime, buffer, sizeof(proj->lifetime));
    buffer += sizeof(proj->lifetime);
    memcpy(&proj->flags, buffer, sizeof(proj->flags));
    buffer += sizeof(proj->flags);
    memcpy(&proj->tex, buffer, sizeof(proj->tex));
    buffer += sizeof(proj->tex);
    memcpy(&proj->uid, buffer, sizeof(proj->uid));
    buffer += sizeof(proj->uid);
    return proj;
}
