#include "internal.h"
#include <stb_image.h>

Map* map_load(const char* path)
{
    i32 x, y, n, i, j, idx;
    u64 map_area;
    unsigned char* data = stbi_load(path, &x, &y, &n, 3);
    if (data == NULL)
        log_write(FATAL, "Could not load map data for %s", path);
    Map* map = st_malloc(sizeof(Map));
    map->width = x;
    map->length = y;
    map_area = map->width * map->length;
    map->data = st_malloc(map_area * sizeof(u32));
    for (i = 0; i < map->length; i++) {
        for (j = 0; j < map->width; j++) {
            idx = (map->length-i-1) * map->width + j;
            map->data[i*map->width+j] = data[3*idx+2] + (data[3*idx+1]<<8) + (data[3*idx]<<16);
        }
    }
    stbi_image_free(data);
    return map;
}

void map_free(Map* map)
{
    st_free(map->data);
    st_free(map);
}
