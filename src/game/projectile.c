#include "internal.h"

extern GameContext game_context;

void projectile_init(void)
{
    game_context.projectiles = list_create();
}

Projectile* projectile_create(vec3 position)
{
    Projectile* proj = st_malloc(sizeof(Projectile));
    proj->position = position;
    proj->direction = vec3_create(0, 0, 0);
    proj->speed = 1;
    proj->size = 0.5;
    proj->lifetime = 1;
    proj->flags = 0;
    list_append(game_context.projectiles, proj);
    return proj;
}

void projectile_update(Projectile* proj, f32 dt)
{
    proj->position = vec3_add(proj->position, vec3_scale(proj->direction, proj->speed * dt));
    proj->lifetime -= dt;
}

void projectile_set_flag(Projectile* proj, ProjectileFlagEnum flag, u32 val)
{
    proj->flags = (proj->flags & ~(1<<flag)) | (val<<flag);
}

bool projectile_get_flag(Projectile* proj, ProjectileFlagEnum flag)
{
    return (proj->flags >> flag) & 1;
}

void projectile_destroy(Projectile* proj)
{
    free(proj);
}

void projectile_cleanup(void)
{
    if (game_context.projectiles == NULL)
        return;
    for (i32 i = 0; i < game_context.projectiles->length; i++)
        free(list_get(game_context.projectiles, i));
    list_destroy(game_context.projectiles);
}
