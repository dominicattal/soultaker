#ifndef COMMAND_H
#define COMMAND_H

#include "gui.h"
#include "state.h"
#include "game.h"
#include "window.h"
#include "renderer.h"
#include "event.h"
#include <ctype.h>

char* command_parse(char* command)
{
    return string_copy(command);
}

#endif
