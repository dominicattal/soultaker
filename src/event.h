#ifndef EVENT_H
#define EVENT_H

#include "game.h"
#include "gui.h"
#include "renderer.h"
#include "util.h"
#include "window.h"

// Events provide a thread-safe way of communicating
// between threads. Each thread may enqueue an event in
// any other thread and may flush their own event queue
// when they want. 

// init all event related things
void event_init(void);
void event_cleanup(void);

// Flush all of the events in the calling
// thread's event queue.
void event_queue_flush(void);

// Game events
void event_create_game_framebuffer_size_callback(void);
void event_create_game_change_map(i32 id);

// Gui events
void event_create_gui_framebuffer_size_callback(u32 width, u32 height);
void event_create_gui_scroll_callback(f64 xoffset, f64 yoffset);
void event_create_gui_mouse_button_callback(i32 button, i32 action, i32 mods);
void event_create_gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
void event_create_gui_control_callback(ControlEnum ctrl, i32 action);
void event_create_gui_char_callback(i32 codepoint);

// Renderer events
void event_create_renderer_write_texture_units(void);

#endif
