#ifndef CONFIG_H
#define CONFIG_H

#include <json.h>

typedef struct Config {
    JsonObject* weapons;
    JsonObject* textures;
    JsonObject* entities;
    JsonObject* maps;
    void* shared_handle;
} Config;

Config*     config_create(void);
void        config_destroy(Config* config);
void*       config_get_function(Config* config, const char* name);

#endif
