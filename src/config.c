#include "config.h"
#include "util.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

#ifdef _WIN32
#include <windows.h>
const char* pathname = "plugins/soultaker.dll";
const char* shared_ext = ".dll";
const int flags = 0;
void* dlopen(const char* path, i32 flags)
{
    return LoadLibrary(path);
}
void* dlsym(void* handle, const char* symbol)
{
    return GetProcAddress(handle, symbol);
}
char* dlerror(void)
{
    return "could not load library";
}
int dlclose(void* handle)
{
    return FreeLibrary(handle);
}
#else
#include <dlfcn.h>
const char* shared_ext = ".so";
const char* pathname = "bin/dev/plugins/soultaker.so";
const int flags = RTLD_NOW;
#endif

static bool is_dir(const char* dir_path, const struct dirent* iterator)
{
#ifdef _WIN32
    struct stat path_stat;
    char* path = string_create("%s/%s", dir_path, iterator->d_name);
    stat(path, &path_stat);
    string_free(path);
    return (path_stat.st_mode & S_IFMT) == S_IFDIR;
#else
    return iterator->d_type == DT_DIR;
#endif
}

static bool ends_with(const char* string, const char* suffix)
{
    int n = strlen(string);
    int m = strlen(suffix);
    if (n < m) 
        return false;
    if (strcmp(string+n-m, suffix) != 0)
        return false;
    return true;
}

static void read_weapons(Config* config, const char* dir_path)
{
    DIR* cur_dir = opendir(dir_path);
    if (cur_dir == NULL) {
        log_write(WARNING, "Something weird happened %s", dir_path);
        return;
    }
    JsonObject* object;
    JsonObject* new_config_weapons;
    char* new_dir;
    char* path;
    const struct dirent* iterator = readdir(cur_dir);
    while (iterator != NULL) {
        if (iterator->d_name[0] != '.') {
            if (is_dir(dir_path, iterator)) {
                new_dir = string_create("%s/%s", dir_path, iterator->d_name);
                read_weapons(config, new_dir);
                string_free(new_dir);
            } else if (ends_with(iterator->d_name, ".json")) {
                path = string_create("%s/%s", dir_path, iterator->d_name);
                log_write(INFO, "Reading weapon config: %s", path);
                object = json_read(path);
                if (object == NULL)
                    log_write(CRITICAL, "Error reading %s", path);
                else {
                    new_config_weapons = json_merge_objects(config->weapons, object);
                    if (new_config_weapons == NULL)
                        log_write(CRITICAL, "Error merging %s", path);
                    else
                        config->weapons = new_config_weapons;
                }
                string_free(path);
            }
        }
        iterator = readdir(cur_dir);
    }
    closedir(cur_dir);
}

static void read_entities(Config* config, const char* dir_path)
{
    DIR* cur_dir = opendir(dir_path);
    if (cur_dir == NULL) {
        log_write(WARNING, "Something weird happened %s", dir_path);
        return;
    }
    JsonObject* object;
    JsonObject* new_config_entities;
    char* new_dir;
    char* path;
    const struct dirent* iterator = readdir(cur_dir);
    while (iterator != NULL) {
        if (iterator->d_name[0] != '.') {
            if (is_dir(dir_path, iterator)) {
                new_dir = string_create("%s/%s", dir_path, iterator->d_name);
                read_entities(config, new_dir);
                string_free(new_dir);
            } else if (ends_with(iterator->d_name, ".json")) {
                path = string_create("%s/%s", dir_path, iterator->d_name);
                log_write(INFO, "Reading entity config: %s", path);
                object = json_read(path);
                if (object == NULL)
                    log_write(CRITICAL, "Error reading %s", path);
                else {
                    new_config_entities = json_merge_objects(config->entities, object);
                    if (new_config_entities == NULL)
                        log_write(CRITICAL, "Error merging %s", path);
                    else
                        config->entities = new_config_entities;
                }
                string_free(path);
            }
        }
        iterator = readdir(cur_dir);
    }
    closedir(cur_dir);
}

static void read_textures(Config* config, const char* dir_path)
{
    DIR* cur_dir = opendir(dir_path);
    if (cur_dir == NULL) {
        log_write(WARNING, "Something weird happened %s", dir_path);
        return;
    }
    JsonObject* object;
    JsonObject* new_config_textures;
    char* new_dir;
    char* path;
    const struct dirent* iterator = readdir(cur_dir);
    while (iterator != NULL) {
        if (iterator->d_name[0] != '.') {
            if (is_dir(dir_path, iterator)) {
                new_dir = string_create("%s/%s", dir_path, iterator->d_name);
                read_textures(config, new_dir);
                string_free(new_dir);
            } else if (ends_with(iterator->d_name, ".json")) {
                path = string_create("%s/%s", dir_path, iterator->d_name);
                log_write(INFO, "Reading texture config: %s", path);
                object = json_read(path);
                if (object == NULL)
                    log_write(CRITICAL, "Error reading %s", path);
                else {
                    new_config_textures = json_merge_objects(config->textures, object);
                    if (new_config_textures == NULL)
                        log_write(CRITICAL, "Error merging %s", path);
                    else
                        config->textures = new_config_textures;
                }
                string_free(path);
            }
        }
        iterator = readdir(cur_dir);
    }
    closedir(cur_dir);
}

static void read_maps(Config* config, const char* dir_path)
{
    DIR* cur_dir = opendir(dir_path);
    if (cur_dir == NULL) {
        log_write(WARNING, "Something weird happened %s", dir_path);
        return;
    }
    JsonObject* object;
    JsonObject* new_config_maps;
    char* new_dir;
    char* path;
    const struct dirent* iterator = readdir(cur_dir);
    while (iterator != NULL) {
        if (iterator->d_name[0] != '.') {
            if (is_dir(dir_path, iterator)) {
                new_dir = string_create("%s/%s", dir_path, iterator->d_name);
                read_maps(config, new_dir);
                string_free(new_dir);
            } else if (ends_with(iterator->d_name, ".json")) {
                path = string_create("%s/%s", dir_path, iterator->d_name);
                log_write(INFO, "Reading map config: %s", path);
                object = json_read(path);
                if (object == NULL)
                    log_write(CRITICAL, "Error reading %s", path);
                else {
                    new_config_maps = json_merge_objects(config->maps, object);
                    if (new_config_maps == NULL)
                        log_write(CRITICAL, "Error merging %s", path);
                    else
                        config->maps = new_config_maps;
                }
                string_free(path);
            }
        }
        iterator = readdir(cur_dir);
    }
    closedir(cur_dir);
}

static void read_config(Config* config, const char* dir_path)
{
    DIR* config_dir = opendir(dir_path);
    if (config_dir == NULL) {
        log_write(WARNING, "%s found in config folder not directory", dir_path);
        return;
    }
    char* new_dir;
    const struct dirent* config_type = readdir(config_dir);
    while (config_type != NULL) {
        if (strcmp(config_type->d_name, "textures") == 0) {
            new_dir = string_create("%s/%s", dir_path, config_type->d_name);
            read_textures(config, new_dir);
            string_free(new_dir);
        } else if (strcmp(config_type->d_name, "entities") == 0) {
            new_dir = string_create("%s/%s", dir_path, config_type->d_name);
            read_entities(config, new_dir);
            string_free(new_dir);
        } else if (strcmp(config_type->d_name, "maps") == 0) {
            new_dir = string_create("%s/%s", dir_path, config_type->d_name);
            read_maps(config, new_dir);
            string_free(new_dir);
        } else if (strcmp(config_type->d_name, "weapons") == 0) {
            new_dir = string_create("%s/%s", dir_path, config_type->d_name);
            read_weapons(config, new_dir);
            string_free(new_dir);
        }
        config_type = readdir(config_dir);
    }
    closedir(config_dir);
}

static void read_plugin(Config* config, const char* dir_path)
{
    DIR* plugin_dir = opendir(dir_path);
    if (plugin_dir == NULL) {
        log_write(WARNING, "%s found in plugins folder not directory", dir_path);
        return;
    }
    char* new_dir;
    const struct dirent* config_root = readdir(plugin_dir);
    while (config_root != NULL) {
        if (strcmp(config_root->d_name, "config") == 0) {
            new_dir = string_create("%s/%s", dir_path, config_root->d_name);
            read_config(config, new_dir);
            string_free(new_dir);
            goto found;
        }
        config_root = readdir(plugin_dir);
    }
    log_write(WARNING, "Could not find config files for plugin %s", dir_path);
found:
    closedir(plugin_dir);
}

static void read_plugins(Config* config)
{
    const char* dir_path = "plugins";
    char* new_dir;
    DIR* root_dir = opendir(dir_path);
    if (root_dir == NULL)
        log_write(FATAL, "plugin directory doesn't exist");
    const struct dirent* plugin = readdir(root_dir);
    while (plugin != NULL) {
        if (plugin->d_name[0] != '.') {
            new_dir = string_create("%s/%s", dir_path, plugin->d_name);
            read_plugin(config, new_dir);
            string_free(new_dir);
        }
        plugin = readdir(root_dir);
    }
    closedir(root_dir);
}

Config* config_create(void)
{
    Config* config = st_malloc(sizeof(Config));
    config->weapons = json_object_create();
    config->textures = json_object_create();
    config->entities = json_object_create();
    config->maps = json_object_create();
    read_plugins(config);
    if (json_object_length(config->textures) == 0)
        log_write(FATAL, "Did not find any textures");
    if (json_object_length(config->entities) == 0)
        log_write(FATAL, "Did not find any entities");
    if (json_object_length(config->maps) == 0)
        log_write(FATAL, "Did not find any maps");
    config->shared_handle = dlopen(pathname, flags);
    return config;
}

void config_destroy(Config* config)
{
    json_object_destroy(config->weapons);
    json_object_destroy(config->textures);
    json_object_destroy(config->entities);
    json_object_destroy(config->maps);
    dlclose(config->shared_handle);
    st_free(config);
}

void* config_get_function(Config* config, const char* name)
{
    return dlsym(config->shared_handle, name);
}
