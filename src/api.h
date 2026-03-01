#ifndef API_H
#define API_H

#ifdef _WIN32
#define st_export __declspec(dllexport)
#else
#define st_export
#endif

#include "game.h"
#include "util.h"
#include <math.h>

#endif
