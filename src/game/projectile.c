#include "internal.h"

extern GameContext game_context;

static void default_update_function(Projectile*, f32) {}
static void default_destroy_function(Projectile*) {}

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
    proj->update = default_update_function;
    proj->destroy = default_destroy_function;
    return proj;
}

void projectile_update(Projectile* proj, f32 dt)
{
    proj->position = vec2_add(proj->position, vec2_scale(proj->direction, proj->speed * dt));
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
