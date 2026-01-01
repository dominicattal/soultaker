#include "internal.h"
#include "../renderer.h"
#include "../window.h"

#define NEAR_CLIP_DISTANCE  0.001f
#define FAR_CLIP_DISTANCE   1000.0f

#define TILE_VERTEX_LENGTH              8
#define WALL_VERTEX_LENGTH              (8 * 6 * 5)
#define ENTITY_FLOATS_PER_VERTEX        13
#define PROJECTILE_FLOATS_PER_VERTEX    11
#define OBSTACLE_FLOATS_PER_VERTEX      8
#define PARTICLE_FLOATS_PER_VERTEX      7
#define PARJICLE_FLOATS_PER_VERTEX      8
#define MAP_CIRCLE_FLOATS_PER_VERTEX    6
#define MAP_SQUARE_FLOATS_PER_VERTEX    7
#define SHADOW_FLOATS_PER_VERTEX        4

typedef enum {
    VAO_QUAD,
    VAO_TILE,
    VAO_WALL,
    VAO_TILE_MINIMAP,
    VAO_WALL_MINIMAP,
    NUM_VAOS
} GameVAOEnum;

typedef enum {
    VBO_QUAD,
    VBO_TILE,
    VBO_WALL,
    SSBO_ENTITY,
    SSBO_ENTITY_SHADOW,
    SSBO_ENTITY_MINIMAP,
    SSBO_PROJECTILE,
    SSBO_OBSTACLE,
    SSBO_OBSTACLE_MINIMAP,
    SSBO_PARSTACLE,
    SSBO_PARTICLE,
    SSBO_PARJICLE,
    VBO_TILE_MINIMAP,
    VBO_WALL_MINIMAP,
    NUM_BUFFERS
} GameBufferEnum;

typedef struct {
    i32 length, capacity;
    GLfloat* buffer;
    bool update;
} VertexBuffer;

typedef struct {
    VertexBuffer buffers[NUM_BUFFERS];
} RenderData;

typedef struct {
    f32 yaw, pitch, zoom, fov;
    f32 view[16], proj[16];
    vec3 position, facing, right, up;
    vec2 target;
} RenderCamera;

typedef struct {
    GLenum target;
    GLenum usage;
    GLuint name;
    i32 length;
    i32 capacity;
    bool update;
} Buffer;

typedef struct {
    RenderData* data;
    RenderData* data_swap;
    RenderCamera camera;
    GLuint fbo, shadow_fbo, rbo;
    GLuint minimap_fbo;
    GLuint game_time_ubo;
    GLuint matrices_ubo;
    GLuint minimap_ubo;
    GLuint vaos[NUM_VAOS];
    Buffer buffers[NUM_BUFFERS];
    pthread_mutex_t mutex;
} RenderContext;

static RenderContext render_context;
extern GameContext game_context;

static void view(f32 m[16], vec3 r, vec3 u, vec3 f, vec3 p)
{
    f32 k1 = p.x * r.x + p.y * r.y + p.z * r.z;
    f32 k2 = p.x * u.x + p.y * u.y + p.z * u.z;
    f32 k3 = p.x * f.x + p.y * f.y + p.z * f.z;
    m[0]  = r.x; m[1]  = u.x; m[2]  = f.x; m[3]  = 0.0f;
    m[4]  = r.y; m[5]  = u.y; m[6]  = f.y; m[7]  = 0.0f;
    m[8]  = r.z; m[9]  = u.z; m[10] = f.z; m[11] = 0.0f;
    m[12] = -k1; m[13] = -k2; m[14] = -k3; m[15] = 1.0f;
}

static void orthographic(f32 m[16], f32 ar, f32 zoom)
{
    f32 r, l, t, b, f, n;
    b = -(t = zoom);
    l = -(r = ar * zoom);
    f = FAR_CLIP_DISTANCE;
    n = NEAR_CLIP_DISTANCE;
    f32 val1, val2, val3, val4, val5, val6;
    val1 = 2 / (r - l);
    val2 = 2 / (t - b);
    val3 = 2 / (f - n);
    val4 = -(r + l) / (r - l);
    val5 = -(t + b) / (t - b);
    val6 = -(f + n) / (f - n);
    m[0]  = val1; m[1]  = 0.0f; m[2]  = 0.0f; m[3]  = 0.0f;
    m[4]  = 0.0f; m[5]  = val2; m[6]  = 0.0f; m[7]  = 0.0f;
    m[8]  = 0.0f; m[9]  = 0.0f; m[10] = val3; m[11] = 0.0f;
    m[12] = val4; m[13] = val5; m[14] = val6; m[15] = 1.0f;
}

static void update_view_matrix(void)
{
    RenderCamera* cam = &render_context.camera;
    vec2 pos = cam->target;
    view(cam->view, cam->right, cam->up, cam->facing, cam->position);
    glBindBuffer(GL_UNIFORM_BUFFER, render_context.matrices_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 16 * sizeof(GLfloat), &cam->view[0]);
    glBufferSubData(GL_UNIFORM_BUFFER, 33 * sizeof(GLfloat), sizeof(GLfloat), &cam->pitch);
    glBufferSubData(GL_UNIFORM_BUFFER, 34 * sizeof(GLfloat), sizeof(GLfloat), &cam->yaw);
    glBindBuffer(GL_UNIFORM_BUFFER, render_context.minimap_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLfloat), &pos.x);
    glBufferSubData(GL_UNIFORM_BUFFER, 1 * sizeof(GLfloat), sizeof(GLfloat), &pos.z);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(GLfloat), sizeof(GLfloat), &cam->yaw);
}

static void update_proj_matrix(void)
{
    RenderCamera* cam = &render_context.camera;
    f32 ar = window_aspect_ratio();
    orthographic(cam->proj, ar, cam->zoom);
    glBindBuffer(GL_UNIFORM_BUFFER, render_context.matrices_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 16 * sizeof(GLfloat), 16 * sizeof(GLfloat), &cam->proj[0]);
    glBufferSubData(GL_UNIFORM_BUFFER, 32 * sizeof(GLfloat), sizeof(GLfloat), &cam->zoom);
    glBindBuffer(GL_UNIFORM_BUFFER, render_context.minimap_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 3 * sizeof(GLfloat), sizeof(GLfloat), &cam->zoom);
}

static void copy_camera(void)
{
    Camera* game_cam = &game_context.camera;
    RenderCamera* render_cam = &render_context.camera;
    render_cam->yaw         = game_cam->yaw;
    render_cam->pitch       = game_cam->pitch;
    render_cam->zoom        = game_cam->zoom;
    render_cam->fov         = game_cam->fov;
    render_cam->position    = game_cam->position;
    render_cam->facing      = game_cam->facing;
    render_cam->right       = game_cam->right;
    render_cam->up          = game_cam->up;
    render_cam->target      = game_cam->target;
}

static VertexBuffer* get_vertex_buffer(GameBufferEnum type)
{
    return &render_context.data->buffers[type];
}

static VertexBuffer* get_vertex_buffer_swap(GameBufferEnum type)
{
    return &render_context.data_swap->buffers[type];
}

static Buffer* get_buffer(GameBufferEnum type)
{
    return &render_context.buffers[type];
}

static void resize_vertex_buffer(VertexBuffer* vb, i32 capacity)
{
    size_t size;
    if (vb->capacity > capacity)
        return;
    vb->capacity = capacity;
    if (vb->capacity == 0) {
        st_free(vb->buffer);
        vb->buffer = NULL;
        return;
    }
    size = capacity * sizeof(GLfloat);
    if (vb->buffer == NULL)
        vb->buffer = st_malloc(size);
    else
        vb->buffer = st_realloc(vb->buffer, size);
}

static void update_game_time(void)
{
    glBindBuffer(GL_UNIFORM_BUFFER, render_context.game_time_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLdouble), &game_context.time);
}
    
static void update_entity_vertex_data(Map* map)
{
    VertexBuffer* vb;
    VertexBuffer* shadow_vb;
    VertexBuffer* map_vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 i, j;
    Entity* entity;
    List* entities;

    entities = map_list_entities(map);
    vb = get_vertex_buffer_swap(SSBO_ENTITY);
    shadow_vb = get_vertex_buffer_swap(SSBO_ENTITY_SHADOW);
    map_vb = get_vertex_buffer_swap(SSBO_ENTITY_MINIMAP);
    resize_vertex_buffer(vb, ENTITY_FLOATS_PER_VERTEX * entities->capacity);
    resize_vertex_buffer(shadow_vb, SHADOW_FLOATS_PER_VERTEX * entities->capacity);
    resize_vertex_buffer(map_vb, MAP_CIRCLE_FLOATS_PER_VERTEX * entities->capacity);

    for (i = j = 0; i < entities->length; i++) {
        entity = list_get(entities, i);
        if (map_fog_contains(map, entity->position))
            continue;
        texture_info(entity_get_texture(entity), &location, &u, &v, &w, &h, &pivot, &stretch);
        vb->buffer[j++] = entity->position.x;
        vb->buffer[j++] = entity->elevation;
        vb->buffer[j++] = entity->position.z;
        vb->buffer[j++] = entity->size;
        vb->buffer[j++] = u;
        vb->buffer[j++] = v;
        vb->buffer[j++] = w;
        vb->buffer[j++] = h;
        vb->buffer[j++] = location;
        vb->buffer[j++] = pivot.x;
        vb->buffer[j++] = pivot.y;
        vb->buffer[j++] = stretch.x;
        vb->buffer[j++] = stretch.y;
    }
    vb->length = j;

    for (i = j = 0; i < entities->length; i++) {
        entity = list_get(entities, i);
        if (map_fog_contains(map, entity->position))
            continue;
        map_vb->buffer[j++] = entity->position.x;
        map_vb->buffer[j++] = entity->position.z;
        map_vb->buffer[j++] = entity->hitbox_radius;
        if (entity_get_flag(entity, ENTITY_FLAG_FRIENDLY)) {
            map_vb->buffer[j++] = 0.0f;
            map_vb->buffer[j++] = 1.0f;
            map_vb->buffer[j++] = 0.0f;
        } else {
            map_vb->buffer[j++] = 1.0f;
            map_vb->buffer[j++] = 0.0f;
            map_vb->buffer[j++] = 0.0f;
        }
    }
    map_vb->length = j;
    
    for (i = j = 0; i < entities->length; i++) {
        entity = list_get(entities, i);
        if (map_fog_contains(map, entity->position))
            continue;
        shadow_vb->buffer[j++] = entity->position.x;
        shadow_vb->buffer[j++] = entity->elevation;
        shadow_vb->buffer[j++] = entity->position.z;
        shadow_vb->buffer[j++] = entity->size;
    }
    shadow_vb->length = j;
}

static void update_projectile_vertex_data(Map* map)
{
    VertexBuffer* vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location, tex;
    i32 i, j;
    Projectile* projectile;
    bool rotate_tex;

    vb = get_vertex_buffer_swap(SSBO_PROJECTILE);
    resize_vertex_buffer(vb, PROJECTILE_FLOATS_PER_VERTEX * game_context.projectiles->capacity);

    for (i = j = 0; i < game_context.projectiles->length; i++) {
        projectile = list_get(game_context.projectiles, i);
        if (map_fog_contains(map, projectile->position))
            continue;
        tex = projectile->tex;
        texture_info(tex, &location, &u, &v, &w, &h, &pivot, &stretch);
        rotate_tex = projectile_get_flag(projectile, PROJECTILE_FLAG_TEX_ROTATION);
        vb->buffer[j++] = projectile->position.x;
        vb->buffer[j++] = projectile->elevation;
        vb->buffer[j++] = projectile->position.z;
        // encode texture rotation as negative num
        vb->buffer[j++] = projectile->size * (rotate_tex ? 1 : -1);
        vb->buffer[j++] = projectile->facing;
        vb->buffer[j++] = projectile->rotation;
        vb->buffer[j++] = u;
        vb->buffer[j++] = v;
        vb->buffer[j++] = w;
        vb->buffer[j++] = h;
        vb->buffer[j++] = location; 
    }

    vb->length = j;
}

static void update_tile_vertex_data(Map* map)
{
    VertexBuffer* vb;
    VertexBuffer* map_vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 i, j;
    bool animate_horizontal_pos, animate_vertical_pos;
    bool animate_horizontal_neg, animate_vertical_neg;
    Tile* tile;
    List* tiles;

    tiles = map_list_tiles(map);
    vb = get_vertex_buffer_swap(VBO_TILE);
    map_vb = get_vertex_buffer_swap(VBO_TILE_MINIMAP);
    resize_vertex_buffer(vb, TILE_VERTEX_LENGTH * tiles->capacity);
    resize_vertex_buffer(map_vb, MAP_SQUARE_FLOATS_PER_VERTEX * tiles->capacity);

    for (i = j = 0; i < tiles->length; i++) {
        tile = list_get(tiles, i);
        if (!tile_get_flag(tile, TILE_FLAG_ACTIVE))
            continue;
        if (map_fog_contains_tile(map, tile))
            continue;
        animate_horizontal_pos = tile_get_flag(tile, TILE_FLAG_ANIMATE_HORIZONTAL_POS);
        animate_vertical_pos = tile_get_flag(tile, TILE_FLAG_ANIMATE_VERTICAL_POS);
        animate_horizontal_neg = tile_get_flag(tile, TILE_FLAG_ANIMATE_HORIZONTAL_NEG);
        animate_vertical_neg = tile_get_flag(tile, TILE_FLAG_ANIMATE_VERTICAL_NEG);
        texture_info(tile->tex, &location, &u, &v, &w, &h, &pivot, &stretch);
        vb->buffer[j++] = tile->position.x;
        vb->buffer[j++] = tile->position.z;
        vb->buffer[j++] = u;
        vb->buffer[j++] = v;
        vb->buffer[j++] = w;
        vb->buffer[j++] = h;
        vb->buffer[j++] = location;
        vb->buffer[j++] = animate_horizontal_pos + (animate_horizontal_neg<<1)
            + (animate_vertical_pos<<2) + (animate_vertical_neg<<3);
    }
    vb->length = j;

    for (i = j = 0; i < tiles->length; i++) {
        tile = list_get(tiles, i);
        if (!tile_get_flag(tile, TILE_FLAG_ACTIVE))
            continue;
        if (map_fog_contains_tile(map, tile))
            continue;
        map_vb->buffer[j++] = tile->position.x;
        map_vb->buffer[j++] = tile->position.z;
        map_vb->buffer[j++] = 1.0f; // tile width and height
        map_vb->buffer[j++] = 1.0f; // always 1
        map_vb->buffer[j++] = (((tile->minimap_color)>>16)&0xFF) / 255.0f;
        map_vb->buffer[j++] = (((tile->minimap_color)>>8)&0xFF) / 255.0f;
        map_vb->buffer[j++] = (tile->minimap_color&0xFF) / 255.0f;
    }
    map_vb->length = j;
}

static void update_wall_vertex_data(Map* map)
{
    VertexBuffer* vb;
    VertexBuffer* map_vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 idx;
    i32 i, j, k;
    Wall* wall;
    List* walls;

    walls = map_list_walls(map);
    vb = get_vertex_buffer_swap(VBO_WALL);
    map_vb = get_vertex_buffer_swap(VBO_WALL_MINIMAP);
    resize_vertex_buffer(vb, WALL_VERTEX_LENGTH * walls->capacity);
    resize_vertex_buffer(map_vb, MAP_SQUARE_FLOATS_PER_VERTEX * walls->capacity);

    static f32 dx[] = {0, 0, 0, 0, 1, 1, 1, 1};
    static f32 dy[] = {0, 0, 1, 1, 0, 0, 1, 1};
    static f32 dz[] = {0, 1, 0, 1, 0, 1, 0, 1};
    static f32 tx[] = {0, 1, 1, 0};
    static f32 ty[] = {0, 0, 1, 1};
    static i32 side_order[5][4] = {
        {4, 5, 7, 6}, // +x
        {3, 7, 5, 1}, // +z
        {1, 0, 2, 3}, // -x
        {0, 4, 6, 2}, // -z
        {2, 6, 7, 3}  // +y
    };
    static i32 winding[] = {0, 1, 2, 0, 2, 3};

    for (i = j = 0; i < walls->length; i++) {
        wall = list_get(walls, i);
        if (!wall_get_flag(wall, WALL_FLAG_ACTIVE))
            continue;
        if (map_fog_contains_wall(map, wall))
            continue;
        texture_info(wall->side_tex, &location, &u, &v, &w, &h, &pivot, &stretch);
        for (i32 side = 0; side < 4; side++) {
            for (k = 0; k < 6; k++) {
                idx = side_order[side][winding[k]];
                // nvidia rounding error? epsilon fixes kind of
                vb->buffer[j++] = wall->position.x + dx[idx] * (wall->size.x + 0.001); 
                vb->buffer[j++] = wall->height * dy[idx];
                vb->buffer[j++] = wall->position.z + dz[idx] * (wall->size.y + 0.001);
                vb->buffer[j++] = u + tx[winding[k]] * w;
                vb->buffer[j++] = v + ty[winding[k]] * h;
                vb->buffer[j++] = location;
                vb->buffer[j++] = wall->position.x + wall->size.x / 2;
                vb->buffer[j++] = wall->position.z + wall->size.y / 2;
            }
        }
        texture_info(wall->top_tex, &location, &u, &v, &w, &h, &pivot, &stretch);
        for (k = 0; k < 6; k++) {
            idx = side_order[4][winding[k]];
            vb->buffer[j++] = wall->position.x + dx[idx] * wall->size.x; 
            vb->buffer[j++] = wall->height * dy[idx];
            vb->buffer[j++] = wall->position.z + dz[idx] * wall->size.y;
            vb->buffer[j++] = u + tx[winding[k]] * w;
            vb->buffer[j++] = v + ty[winding[k]] * h;
            vb->buffer[j++] = location;
            vb->buffer[j++] = wall->position.x + wall->size.x / 2;
            vb->buffer[j++] = wall->position.z + wall->size.y / 2;
        }
    }
    vb->length = j;

    for (i = j = 0; i < walls->length; i++) {
        wall = list_get(walls, i);
        if (!wall_get_flag(wall, WALL_FLAG_ACTIVE))
            continue;
        if (map_fog_contains_wall(map, wall))
            continue;
        map_vb->buffer[j++] = wall->position.x;
        map_vb->buffer[j++] = wall->position.z;
        map_vb->buffer[j++] = wall->size.x;
        map_vb->buffer[j++] = wall->size.z;
        map_vb->buffer[j++] = (((wall->minimap_color)>>16)&0xFF) / 255.0f;
        map_vb->buffer[j++] = (((wall->minimap_color)>>8)&0xFF) / 255.0f;
        map_vb->buffer[j++] = (wall->minimap_color&0xFF) / 255.0f;
    }
    map_vb->length = j;
}

static void update_parstacle_vertex_data(Map* map)
{
    VertexBuffer* vb;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 i, j;
    Parstacle* parstacle;

    vb = get_vertex_buffer_swap(SSBO_PARSTACLE);
    resize_vertex_buffer(vb, OBSTACLE_FLOATS_PER_VERTEX * game_context.parstacles->capacity);

    i32 tex = texture_get_id("bush");
    texture_info(tex, &location, &u, &v, &w, &h, &pivot, &stretch);
    
    for (i = j = 0; i < game_context.parstacles->length; i++) {
        parstacle = list_get(game_context.parstacles, i);
        if (map_fog_contains(map, parstacle->position))
            continue;
        vb->buffer[j++] = parstacle->position.x;
        vb->buffer[j++] = parstacle->position.z;
        vb->buffer[j++] = parstacle->size;
        vb->buffer[j++] = u;
        vb->buffer[j++] = v;
        vb->buffer[j++] = w;
        vb->buffer[j++] = h;
        vb->buffer[j++] = location;
    }

    vb->length = j;
}

static void update_obstacle_vertex_data(Map* map)
{
    VertexBuffer* vb;
    VertexBuffer* map_vb;
    Obstacle* obstacle;
    vec2 pivot, stretch;
    f32 u, v, w, h;
    i32 location;
    i32 i, j;

    vb = get_vertex_buffer_swap(SSBO_OBSTACLE);
    map_vb = get_vertex_buffer(SSBO_OBSTACLE_MINIMAP);
    resize_vertex_buffer(vb, OBSTACLE_FLOATS_PER_VERTEX * game_context.obstacles->capacity);
    resize_vertex_buffer(map_vb, MAP_CIRCLE_FLOATS_PER_VERTEX * game_context.obstacles->capacity);

    i32 tex = texture_get_id("rock");
    texture_info(tex, &location, &u, &v, &w, &h, &pivot, &stretch);
    
    for (i = j = 0; i < game_context.obstacles->length; i++) {
        obstacle = list_get(game_context.obstacles, i);
        if (map_fog_contains(map, obstacle->position))
            continue;
        vb->buffer[j++] = obstacle->position.x;
        vb->buffer[j++] = obstacle->position.z;
        vb->buffer[j++] = obstacle->size;
        vb->buffer[j++] = u;
        vb->buffer[j++] = v;
        vb->buffer[j++] = w;
        vb->buffer[j++] = h;
        vb->buffer[j++] = location;
    }
    vb->length = j;

    for (i = j = 0; i < game_context.obstacles->length; i++) {
        obstacle = list_get(game_context.obstacles, i);
        if (map_fog_contains(map, obstacle->position))
            continue;
        map_vb->buffer[j++] = obstacle->position.x;
        map_vb->buffer[j++] = obstacle->position.z;
        map_vb->buffer[j++] = obstacle->size / 2;
        map_vb->buffer[j++] = 1.0f;
        map_vb->buffer[j++] = 0.5f;
        map_vb->buffer[j++] = 0.5f;
    }
    map_vb->length = j;
}

static void update_particle_vertex_data(Map* map)
{
    VertexBuffer* vb;
    Particle* particle;
    i32 i, j;

    vb = get_vertex_buffer_swap(SSBO_PARTICLE);
    resize_vertex_buffer(vb, PARTICLE_FLOATS_PER_VERTEX * game_context.particles->capacity);
   
    for (i = j = 0; i < game_context.particles->length; i++) {
        particle = list_get(game_context.particles, i);
        if (map_fog_contains(map, vec2_create(particle->position.x, particle->position.z)))
            continue;
        vb->buffer[j++] = particle->position.x;
        vb->buffer[j++] = particle->position.y;
        vb->buffer[j++] = particle->position.z;
        vb->buffer[j++] = particle->color.x;
        vb->buffer[j++] = particle->color.y;
        vb->buffer[j++] = particle->color.z;
        vb->buffer[j++] = particle->size;
    }
    
    vb->length = j;
}

static void update_parjicle_vertex_data(Map* map)
{
    VertexBuffer* vb;
    Parjicle* parjicle;
    bool rotate_tex;
    i32 i, j;

    vb = get_vertex_buffer_swap(SSBO_PARJICLE);
    resize_vertex_buffer(vb, PARJICLE_FLOATS_PER_VERTEX * game_context.parjicles->capacity);
   
    for (i = j = 0; i < game_context.parjicles->length; i++) {
        parjicle = list_get(game_context.parjicles, i);
        if (map_fog_contains(map, vec2_create(parjicle->position.x, parjicle->position.z)))
            continue;
        rotate_tex = parjicle_is_flag_set(parjicle, PARJICLE_FLAG_TEX_ROTATION);
        vb->buffer[j++] = parjicle->position.x;
        vb->buffer[j++] = parjicle->position.y;
        vb->buffer[j++] = parjicle->position.z;
        vb->buffer[j++] = parjicle->color.x;
        vb->buffer[j++] = parjicle->color.y;
        vb->buffer[j++] = parjicle->color.z;
        vb->buffer[j++] = parjicle->size * (rotate_tex ? 1 : -1);
        vb->buffer[j++] = parjicle->rotation;
    }
    
    vb->length = j;
}

static bool is_vertex_buffer_update(GameBufferEnum type)
{
    VertexBuffer* vb = get_vertex_buffer_swap(type);
    return vb->update;
}

static void vertex_buffer_updated(GameBufferEnum type)
{
    VertexBuffer* vb = get_vertex_buffer_swap(type);
    Buffer* buffer = get_buffer(type);
    vb->update = false;
    buffer->update = true;
}

void game_update_vertex_data(void)
{
    Map* map;
    VertexBuffer* vb;
    RenderData* tmp;

    vb = get_vertex_buffer_swap(SSBO_ENTITY);
    vb->update = true;
    vb = get_vertex_buffer_swap(SSBO_PROJECTILE);
    vb->update = true;
    vb = get_vertex_buffer_swap(SSBO_PARTICLE);
    vb->update = true;
    vb = get_vertex_buffer_swap(SSBO_PARJICLE);
    vb->update = true;

    map = game_context.current_map;

    if (is_vertex_buffer_update(SSBO_ENTITY)) {
        update_entity_vertex_data(map);
        vertex_buffer_updated(SSBO_ENTITY);
        vertex_buffer_updated(SSBO_ENTITY_MINIMAP);
        vertex_buffer_updated(SSBO_ENTITY_SHADOW);
    }
    if (is_vertex_buffer_update(SSBO_PROJECTILE)) {
        update_projectile_vertex_data(map);
        vertex_buffer_updated(SSBO_PROJECTILE);
    }
    if (is_vertex_buffer_update(SSBO_PARSTACLE)) {
        update_parstacle_vertex_data(map);
        vertex_buffer_updated(SSBO_PARSTACLE);
    }
    if (is_vertex_buffer_update(SSBO_OBSTACLE)) {
        update_obstacle_vertex_data(map);
        vertex_buffer_updated(SSBO_OBSTACLE);
        vertex_buffer_updated(SSBO_OBSTACLE_MINIMAP);
    }
    if (is_vertex_buffer_update(SSBO_PARTICLE)) {
        update_particle_vertex_data(map);
        vertex_buffer_updated(SSBO_PARTICLE);
    }
    if (is_vertex_buffer_update(SSBO_PARJICLE)) {
        update_parjicle_vertex_data(map);
        vertex_buffer_updated(SSBO_PARJICLE);
    }
    if (is_vertex_buffer_update(VBO_TILE)) {
        update_tile_vertex_data(map);
        vertex_buffer_updated(VBO_TILE);
        vertex_buffer_updated(VBO_TILE_MINIMAP);
    }
    if (is_vertex_buffer_update(VBO_WALL)) {
        update_wall_vertex_data(map);
        vertex_buffer_updated(VBO_WALL);
        vertex_buffer_updated(VBO_WALL_MINIMAP);
    }

    pthread_mutex_lock(&render_context.mutex);
    tmp = render_context.data;
    render_context.data = render_context.data_swap;
    render_context.data_swap = tmp;
    copy_camera();
    pthread_mutex_unlock(&render_context.mutex);
}

static void render_tiles(void)
{
    Buffer* buffer = &render_context.buffers[VBO_TILE];
    i32 num_tiles = buffer->length / TILE_VERTEX_LENGTH;
    shader_use(SHADER_PROGRAM_TILE);
    glBindVertexArray(render_context.vaos[VAO_TILE]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->name);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, num_tiles);
}

static void render_minimap_tiles(void)
{
    Buffer* buffer = &render_context.buffers[VBO_TILE_MINIMAP];
    i32 num_tiles = buffer->length / MAP_SQUARE_FLOATS_PER_VERTEX;
    shader_use(SHADER_PROGRAM_MINIMAP_SQUARE);
    glBindVertexArray(render_context.vaos[VAO_TILE_MINIMAP]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->name);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, num_tiles);
}

static void render_walls(void)
{
    Buffer* buffer = &render_context.buffers[VBO_WALL];
    i32 num_walls = buffer->length / WALL_VERTEX_LENGTH;
    shader_use(SHADER_PROGRAM_WALL);
    glBindVertexArray(render_context.vaos[VAO_WALL]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->name);
    // 6 vertices per face, 5 faces, kind of a mess should fix this
    glDrawArrays(GL_TRIANGLES, 0, 6 * 5 * num_walls);
}

static void render_minimap_walls(void)
{
    Buffer* buffer = &render_context.buffers[VBO_WALL_MINIMAP];
    i32 num_walls = buffer->length / MAP_SQUARE_FLOATS_PER_VERTEX;
    shader_use(SHADER_PROGRAM_MINIMAP_SQUARE);
    glBindVertexArray(render_context.vaos[VAO_WALL_MINIMAP]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->name);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, num_walls);
}

static void render_entities(void)
{
    Buffer* buffer = get_buffer(SSBO_ENTITY);
    shader_use(SHADER_PROGRAM_ENTITY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer->name);
    glDrawArrays(GL_TRIANGLES, 0, 6 * buffer->length / ENTITY_FLOATS_PER_VERTEX);
}

static void render_minimap_entities(void)
{
    Buffer* buffer = get_buffer(SSBO_ENTITY_MINIMAP);
    shader_use(SHADER_PROGRAM_MINIMAP_CIRCLE);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer->name);
    glDrawArrays(GL_TRIANGLES, 0, 6 * buffer->length / MAP_CIRCLE_FLOATS_PER_VERTEX);
}

static void render_shadow_entities(void)
{
    return;
    Buffer* buffer = get_buffer(SSBO_ENTITY_SHADOW);
    shader_use(SHADER_PROGRAM_SHADOW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer->name);
    glDrawArrays(GL_TRIANGLES, 0, 6 * buffer->length / SHADOW_FLOATS_PER_VERTEX);
}

static void render_projectiles(void)
{
    Buffer* buffer = get_buffer(SSBO_PROJECTILE);
    shader_use(SHADER_PROGRAM_PROJECTILE);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer->name);
    glDrawArrays(GL_TRIANGLES, 0, 6 * buffer->length / PROJECTILE_FLOATS_PER_VERTEX);
}

static void render_obstacles(void)
{
    Buffer* buffer = get_buffer(SSBO_OBSTACLE);
    shader_use(SHADER_PROGRAM_OBSTACLE);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer->name);
    glDrawArrays(GL_TRIANGLES, 0, 6 * buffer->length / OBSTACLE_FLOATS_PER_VERTEX);
}

static void render_minimap_obstacles(void)
{
    Buffer* buffer = get_buffer(SSBO_OBSTACLE_MINIMAP);
    shader_use(SHADER_PROGRAM_MINIMAP_CIRCLE);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer->name);
    glDrawArrays(GL_TRIANGLES, 0, 6 * buffer->length / MAP_CIRCLE_FLOATS_PER_VERTEX);
}

static void render_parstacles(void)
{
    Buffer* buffer = get_buffer(SSBO_PARSTACLE);
    shader_use(SHADER_PROGRAM_OBSTACLE);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer->name);
    glDrawArrays(GL_TRIANGLES, 0, 6 * buffer->length / OBSTACLE_FLOATS_PER_VERTEX);
}

static void render_particles(void)
{
    Buffer* buffer = get_buffer(SSBO_PARTICLE);
    shader_use(SHADER_PROGRAM_PARTICLE);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer->name);
    glDrawArrays(GL_TRIANGLES, 0, 6 * buffer->length / PARTICLE_FLOATS_PER_VERTEX);
}

static void render_parjicles(void)
{
    Buffer* buffer = get_buffer(SSBO_PARJICLE);
    shader_use(SHADER_PROGRAM_PARJICLE);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer->name);
    glDrawArrays(GL_TRIANGLES, 0, 6 * buffer->length / PARJICLE_FLOATS_PER_VERTEX);
}

void game_render_init(void)
{
    Buffer* buffer;
    i32 i;
    pthread_mutex_init(&render_context.mutex, NULL);
    render_context.data = st_calloc(1, sizeof(RenderData));
    render_context.data_swap = st_calloc(1, sizeof(RenderData));

    glGenBuffers(1, &render_context.game_time_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, render_context.game_time_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLdouble), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBO_INDEX_GAME_TIME, render_context.game_time_ubo);

    glGenVertexArrays(NUM_VAOS, &render_context.vaos[0]);
    for (i = 0; i < NUM_BUFFERS; i++) {
        buffer = &render_context.buffers[i];
        glGenBuffers(1, &buffer->name);
        buffer->target = GL_SHADER_STORAGE_BUFFER;
        buffer->usage = GL_DYNAMIC_DRAW;
    }

    render_context.buffers[VBO_TILE].target = GL_ARRAY_BUFFER;
    render_context.buffers[VBO_TILE].usage = GL_STATIC_DRAW;
    render_context.buffers[VBO_WALL].target = GL_ARRAY_BUFFER;
    render_context.buffers[VBO_WALL].usage = GL_STATIC_DRAW;
    render_context.buffers[VBO_TILE_MINIMAP].target = GL_ARRAY_BUFFER;
    render_context.buffers[VBO_TILE_MINIMAP].usage = GL_STATIC_DRAW;
    render_context.buffers[VBO_WALL_MINIMAP].target = GL_ARRAY_BUFFER;
    render_context.buffers[VBO_WALL_MINIMAP].usage = GL_STATIC_DRAW;

    glGenBuffers(1, &render_context.matrices_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, render_context.matrices_ubo);
    glBufferData(GL_UNIFORM_BUFFER, 35 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBO_INDEX_MATRICES, render_context.matrices_ubo);

    glGenBuffers(1, &render_context.minimap_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, render_context.minimap_ubo);
    glBufferData(GL_UNIFORM_BUFFER, 5 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBO_INDEX_MINIMAP, render_context.minimap_ubo);

    f32 quad_data[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    glBindVertexArray(render_context.vaos[VAO_QUAD]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.buffers[VBO_QUAD].name);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(render_context.vaos[VAO_TILE]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.buffers[VBO_QUAD].name);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.buffers[VBO_TILE].name);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(7 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(render_context.vaos[VAO_WALL]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.buffers[VBO_WALL].name);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glBindVertexArray(render_context.vaos[VAO_TILE_MINIMAP]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.buffers[VBO_QUAD].name);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.buffers[VBO_TILE_MINIMAP].name);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(render_context.vaos[VAO_WALL_MINIMAP]);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.buffers[VBO_QUAD].name);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, render_context.buffers[VBO_WALL_MINIMAP].name);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);

    shader_use(SHADER_PROGRAM_ENTITY);
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_ENTITY, "floats_per_vertex"), ENTITY_FLOATS_PER_VERTEX);
    shader_use(SHADER_PROGRAM_PROJECTILE);
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_PROJECTILE, "floats_per_vertex"), PROJECTILE_FLOATS_PER_VERTEX);
    shader_use(SHADER_PROGRAM_OBSTACLE);
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_OBSTACLE, "floats_per_vertex"), OBSTACLE_FLOATS_PER_VERTEX);
    shader_use(SHADER_PROGRAM_PARTICLE);
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_PARTICLE, "floats_per_vertex"), PARTICLE_FLOATS_PER_VERTEX);
    shader_use(SHADER_PROGRAM_PARJICLE);
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_PARJICLE, "floats_per_vertex"), PARJICLE_FLOATS_PER_VERTEX);
    shader_use(SHADER_PROGRAM_MINIMAP_CIRCLE);
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_MINIMAP_CIRCLE, "floats_per_vertex"), MAP_CIRCLE_FLOATS_PER_VERTEX);
    shader_use(SHADER_PROGRAM_SHADOW);
    glUniform1i(shader_get_uniform_location(SHADER_PROGRAM_SHADOW, "floats_per_vertex"), SHADOW_FLOATS_PER_VERTEX);
    shader_use(SHADER_PROGRAM_NONE);

    GLuint unit, name;

    glGenRenderbuffers(1, &render_context.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, render_context.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width(), window_height());

    unit = texture_get_unit(TEX_GAME_SCENE);
    name = texture_get_name(TEX_GAME_SCENE);
    glGenFramebuffers(1, &render_context.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, render_context.fbo);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width(), window_height(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, name, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_context.rbo);
    
    unit = texture_get_unit(TEX_GAME_SHADOW_SCENE);
    name = texture_get_name(TEX_GAME_SHADOW_SCENE);
    glGenFramebuffers(1, &render_context.shadow_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, render_context.shadow_fbo);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width(), window_height(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, name, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_context.rbo);

    unit = texture_get_unit(TEX_GAME_MINIMAP_SCENE);
    name = texture_get_name(TEX_GAME_MINIMAP_SCENE);
    glGenFramebuffers(1, &render_context.minimap_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, render_context.minimap_fbo);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 252, 252, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, name, 0);

    f32 ar = 1.0f;
    glBindBuffer(GL_UNIFORM_BUFFER, render_context.minimap_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat), sizeof(GLfloat), &ar);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void copy_buffers(void)
{
    VertexBuffer* vb;
    Buffer* buffer;
    i32 i;
    for (i = 0; i < NUM_BUFFERS; i++) {
        vb = get_vertex_buffer(i);
        buffer = get_buffer(i);
        if (!buffer->update)
            continue;
        glBindBuffer(buffer->target, buffer->name);
        if (buffer->capacity < vb->capacity) {
            glBufferData(buffer->target, vb->capacity * sizeof(GLfloat), NULL, buffer->usage);
            buffer->capacity = vb->capacity;
        }
        glBufferSubData(buffer->target, 0, vb->length * sizeof(GLfloat), vb->buffer);
        buffer->length = vb->length;
        glBindBuffer(buffer->target, 0);
        buffer->update = false;
    }
}

void game_render(void)
{
    GLuint loc, unit;

    if (game_context.halt_render)
        return;

    pthread_mutex_lock(&render_context.mutex);
    update_game_time();
    update_view_matrix();
    update_proj_matrix();
    copy_buffers();
    pthread_mutex_unlock(&render_context.mutex);

    GLenum buffer[] = { GL_COLOR_ATTACHMENT0 };
    const f32 transparent[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    const f32 color[4] = {0.4f, 0.4f, 0.4f, 1.0f};

    glViewport(0, 0, 252, 252);
    glBindFramebuffer(GL_FRAMEBUFFER, render_context.minimap_fbo);
    renderer_check_framebuffer_status(GL_FRAMEBUFFER, "minimap");
    glDrawBuffers(1, buffer);
    glClearBufferfv(GL_COLOR, 0, transparent);
    render_minimap_tiles();
    render_minimap_walls();
    render_minimap_obstacles();
    render_minimap_entities();

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glViewport(0, 0, window_width(), window_height());
    glBindFramebuffer(GL_FRAMEBUFFER, render_context.fbo);
    renderer_check_framebuffer_status(GL_FRAMEBUFFER, "game");
    //glStencilMask(0x01); // stencil mask affects glClear
    glDrawBuffers(1, buffer);
    glClearBufferfv(GL_COLOR, 0, color);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    glStencilFunc(GL_ALWAYS, 1, 0x01);
    glStencilMask(0x01);
    render_walls();

    glStencilFunc(GL_NOTEQUAL, 1, 0x01);
    glStencilMask(0x00);
    render_tiles();

    glStencilFunc(GL_ALWAYS, 1, 0x01);
    glStencilMask(0x01);
    render_obstacles();
    render_parstacles();
    render_entities();
    render_projectiles();
    render_particles();
    render_parjicles();

    glBindFramebuffer(GL_FRAMEBUFFER, render_context.shadow_fbo);
    renderer_check_framebuffer_status(GL_FRAMEBUFFER, "shadow");
    glClearBufferfv(GL_COLOR, 0, color);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
    glDisable(GL_DEPTH_TEST);
    glStencilMask(0x00);
    shader_use(SHADER_PROGRAM_SCREEN);
    unit = texture_get_unit(TEX_GAME_SCENE);
    loc = shader_get_uniform_location(SHADER_PROGRAM_SCREEN, "screenTex"); 
    glUniform1i(loc, unit);
    glBindVertexArray(render_context.vaos[VAO_QUAD]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glStencilFunc(GL_NOTEQUAL, 1, 0x01);
    render_shadow_entities();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_STENCIL_TEST);
    shader_use(SHADER_PROGRAM_SCREEN);
    unit = texture_get_unit(TEX_GAME_SHADOW_SCENE);
    loc = shader_get_uniform_location(SHADER_PROGRAM_SCREEN, "screenTex"); 
    glUniform1i(loc, unit);
    glBindVertexArray(render_context.vaos[VAO_QUAD]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void game_render_framebuffer_size_callback(void)
{
    if (render_context.fbo == 0 
     || render_context.rbo == 0
     || render_context.shadow_fbo == 0)
        return;

    i32 width = window_width();
    i32 height = window_height();
    if (width == 0 || height == 0)
        return;

    GLuint unit, name;
    unit = texture_get_unit(TEX_GAME_SCENE);
    name = texture_get_name(TEX_GAME_SCENE);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindFramebuffer(GL_FRAMEBUFFER, render_context.fbo);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
            0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    unit = texture_get_unit(TEX_GAME_SHADOW_SCENE);
    name = texture_get_name(TEX_GAME_SHADOW_SCENE);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindFramebuffer(GL_FRAMEBUFFER, render_context.shadow_fbo);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
            0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindRenderbuffer(GL_RENDERBUFFER, render_context.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void game_render_update_obstacles(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(SSBO_OBSTACLE);
    vb->update = true;
    vb = get_vertex_buffer_swap(SSBO_OBSTACLE);
    vb->update = true;
}

void game_render_update_parstacles(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(SSBO_PARSTACLE);
    vb->update = true;
    vb = get_vertex_buffer_swap(SSBO_PARSTACLE);
    vb->update = true;
}

void game_render_update_tiles(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_TILE);
    vb->update = true;
    vb = get_vertex_buffer_swap(VBO_TILE);
    vb->update = true;
}

void game_render_update_walls(void)
{
    VertexBuffer* vb;
    vb = get_vertex_buffer(VBO_WALL);
    vb->update = true;
    vb = get_vertex_buffer_swap(VBO_WALL);
    vb->update = true;
}

void game_render_cleanup(void)
{
    pthread_mutex_destroy(&render_context.mutex);
    glDeleteVertexArrays(NUM_VAOS, &render_context.vaos[0]);
    glDeleteBuffers(1, &render_context.game_time_ubo);
    glDeleteFramebuffers(1, &render_context.fbo);
    glDeleteFramebuffers(1, &render_context.shadow_fbo);
    glDeleteBuffers(1, &render_context.minimap_fbo);
    glDeleteRenderbuffers(1, &render_context.rbo);
    glDeleteBuffers(1, &render_context.matrices_ubo);
    glDeleteBuffers(1, &render_context.minimap_ubo);
    for (i32 i = 0; i < NUM_BUFFERS; i++) {
        glDeleteBuffers(1, &render_context.buffers[i].name);
        st_free(render_context.data->buffers[i].buffer);
        st_free(render_context.data_swap->buffers[i].buffer);
    }
    st_free(render_context.data);
    st_free(render_context.data_swap);
}

