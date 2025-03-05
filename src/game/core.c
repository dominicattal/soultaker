#include "internal.h"
#include "../renderer.h"

GameContext game_context;

static void update_entity_vertex_data()
{
    #define FLOATS_PER_VERTEX 4
    if (FLOATS_PER_VERTEX * game_context.entities->capacity > game_context.data_swap.entity_capacity) {
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
    #undef FLOATS_PER_VERTEX
}

static void update_tile_vertex_data()
{
    #define FLOATS_PER_VERTEX 7
    if (FLOATS_PER_VERTEX * game_context.tiles->capacity > game_context.data_swap.tile_capacity) {
        game_context.data_swap.tile_capacity = game_context.tiles->capacity;
        size_t size = FLOATS_PER_VERTEX * game_context.data_swap.tile_capacity * sizeof(GLfloat);
        if (game_context.tiles == NULL)
            game_context.data_swap.tile_buffer = malloc(size);
        else
            game_context.data_swap.tile_buffer = realloc(game_context.data_swap.tile_buffer, size);
        assert(game_context.tiles != NULL);
    }
    game_context.data_swap.tile_length = 0;
    f32 u, v, w, h;
    i32 location;
    for (i32 i = 0; i < game_context.tiles->length; i++) {
        Tile* tile = list_get(game_context.tiles, i);
        texture_info(tile->tex, &u, &v, &w, &h, &location);
        game_context.data_swap.tile_buffer[7*i]   = tile->position.x;
        game_context.data_swap.tile_buffer[7*i+1] = tile->position.y;
        game_context.data_swap.tile_buffer[7*i+2] = u;
        game_context.data_swap.tile_buffer[7*i+3] = v;
        game_context.data_swap.tile_buffer[7*i+4] = w;
        game_context.data_swap.tile_buffer[7*i+5] = h;
        game_context.data_swap.tile_buffer[7*i+6] = location;
        game_context.data_swap.tile_length += FLOATS_PER_VERTEX;
    }
    #undef FLOATS_PER_VERTEX
}

void game_update_vertex_data()
{
    update_entity_vertex_data();
    update_tile_vertex_data();
    pthread_mutex_lock(&game_context.data_mutex);
    GameData tmp = game_context.data;
    game_context.data = game_context.data_swap;
    game_context.data_swap = tmp;
    pthread_mutex_unlock(&game_context.data_mutex);
}

void* game_update(void* vargp)
{
    f64 start;
    while (!game_context.kill_thread)
    {
        start = get_time();
        game_update_vertex_data();
        game_context.dt = get_time() - start;
    }
    return NULL;
}

void game_init(void)
{
    tile_init();
    entity_init();
    camera_init();
    game_render_init();
    pthread_mutex_init(&game_context.data_mutex, NULL);
    pthread_create(&game_context.thread_id, NULL, game_update, NULL);
}

void game_cleanup(void)
{
    game_context.kill_thread = true;
    pthread_join(game_context.thread_id, NULL);
    pthread_mutex_destroy(&game_context.data_mutex);
    game_render_cleanup();
    camera_cleanup();
    entity_cleanup();
    tile_cleanup();
    free(game_context.data.entity_buffer);
    free(game_context.data_swap.entity_buffer);
}

f32 game_dt(void)
{
    return game_context.dt;
}
