#ifndef API_H
#define API_H

#include "util.h"
#include "game.h"

#ifdef _WIN32
#define st_export __declspec(dllexport)
#else
#define st_export
#endif

typedef struct GameApi GameApi;

#endif
