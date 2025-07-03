#include "internal.h"

extern GameContext game_context;

void projectile_init(void)
{
    game_context.projectiles = list_create();
}

void projectile_clear(void)
{
    if (game_context.projectiles == NULL)
        return;
    while (game_context.projectiles->length > 0)
        projectile_destroy(list_remove(game_context.projectiles, 0));
}

static void default_update_function(Projectile*, f32) {}
static void default_destroy_function(Projectile*) {}

Projectile* projectile_create(vec3 position)
{
    Projectile* proj = st_malloc(sizeof(Projectile));
    proj->position = position;
    proj->position.y = 0.5f;
    proj->direction = vec3_create(0, 0, 0);
    proj->facing = 0;
    proj->rotation = 0;
    proj->speed = 1;
    proj->size = 0.5;
    proj->lifetime = 1;
    proj->flags = 0;
    proj->update = default_update_function;
    proj->destroy = default_destroy_function;
    list_append(game_context.projectiles, proj);
    return proj;
}

void projectile_update(Projectile* proj, f32 dt)
{
    proj->position = vec3_add(proj->position, vec3_scale(proj->direction, proj->speed * dt));
    proj->lifetime -= dt;
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
    st_free(proj);
}

void projectile_cleanup(void)
{
    if (game_context.projectiles == NULL)
        return;
    for (i32 i = 0; i < game_context.projectiles->length; i++)
        projectile_destroy(list_get(game_context.projectiles, i));
    list_destroy(game_context.projectiles);
}
