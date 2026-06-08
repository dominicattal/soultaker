#include "../game.h"
#include "../renderer.h"
#include <math.h>
#include <string.h>

extern GameContext game_context;

void wall_init(void)
{
}

Wall* wall_create(vec2 position, f32 height, u32 minimap_color)
{
    Wall* wall = st_malloc(sizeof(Wall));
    wall->map_info = (MapInfo) {0};
    wall->position = position;
    wall->minimap_color = minimap_color;
    wall->size = vec2_create(1, 1);
    wall->height = height;
    wall->top_tex = texture_get_id("wall_1");
    wall->side_tex = texture_get_id("wall_2");
    wall->flags = 0;
    wall_set_flag(wall, WALL_FLAG_ACTIVE, true);
    return wall;
}

void wall_set_flag(Wall* wall, WallFlagEnum flag, bool val)
{
    wall->flags = (wall->flags & ~(1<<flag)) | (val<<flag);
}

bool wall_get_flag(Wall* wall, WallFlagEnum flag)
{
    return (wall->flags >> flag) & 1;
}

void wall_destroy(Wall* wall)
{
    st_free(wall);
}

void wall_cleanup(void)
{
}

size_t wall_sizeof(void)
{
    Wall wall;
    return sizeof(wall.position)
        +  sizeof(wall.size)
        +  sizeof(wall.height)
        +  sizeof(wall.top_tex)
        +  sizeof(wall.side_tex)
        +  sizeof(wall.minimap_color)
        +  sizeof(wall.flags);
}

char* wall_write(Wall* wall, char* buffer)
{
    memcpy(buffer, &wall->position, sizeof(wall->position));
    buffer += sizeof(wall->position);
    memcpy(buffer, &wall->size, sizeof(wall->size));
    buffer += sizeof(wall->size);
    memcpy(buffer, &wall->height, sizeof(wall->height));
    buffer += sizeof(wall->height);
    memcpy(buffer, &wall->top_tex, sizeof(wall->top_tex));
    buffer += sizeof(wall->top_tex);
    memcpy(buffer, &wall->side_tex, sizeof(wall->side_tex));
    buffer += sizeof(wall->side_tex);
    memcpy(buffer, &wall->minimap_color, sizeof(wall->minimap_color));
    buffer += sizeof(wall->minimap_color);
    memcpy(buffer, &wall->flags, sizeof(wall->flags));
    buffer += sizeof(wall->flags);
    return buffer;
}

Wall* wall_read(char** buffer)
{
    Wall* wall = st_calloc(1, sizeof(Wall));
    memcpy(&wall->position, *buffer, sizeof(wall->position));
    *buffer += sizeof(wall->position);
    memcpy(&wall->size, *buffer, sizeof(wall->size));
    *buffer += sizeof(wall->size);
    memcpy(&wall->height, *buffer, sizeof(wall->height));
    *buffer += sizeof(wall->height);
    memcpy(&wall->top_tex, *buffer, sizeof(wall->top_tex));
    *buffer += sizeof(wall->top_tex);
    memcpy(&wall->side_tex, *buffer, sizeof(wall->side_tex));
    *buffer += sizeof(wall->side_tex);
    memcpy(&wall->minimap_color, *buffer, sizeof(wall->minimap_color));
    *buffer += sizeof(wall->minimap_color);
    memcpy(&wall->flags, *buffer, sizeof(wall->flags));
    *buffer += sizeof(wall->flags);
    return wall;
}
