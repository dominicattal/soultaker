#ifndef COMMAND_H
#define COMMAND_H

#include "gui.h"
#include "state.h"
#include "game.h"
#include "window.h"
#include "renderer.h"
#include "event.h"
#include <string.h>
#include <ctype.h>

typedef struct {
    i32 l, r;
} StringView;

static i32 string_view_cmp(StringView* string_view, const char* command, const char* string)
{
    i32 length = string_view->r - string_view->l + 1;
    i32 i;
    for (i = 0; i < length && string[i] != '\0'; i++) {
        if (command[i] > string[i])
            return 1;
        else if (command[i] < string[i])
            return -1;
    }
    if (i == length && string[i] != '\0')
        return 1;
    if (i != length && string[i] == '\0')
        return -1;
    return 0;
}

static bool string_view_eq(StringView* string_view, const char* command, const char* string)
{
    return string_view_cmp(string_view, command, string) == 0;
}

static char* string_view_c_str(StringView* string_view, const char* command)
{
    i32 length = string_view->r - string_view->l + 1;
    char* string = st_malloc((length + 1) * sizeof(char));
    memcpy(string, command + string_view->l, length);
    string[length] = '\0';
    return string;
}

static void string_view_log(StringView* string_view, const char* command)
{
    char* string = string_view_c_str(string_view, command);
    log_write(DEBUG, string);
    st_free(string);
}

static char* parse_map(List* string_views, const char* command)
{
    i32 map_id;
    StringView* string_view;
    char* map_name = NULL;
    char* response = NULL;
    if (string_views->length < 2) {
        response = string_copy("not enough arguments for map");
        goto fail;
    }
    string_view = list_get(string_views, 1);
    string_view_log(string_view, command);
    map_name = string_view_c_str(string_view, command);
    map_id = map_get_id(map_name);
    if (map_id == -1) {
        response = string_create("unrecognized map %s", map_name);
        goto fail;
    }
    event_create_game_change_map(map_id);
    response = string_create("Loaded map %s", map_name);
fail:
    log_assert(response != NULL, "response null for map parsing");
    if (map_name != NULL)
        st_free(map_name);
    return response;
}

static char* parse_toggle(List* string_views, const char* command)
{
    i32 map_id;
    StringView* string_view;
    char* flag_name = NULL;
    char* response = NULL;
    if (string_views->length < 2) {
        response = string_copy("not enough arguments for toggle");
        goto fail;
    }
    string_view = list_get(string_views, 1);
    string_view_log(string_view, command);
    flag_name = string_view_c_str(string_view, command);
    if (strcmp(flag_name, "spatial_hash_lines") == 0) {
        map_toggle_spatial_hash_lines(game_context.current_map);
        response = string_copy("Toggled spatial has lines");
    }
    if (strcmp(flag_name, "debug") == 0) {
        if (gui_toggle_debug())
            response = string_copy("Toggled debug");
        else
            response = string_copy("could not toggle debug");
    }
    if (strcmp(flag_name, "camera_lock") == 0) {
        bool status = camera_toggle_lock();
        if (status)
            response = string_copy("Camera lock is on");
        else
            response = string_copy("Camera lock is off");
    }
fail:
    if (response == NULL)
        response = string_copy("Invalid toggle argument");
    if (flag_name != NULL)
        st_free(flag_name);
    return response;
}

static char* parse_set(List* string_views, const char* command)
{
    char* response = NULL;
    char* var_name = NULL;
    StringView* string_view;
    char* c_str;
    if (string_views->length < 2) {
        response = string_copy("Missing argument for set");
        goto fail;
    }
    string_view = list_get(string_views, 1);
    string_view_log(string_view, command);
    var_name = string_view_c_str(string_view, command);
    if (strcmp(var_name, "camera_target") == 0) {
        if (string_views->length < 4) {
            response = string_copy("set camera_target {x} {z}");
            goto fail;
        }
        string_view = list_get(string_views, 2);
        c_str = string_view_c_str(string_view, command);
        f32 x = atof(c_str);
        st_free(c_str);
        string_view = list_get(string_views, 3);
        c_str = string_view_c_str(string_view, command);
        f32 z = atof(c_str);
        st_free(c_str);
        camera_set_target(vec2_create(x, z));
        response = string_create("set camera target to %.3f %.3f", x, z);
    } else if (strcmp(var_name, "tps") == 0) {
        if (string_views->length < 3) {
            response = string_copy("set tps {tps}");
            goto fail;
        }
        string_view = list_get(string_views, 2);
        c_str = string_view_c_str(string_view, command);
        game_context.tps = atoi(c_str);
        game_context.timestep = 1.0 / game_context.tps;
        st_free(c_str);
    }
fail:
    if (var_name != NULL)
        st_free(var_name);
    if (response == NULL)
        response = string_copy("Invalid set argument");
    return response;
}

char* command_parse(char* command)
{
    List* string_views = list_create();
    char* response = NULL;
    StringView* string_view;
    i32 i, l, r;
    for (l = r = 0; command[r] != '\0'; r++) {
        if (command[r] == ' ') {
            if (l != r) {
                string_view = st_malloc(sizeof(StringView));
                string_view->l = l;
                string_view->r = r-1;
                list_append(string_views, string_view);
            }
            l = r+1;
        }
    }
    if (l != r) {
        string_view = st_malloc(sizeof(StringView));
        string_view->l = l;
        string_view->r = r-1;
        list_append(string_views, string_view);
    }
    l = r+1;

    if (string_views->length == 0) {
        response = string_copy("no arguments given");
        goto destroy;
    }

    string_view = list_get(string_views, 0);
    if (string_view_eq(string_view, command, "map"))
        response = parse_map(string_views, command);
    else if (string_view_eq(string_view, command, "toggle"))
        response = parse_toggle(string_views, command);
    else if (string_view_eq(string_view, command, "set"))
        response = parse_set(string_views, command);
    else if (string_view_eq(string_view, command, "pause")) {
        game_pause();
        response = string_create("Paused game");
    } else if (string_view_eq(string_view, command, "resume")) {
        game_resume();
        response = string_create("Resumed game");
    } else if (string_view_eq(string_view, command, "defog")) {
        map_fog_clear(game_context.current_map);
        response = string_copy("defogged map");
    } else if (string_view_eq(string_view, command, "pos")) {
        vec2 target_position = camera_get_target_position();
        response = string_create("%f %f", target_position.x, target_position.z);
    } else
        response = string_create("Unrecognized command `%s`", command);

destroy:
    for (i = 0; i < string_views->length; i++)
        st_free(list_get(string_views, i));
    list_destroy(string_views);
    log_assert(response != NULL, "could not get response from command");
    return response;
}

#endif
