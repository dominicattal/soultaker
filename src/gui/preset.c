#include "internal.h"

extern GUIContext gui_context;

void gui_preset_load(GUIPreset preset)
{
    gui_comp_destroy_children(gui_context.root);
    switch (preset) {
        case GUI_PRESET_TEST:
            load_preset_test(gui_context.root); break;
        case GUI_PRESET_MAIN_MENU:
            load_preset_main_menu(gui_context.root); break;
        case GUI_PRESET_DEBUG:
            load_preset_debug(gui_context.root); break;
        default:
            break;
    }
}
