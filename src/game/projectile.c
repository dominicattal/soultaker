#include "../game.h"

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
    proj->update = NULL;
    proj->destroy = NULL;
    proj->data = NULL;
    return proj;
}

void projectile_update(Projectile* proj, f32 dt)
{
    proj->position = vec2_add(proj->position, vec2_scale(proj->direction, proj->speed * dt));
    proj->lifetime -= dt;
    if (proj->update != NULL)
        proj->update(&game_api, proj, dt);
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
    if (proj->destroy != NULL)
        proj->destroy(&game_api, proj);
    st_free(proj);
}
