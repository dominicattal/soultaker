#ifndef AUDIO_H
#define AUDIO_H

#include "util.h"

void audio_init(void);
i32  audio_get_id(const char* name);
void audio_play(i32 aud_id);
void audio_cleanup(void);

#endif
