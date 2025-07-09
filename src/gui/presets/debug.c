#include "../internal.h"
#include "../../game.h"
#include "../../state.h"
#include <math.h>
#include <string.h>

static void keyfunc(GUIComp* comp, i32 key, i32 scancode, i32 action, i32 mods)
{
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        if (gui_event_comp_equal(GUI_COMP_TYPING, comp)) {
            game_resume_input();
            if (comp->text != NULL)
                gui_parse_command(comp->text);
            gui_comp_set_text(comp, 0, "");
            gui_set_event_comp(GUI_COMP_TYPING, NULL);
            gui_comp_set_color(comp, 255, 255, 255, 0);
        }
        else {
            game_halt_input();
            gui_comp_set_text(comp, 0, "");
            gui_set_event_comp(GUI_COMP_TYPING, comp);
            gui_comp_set_color(comp, 255, 255, 255, 200);
        }
    }
}

static void update_healthbar(GUIComp* comp, f32 dt)
{
    char buf[100];
    f32 health = game_get_boss_health();
    sprintf(buf, "%d", (int)round(health));
    gui_comp_set_text(comp, 100, buf);
}

void load_preset_debug(GUIComp* root)
{
    GUIComp* textbox = gui_comp_create(0, 50, 400, 50);
    gui_comp_set_color(textbox, 255, 255, 255, 0);
    gui_comp_set_is_text(textbox, true);
    gui_comp_set_valign(textbox, ALIGN_BOTTOM);
    gui_comp_set_font_size(textbox, 16);
    gui_comp_set_font(textbox, FONT_MONOSPACE);
    textbox->key_func = keyfunc;
    gui_comp_set_clickable(textbox, true);

    GUIComp* healthbar = gui_comp_create(0, 200, 200, 50);
    gui_comp_set_color(healthbar, 255, 255, 255, 100);
    gui_comp_set_is_text(healthbar, true);
    gui_comp_set_font_size(healthbar, 16);
    gui_comp_set_font(healthbar, FONT_MONOSPACE);
    healthbar->update_func = update_healthbar;

    GUIComp* weapon_info = gui_comp_create(0, 0, 160, 160);
    gui_comp_set_align(weapon_info, ALIGN_RIGHT, ALIGN_TOP);
    gui_comp_set_color(weapon_info, 255, 255, 255, 100);
    gui_comp_set_is_text(weapon_info, true);
    gui_comp_set_font_size(weapon_info, 16);
    gui_comp_set_font(weapon_info, FONT_MONOSPACE);
    gui_comp_set_text(weapon_info, 1, "A");
    gui_set_event_comp(GUI_COMP_WEAPON_INFO, weapon_info);

    gui_comp_attach(root, textbox);
    gui_comp_attach(root, healthbar);
    gui_comp_attach(root, weapon_info);
}
