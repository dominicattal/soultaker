#include "internal.h"
#include <assert.h>

GameContext game_context;

void game_update_vertex_data()
{
    #define FLOATS_PER_VERTEX 4
    if (game_context.entities->capacity > game_context.data_swap.entity_capacity) {
        game_context.data_swap.entity_capacity = game_context.entities->capacity;
        size_t size = FLOATS_PER_VERTEX * game_context.data_swap.entity_capacity * sizeof(GLfloat);
        if (game_context.entities == NULL)
            game_context.data_swap.entity_buffer = malloc(size);
        else
            game_context.data_swap.entity_buffer = realloc(game_context.data_swap.entity_buffer, size);
        assert(game_context.entities != NULL);
    }
    game_context.data_swap.entity_length = 0;
    for (i32 i = 0; i < game_context.entities->length; i++) {
        Entity* entity = list_get(game_context.entities, i);
        game_context.data_swap.entity_buffer[4*i]   = entity->position.x;
        game_context.data_swap.entity_buffer[4*i+1] = entity->position.y;
        game_context.data_swap.entity_buffer[4*i+2] = entity->position.z;
        game_context.data_swap.entity_buffer[4*i+3] = 1.0f;
        game_context.data_swap.entity_length += FLOATS_PER_VERTEX;
    }
    pthread_mutex_lock(&game_context.data_mutex);
    GameData tmp = game_context.data;
    game_context.data = game_context.data_swap;
    game_context.data_swap = tmp;
    pthread_mutex_unlock(&game_context.data_mutex);
}

void* game_update(void* vargp)
{
    f64 start;
    entity_init();
    while (!game_context.kill_thread)
    {
        start = get_time();
        game_update_vertex_data();
        game_context.dt = get_time() - start;
    }
    entity_cleanup();
    return NULL;
}

void game_init(void)
{
    pthread_mutex_init(&game_context.data_mutex, NULL);
    pthread_create(&game_context.thread_id, NULL, game_update, NULL);
    camera_init();
    game_render_init();
}

void game_cleanup(void)
{
    game_context.kill_thread = true;
    pthread_join(game_context.thread_id, NULL);
    pthread_mutex_destroy(&game_context.data_mutex);
    game_render_cleanup();
    camera_cleanup();
    free(game_context.data.entity_buffer);
    free(game_context.data_swap.entity_buffer);
}

f32 game_dt(void)
{
    return game_context.dt;
}
