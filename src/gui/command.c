#ifndef COMMAND_H
#define COMMAND_H

#include "internal.h"
#include "../game.h"
#include <ctype.h>

#define ERROR       -1
#define START       0
#define LOGLEVEL    1
#define LOGLEVELFIN 2
#define PRESET      3
#define PRESETFIN   4

extern GUIContext gui_context;

static char* error_message;

static const char* state_string(i32 state)
{
    switch (state) {
        case ERROR:
            return "Error: ";
        case START: 
            return "Must type something";
        case LOGLEVEL: 
            return "Must provide log level";
        case LOGLEVELFIN:
            return "Successfully changed log level";
        case PRESET:
            return "Must provide preset name";
        case PRESETFIN:
            return "Successfully loaded preset";
    }
    return "";
}

static bool cmp(const char* target, const char* input, i32 left, i32 right)
{
    i32 i;
    for (i = 0; target[i] != '\0' && i <= right; i++)
        if (input[i+left] != target[i])
            return false;
    return target[i] == '\0' && i > right;
}

static i32 new_state(i32 state, char* command, i32 left, i32 right)
{
    switch (state) {
        case START:
            if (cmp("loglevel", command, left, right))
                return LOGLEVEL;
            if (cmp("load", command, left, right))
                return PRESET;
            return ERROR;
        case LOGLEVEL:
            i32 level;
            char c = command[right+1];
            char* endptr;
            command[right+1] = '\0';
            level = strtol(command+left, &endptr, 10);
            command[right+1] = c;
            if (endptr == command+left || *endptr != '\0') {
                error_message = "Invalid number";
                return ERROR;
            }
            log_set_level(level);
            return LOGLEVELFIN;
        case PRESET:
            command[right+1] = '\0';
            i32 id = game_preset_map_id(command+left);
            command[right+1] = '\0';
            if (id == -1) {
                error_message = "Invalid preset name";
                return ERROR;
            }
            game_event_create_preset_load(id);
            return PRESETFIN;
    }
    return 0;
}

static bool target_char(char c)
{
    return isspace(c) || c == '\0';
}

const char* gui_parse_command(char* command)
{
    i32 state, left, right;
    error_message = "Unrecognized command";
    state = left = right = 0;
    for (right = 0; command[right] != '\0'; right++) {
        if (left == -1 && !target_char(command[right]))
            left = right;
        if (left != -1 && target_char(command[right+1])) {
            state = new_state(state, command, left, right);
            left = -1;
        }
    }
    return state_string(state);
}

#endif
