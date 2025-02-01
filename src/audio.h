#ifndef AUDIO_H
#define AUDIO_H

#include "util.h"

typedef enum {
    AUD_HIT,
    NUM_SOUNDS
} AudioID;

typedef struct {
    ALCdevice* device;
    ALCcontext* context;
    ALuint buffers[NUM_SOUNDS];
} AudioContext;

extern AudioContext audio_context;

void audio_init(void);
void audio_play_sound(AudioID id);
void audio_cleanup(void);

#endif
