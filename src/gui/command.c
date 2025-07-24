#ifndef COMMAND_H
#define COMMAND_H

#include "internal.h"
#include "../game.h"
#include "../event.h"
#include <ctype.h>

typedef enum {
    CS_ERROR,
    CS_START,
    CS_LOGLEVEL,
    CS_LOGLEVELFIN,
    CS_PRESET,
    CS_PRESETFIN,
    CS_POSITIONFIN,
    CS_SUMMON,
    CS_SUMMONFIN,
    CS_RESPAWNFIN,
    NUM_COMMAND_STATES
} CommandState;

extern GUIContext gui_context;

static struct {

    // static allocation, managed by command context
    char* error_message;

    // arguments for command
    char* arg_string;

} command_context;

static char* state_string(CommandState state)
{
    char* message;
    switch (state) {
        case CS_ERROR:
            message = string_copy(command_context.error_message);
            break;
        case CS_START: 
            message = string_copy("Must type something");
            break;
        case CS_LOGLEVEL: 
            message = string_copy("Must provide log level");
            break;
        case CS_LOGLEVELFIN:
            message = string_copy("Successfully changed log level");
            break;
        case CS_PRESET:
            message = string_copy("Must provide preset name");
            break;
        case CS_PRESETFIN:
            message = string_create("Successfully loaded preset %s", 
                    command_context.arg_string);
            break;
        case CS_POSITIONFIN:
            vec2 position = player_position();
            message = string_create("%.2f %.2f", position.x, position.y);
            break;
        case CS_SUMMON:
            message = string_create("Must provide entity name");
            break;
        case CS_SUMMONFIN:
            message = string_create("Successfully summoned %s", 
                    command_context.arg_string);
            break;
        case CS_RESPAWNFIN:
            message = string_copy("Respawned");
            break;
        default:
            message = string_copy("Unrecognized command");
            break;
    }
    string_free(command_context.arg_string);
    command_context.arg_string = NULL;
    return message;
}

// compare substring of input to target
static bool cmp(const char* target, const char* input, i32 left, i32 right)
{
    i32 i;
    for (i = 0; target[i] != '\0' && i <= right; i++)
        if (input[i+left] != target[i])
            return false;
    return target[i] == '\0' && i > right;
}

static CommandState new_state(CommandState state, char* command, i32 left, i32 right)
{
    i32 id;
    char c;
    switch (state) {
        case CS_START:
            if (cmp("loglevel", command, left, right))
                return CS_LOGLEVEL;
            if (cmp("preset", command, left, right))
                return CS_PRESET;
            if (cmp("summon", command, left, right))
                return CS_SUMMON;
            if (cmp("position", command, left, right))
                return CS_POSITIONFIN;
            if (cmp("respawn", command, left, right)) {
                event_create_game_respawn();
                return CS_RESPAWNFIN;
            }
            command_context.error_message = "Unrecognized command";
            return CS_ERROR;
        case CS_SUMMON:
            c = command[right+1];
            command[right+1] = '\0';
            command_context.arg_string = string_copy(command+left);
            id = entity_get_id(command+left);
            command[right+1] = c;
            if (id == -1) {
                command_context.error_message = "Invalid entity name";
                return CS_ERROR;
            }
            event_create_game_summon(id);
            return CS_SUMMONFIN;
        case CS_LOGLEVEL:
            i32 level;
            c = command[right+1];
            char* endptr;
            command[right+1] = '\0';
            level = strtol(command+left, &endptr, 10);
            command[right+1] = c;
            if (endptr == command+left || *endptr != '\0') {
                command_context.error_message = "Invalid number";
                return CS_ERROR;
            }
            log_set_level(level);
            return CS_LOGLEVELFIN;
        case CS_PRESET:
            c = command[right+1];
            command[right+1] = '\0';
            command_context.arg_string = string_copy(command+left);
            id = game_preset_get_id(command+left);
            command[right+1] = c;
            if (id == -1) {
                command_context.error_message = "Invalid preset name";
                return CS_ERROR;
            }
            event_create_game_preset_load(id);
            return CS_PRESETFIN;
        default:
            return CS_ERROR;
    }
    return CS_ERROR;
}

static bool whitespace(char c)
{
    return isspace(c) || c == '\0';
}

char* gui_command_parse(char* command)
{
    CommandState state;
    i32 left, right;
    state = CS_START;
    left = right = 0;
    // sliding window; keep moving right until whitespace, then reset
    for (right = 0; command[right] != '\0'; right++) {
        // skip whitespace
        if (left == -1 && !whitespace(command[right]))
            left = right;

        if (left != -1 && whitespace(command[right+1])) {
            state = new_state(state, command, left, right);
            left = -1;
        }
    }
    return state_string(state);
}

#endif
