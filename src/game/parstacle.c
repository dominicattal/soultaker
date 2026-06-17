#include "../game.h"
#include <string.h>

extern GameContext game_context;

Parstacle* parstacle_create(vec2 position)
{
    Parstacle* parstacle = st_malloc(sizeof(Parstacle));
    parstacle->position = position;
    parstacle->size = 1.5;
    parstacle->uid = game_map_uid(parstacle, GAME_OBJ_PARSTACLE);
    return parstacle;
}

void parstacle_destroy(Parstacle* parstacle)
{
    game_free_uid(parstacle->uid);
    st_free(parstacle);
}

size_t parstacle_sizeof(void)
{
    Obstacle parstacle;
    return sizeof(parstacle.position)
         + sizeof(parstacle.size)
         + sizeof(parstacle.tex)
         + sizeof(parstacle.uid);
}

Parstacle* parstacle_read(char* buffer)
{
    Parstacle* parstacle = st_calloc(1, sizeof(Parstacle));
    memcpy(&parstacle->position, buffer, sizeof(parstacle->position));
    buffer += sizeof(parstacle->position);
    memcpy(&parstacle->size, buffer, sizeof(parstacle->size));
    buffer += sizeof(parstacle->size);
    memcpy(&parstacle->tex, buffer, sizeof(parstacle->tex));
    buffer += sizeof(parstacle->tex);
    memcpy(&parstacle->uid, buffer, sizeof(parstacle->uid));
    buffer += sizeof(parstacle->uid);
    return parstacle;
}

void parstacle_write(Parstacle* parstacle, char* buffer)
{
    memcpy(buffer, &parstacle->position, sizeof(parstacle->position));
    buffer += sizeof(parstacle->position);
    memcpy(buffer, &parstacle->size, sizeof(parstacle->size));
    buffer += sizeof(parstacle->size);
    memcpy(buffer, &parstacle->tex, sizeof(parstacle->tex));
    buffer += sizeof(parstacle->tex);
    memcpy(buffer, &parstacle->uid, sizeof(parstacle->uid));
    buffer += sizeof(parstacle->uid);
}
