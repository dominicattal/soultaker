#include "internal.h"
#include "../renderer.h"

GameContext game_context;

static void update_entity_vertex_data()
{
    #define FLOATS_PER_VERTEX 8
    if (FLOATS_PER_VERTEX * game_context.entities->capacity > game_context.data_swap.entity_capacity) {
        game_context.data_swap.entity_capacity = game_context.entities->capacity;
        size_t size = FLOATS_PER_VERTEX * game_context.data_swap.entity_capacity * sizeof(GLfloat);
        if (game_context.data_swap.entity_buffer == NULL)
            game_context.data_swap.entity_buffer = malloc(size);
        else
            game_context.data_swap.entity_buffer = realloc(game_context.data_swap.entity_buffer, size);
        assert(game_context.entities != NULL);
    }
    #undef FLOATS_PER_VERTEX
    game_context.data_swap.entity_length = 0;
    f32 u, v, w, h;
    i32 location;
    texture_info(TEX_KNIGHT, &u, &v, &w, &h, &location);
    #define V game_context.data_swap.entity_buffer[game_context.data_swap.entity_length++]
    for (i32 i = 0; i < game_context.entities->length; i++) {
        Entity* entity = list_get(game_context.entities, i);
        V = entity->position.x;
        V = entity->position.y;
        V = entity->position.z;
        V = u;
        V = v;
        V = w;
        V = h;
        V = location;
    }
    #undef V
}

static void update_tile_vertex_data()
{
    #define FLOATS_PER_VERTEX 7
    if (FLOATS_PER_VERTEX * game_context.tiles->capacity > game_context.data_swap.tile_capacity) {
        game_context.data_swap.tile_capacity = game_context.tiles->capacity;
        size_t size = FLOATS_PER_VERTEX * game_context.data_swap.tile_capacity * sizeof(GLfloat);
        if (game_context.data_swap.tile_buffer == NULL)
            game_context.data_swap.tile_buffer = malloc(size);
        else
            game_context.data_swap.tile_buffer = realloc(game_context.data_swap.tile_buffer, size);
        assert(game_context.tiles != NULL);
    }
    #undef FLOATS_PER_VERTEX
    game_context.data_swap.tile_length = 0;
    f32 u, v, w, h;
    i32 location;
    #define V game_context.data_swap.tile_buffer[game_context.data_swap.tile_length++]
    for (i32 i = 0; i < game_context.tiles->length; i++) {
        Tile* tile = list_get(game_context.tiles, i);
        texture_info(tile->tex, &u, &v, &w, &h, &location);
        V = tile->position.x;
        V = tile->position.y;
        V = u;
        V = v;
        V = w;
        V = h;
        V = location;
    }
    #undef V
}

static void update_wall_vertex_data(void)
{
    #define FLOATS_PER_VERTEX (6 * 6 * 5)
    if (FLOATS_PER_VERTEX * game_context.walls->capacity > game_context.data_swap.wall_capacity) {
        game_context.data_swap.wall_capacity = game_context.walls->capacity;
        size_t size = FLOATS_PER_VERTEX * game_context.data_swap.wall_capacity * sizeof(GLfloat);
        if (game_context.data_swap.wall_buffer == NULL)
            game_context.data_swap.wall_buffer = malloc(size);
        else
            game_context.data_swap.wall_buffer = realloc(game_context.data_swap.wall_buffer, size);
        assert(game_context.walls != NULL);
    }
    #undef FLOATS_PER_VERTEX
    game_context.data_swap.wall_length = 0;
    static f32 dx[] = {0, 0, 0, 0, 1, 1, 1, 1};
    static f32 dy[] = {0, 0, 1, 1, 0, 0, 1, 1};
    static f32 dz[] = {0, 1, 0, 1, 0, 1, 0, 1};
    static f32 tx[] = {0, 0, 1, 1};
    static f32 ty[] = {0, 1, 0, 1};
    static i32 side_order[5][4] = {
        {4, 5, 7, 6}, // +x
        {1, 3, 7, 5}, // +z
        {0, 2, 3, 1}, // -x
        {0, 4, 6, 2}, // -z
        {2, 6, 7, 3}  // +y
    };
    static i32 winding[] = {0, 1, 2, 0, 2, 3};
    f32 u, v, w, h;
    i32 location;
    i32 idx;
    #define V game_context.data_swap.wall_buffer[game_context.data_swap.wall_length++]
    for (i32 i = 0; i < game_context.walls->length; i++) {
        Wall* wall = list_get(game_context.walls, i);
        texture_info(wall->side_tex, &u, &v, &w, &h, &location);
        for (i32 side = 0; side < 4; side++) {
            for (i32 j = 0; j < 6; j++) {
                idx = side_order[side][winding[j]];
                V = wall->position.x + dx[idx]; 
                V = wall->height * dy[idx];
                V = wall->position.y + dz[idx];
                V = u + dx[winding[j]] * w;
                V = v + dy[winding[j]] * h;
                V = location;
            }
        }
        texture_info(wall->top_tex, &u, &v, &w, &h, &location);
        for (i32 j = 0; j < 6; j++) {
            idx = side_order[4][winding[j]];
            V = wall->position.x + dx[idx]; 
            V = wall->height * dy[idx];
            V = wall->position.y + dz[idx];
            V = u + tx[winding[j]] * w;
            V = v + ty[winding[j]] * h;
            V = location;
        }
    }
    #undef V
}

void game_update_vertex_data()
{
    update_entity_vertex_data();
    update_tile_vertex_data();
    update_wall_vertex_data();
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
    wall_init();
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
    wall_cleanup();
    free(game_context.data.entity_buffer);
    free(game_context.data_swap.entity_buffer);
}

f32 game_dt(void)
{
    return game_context.dt;
}
