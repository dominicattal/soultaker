#include "../audio.h"
#include <stdio.h>

AudioContext audio_context;

static void checkError(const char* msg) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        fprintf(stderr, "OpenAL Error: %s\n", msg);
        fprintf(stderr, "Error code: %X\n", error);
        exit(1);
    }
}

static void load_sound(ALuint id, char *path)
{
    SNDFILE* file;
    SF_INFO sfinfo;
    ALenum format;
    ALshort* samples;
    ALsizei numSamples;
    ALsizei size;
    ALsizei freq;
    
    file = sf_open(path, SFM_READ, &sfinfo);
    if (!file) {
        fprintf(stderr, "Failed to open sound file: %s.\n", path);
        exit(1);
    }

    if (sfinfo.channels == 1) {
        format = AL_FORMAT_MONO16;
    } else if (sfinfo.channels == 2) {
        format = AL_FORMAT_STEREO16;
    } else {
        fprintf(stderr, "Unsupported number of channels: %s.\n", path);
        sf_close(file);
        exit(1);
    }

    size = sfinfo.frames * sfinfo.channels * sizeof(ALshort);
    samples = malloc(size);
    if (!samples) {
        fprintf(stderr, "Failed to allocate memory for audio samples: %s.\n", path);
        sf_close(file);
        exit(1);
    }

    numSamples = sf_read_short(file, samples, sfinfo.frames * sfinfo.channels);
    if (numSamples != sfinfo.frames * sfinfo.channels) {
        fprintf(stderr, "Failed to read all samples from file: %s.\n", path);
        free(samples);
        sf_close(file);
        exit(1);
    }
    freq = sfinfo.samplerate;
    sf_close(file);
    
    alBufferData(audio_context.buffers[id], format, samples, size, freq);
    checkError("Failed to fill buffer with data.");

    free(samples);
}

void audio_init(void) {
    audio_context.device = alcOpenDevice(NULL);
    if (!audio_context.device) {
        printf("Failed to open OpenAL device.\n");
        exit(1);
    }
    audio_context.context = alcCreateContext(audio_context.device, NULL);
    if (!audio_context.context) {
        printf("Failed to create OpenAL context.\n");
        alcCloseDevice(audio_context.device);
        exit(1);
    }
    alcMakeContextCurrent(audio_context.context);

    alGenBuffers(NUM_SOUNDS, audio_context.buffers);
    checkError("Failed to generate buffers.");

    load_sound(AUD_HIT, "assets/audio/hit.wav");
}

void audio_cleanup(void)
{
    alDeleteBuffers(NUM_SOUNDS, audio_context.buffers);
    checkError("Failed to delete buffers.");

    alcMakeContextCurrent(NULL);
    alcDestroyContext(audio_context.context);
    alcCloseDevice(audio_context.device);
}

