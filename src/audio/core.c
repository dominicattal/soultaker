#include "../audio.h"
#include <stdio.h>

AudioContext audio_context;

static void checkError(const char* msg) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR)
        log_write(FATAL, "OpenAL error (%X) %s", error, msg);
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
    if (!file)
        log_write(FATAL, "Failed to open sound file %s", path);

    format = 0;
    if (sfinfo.channels == 1) {
        format = AL_FORMAT_MONO16;
    } else if (sfinfo.channels == 2) {
        format = AL_FORMAT_STEREO16;
    } else {
        sf_close(file);
        log_write(FATAL, "Unsupported number of channels %s", path);
    }

    size = sfinfo.frames * sfinfo.channels * sizeof(ALshort);
    samples = malloc(size);
    if (!samples) {
        sf_close(file);
        log_write(FATAL, "Failed to allocate memory for audio samples %s", path);
    }

    numSamples = sf_read_short(file, samples, sfinfo.frames * sfinfo.channels);
    if (numSamples != sfinfo.frames * sfinfo.channels) {
        free(samples);
        sf_close(file);
        log_write(FATAL, "Failed to read all samples from file %s", path);
    }
    freq = sfinfo.samplerate;
    sf_close(file);
    
    alBufferData(audio_context.buffers[id], format, samples, size, freq);
    checkError("Failed to fill buffer with data.");

    free(samples);
}

static void load_sounds(void)
{
    load_sound(AUD_HIT, "assets/audio/hit.wav");
}

void audio_init(void) {
    log_write(INFO, "Initializing audio...");
    audio_context.device = alcOpenDevice(NULL);
    if (!audio_context.device)
        log_write(FATAL, "Failed to open OpenAL device");

    audio_context.context = alcCreateContext(audio_context.device, NULL);
    if (!audio_context.context) {
        printf("Failed to create OpenAL context.\n");
        log_write(FATAL, "Failed to create OpenAL context");
    }
    alcMakeContextCurrent(audio_context.context);

    alGenBuffers(NUM_SOUNDS, audio_context.buffers);
    checkError("Failed to generate buffers.");

    load_sounds();

    log_write(INFO, "Initialized audio");
}

void audio_cleanup(void)
{
    log_write(INFO, "Cleaning up audio...");
    alDeleteBuffers(NUM_SOUNDS, audio_context.buffers);
    checkError("Failed to delete buffers.");

    alcMakeContextCurrent(NULL);
    alcDestroyContext(audio_context.context);
    alcCloseDevice(audio_context.device);
    log_write(INFO, "Cleaned up audio");
}

