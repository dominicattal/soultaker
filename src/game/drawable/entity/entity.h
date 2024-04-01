#ifndef ENTITY_H
#define ENTITY_H

#include <gtype.h>
#include <gvec.h>

typedef enum entitytype { PLAYER, PROJECTILE } entitytype;

typedef struct Entity {
    f32 speed, scale, lifetime;
    entitytype type;
    vec3f direction;
} Entity;

Entity* entity_create(entitytype type, f32 scale);
void entity_destroy(Entity* entity);

#endif