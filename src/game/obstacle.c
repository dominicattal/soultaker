#include "../game.h"
#include <string.h>

extern GameContext game_context;

Obstacle* obstacle_create(vec2 position)
{
    Obstacle* obstacle = st_malloc(sizeof(Obstacle));
    obstacle->position = position;
    obstacle->size = 1.0f;
    obstacle->uid = game_map_uid(obstacle, GAME_OBJ_OBSTACLE);

    if (game_context.hosting)
        host_create_game_obj(obstacle->uid);

    list_i32_append(game_context.updated_uids, obstacle->uid);

    return obstacle;
}

void obstacle_destroy(Obstacle* obstacle)
{
    game_free_uid(obstacle->uid);
    st_free(obstacle);
}

size_t obstacle_sizeof(void)
{
    Obstacle obstacle;
    return sizeof(obstacle.position)
         + sizeof(obstacle.size)
         + sizeof(obstacle.tex)
         + sizeof(obstacle.uid);
}

Obstacle* obstacle_read(char* buffer)
{
    Obstacle* obstacle = st_calloc(1, sizeof(Obstacle));
    memcpy(&obstacle->position, buffer, sizeof(obstacle->position));
    buffer += sizeof(obstacle->position);
    memcpy(&obstacle->size, buffer, sizeof(obstacle->size));
    buffer += sizeof(obstacle->size);
    memcpy(&obstacle->tex, buffer, sizeof(obstacle->tex));
    buffer += sizeof(obstacle->tex);
    memcpy(&obstacle->uid, buffer, sizeof(obstacle->uid));
    buffer += sizeof(obstacle->uid);
    return obstacle;
}

void obstacle_write(Obstacle* obstacle, char* buffer)
{
    memcpy(buffer, &obstacle->position, sizeof(obstacle->position));
    buffer += sizeof(obstacle->position);
    memcpy(buffer, &obstacle->size, sizeof(obstacle->size));
    buffer += sizeof(obstacle->size);
    memcpy(buffer, &obstacle->tex, sizeof(obstacle->tex));
    buffer += sizeof(obstacle->tex);
    memcpy(buffer, &obstacle->uid, sizeof(obstacle->uid));
    buffer += sizeof(obstacle->uid);
}
