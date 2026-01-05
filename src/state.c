#include "state.h"
#include "event.h"
#include "window.h"
#include "renderer.h"
#include "game.h"
#include "gui.h"
#include "audio.h"
#include <stdio.h>
#include <pthread.h>
#include <json.h>

#ifdef DEBUG_BUILD
    #define BUILD_INFO "DEBUG"
#elif RELEASE_BUILD
    #define BUILD_INFO "RELEASE"
#endif

#ifdef _WIN32
#include <windows.h>
const char* pathname = "plugins/soultaker.dll";
const int flags = 0;
void* dlopen(const char* path, i32 flags)
{
    return LoadLibrary(path);
}
void* dlsym(void* handle, const char* symbol)
{
    return GetProcAddress(handle, symbol);
}
char* dlerror(void)
{
    return "could not load library";
}
int dlclose(void* handle)
{
    return FreeLibrary(handle);
}
#else
#include <dlfcn.h>
const char* pathname = "bin/dev/plugins/soultaker.dll";
const int flags = RTLD_NOW;
#endif

struct {
    void* handle;
} state_context;

void state_init(void)
{
    thread_link("Main");

    state_context.handle = dlopen(pathname, flags);
    if (!state_context.handle)
        log_write(FATAL, dlerror());

    event_init();
    window_init();
    renderer_init();
    gui_init();
    game_init();
}

void state_loop(void)
{
    //f64 start, end, dt;
    //start = get_time();
    while (!window_closed())
    {
        window_update();
        game_process_input();
        renderer_render();
        //end = get_time();
        //dt = end - start;
        //start = end;
    }
}

void state_cleanup(void)
{
    // add signals so game doesnt cleanup b4 gui finishes with it
    log_unlock();
    game_cleanup();
    gui_cleanup();
    renderer_cleanup();
    window_cleanup();
    event_cleanup();
#ifdef DEBUG_BUILD
    print_heap_info();
#endif
    log_cleanup();

    dlclose(state_context.handle);
}

void* state_load_function(const char* name)
{
    return dlsym(state_context.handle, name);
}
