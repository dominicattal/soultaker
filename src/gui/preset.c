#include "internal.h"
#include "../game.h"
#include "../state.h"
#include <math.h>
#include <string.h>

// **************************************************

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

#define STAT_POINT_WIDTH 350

static void update_player_health(GUIComp* comp, f32 dt)
{
    GUIComp* current_health = comp->children[0];
    f32 health = player_health();
    f32 max_health = player_max_health();
    i32 width = (i32)round(STAT_POINT_WIDTH * health / max_health);
    gui_comp_set_w(current_health, width);
}

static void update_player_mana(GUIComp* comp, f32 dt)
{
    GUIComp* current_mana = comp->children[0];
    f32 mana = player_mana();
    f32 max_mana = player_max_mana();
    i32 width = (i32)round(STAT_POINT_WIDTH * mana / max_mana);
    gui_comp_set_w(current_mana, width);
}

static void update_boss_health(GUIComp* comp, f32 dt)
{
    GUIComp* current_health = comp->children[0];
    f32 health = game_get_boss_health();
    f32 max_health = game_get_boss_max_health();
    i32 width = (i32)round(STAT_POINT_WIDTH * health / max_health);
    gui_comp_set_w(current_health, width);
}

static void update_player_souls(GUIComp* comp, f32 dt)
{
    GUIComp* current_souls = comp->children[0];
    f32 souls = player_souls();
    f32 max_souls = player_max_souls();
    i32 width = (i32)round(STAT_POINT_WIDTH * souls / max_souls);
    gui_comp_set_w(current_souls, width);
}

static GUIComp* create_player_health(void)
{
    GUIComp* player_health = gui_comp_create(0, 0, STAT_POINT_WIDTH, 20);
    player_health->update= update_player_health;
    gui_comp_set_color(player_health, 255, 0, 0, 255);

    GUIComp* current_health = gui_comp_create(0, 0, STAT_POINT_WIDTH, 20);
    gui_comp_set_color(current_health, 0, 255, 0, 255);

    GUIComp* hp_text = gui_comp_create(0, 0, STAT_POINT_WIDTH, 20);
    gui_comp_set_text_align(hp_text, ALIGN_CENTER, ALIGN_CENTER);
    gui_comp_set_color(hp_text, 0, 0, 0, 0);
    gui_comp_set_text(hp_text, 2, "HP");
    gui_comp_set_font_size(hp_text, 16);
    gui_comp_set_font(hp_text, FONT_MONOSPACE);

    gui_comp_attach(player_health, current_health);
    gui_comp_attach(player_health, hp_text);

    return player_health;
}

static GUIComp* create_player_mana(void)
{
    GUIComp* player_mana = gui_comp_create(0, 20, STAT_POINT_WIDTH, 20);
    player_mana->update = update_player_mana;
    gui_comp_set_color(player_mana, 255, 255, 0, 255);

    GUIComp* current_mana = gui_comp_create(0, 0, STAT_POINT_WIDTH, 20);
    gui_comp_set_color(current_mana, 0, 0, 255, 255);

    GUIComp* mp_text = gui_comp_create(0, 0, STAT_POINT_WIDTH, 20);
    gui_comp_set_text_align(mp_text, ALIGN_CENTER, ALIGN_CENTER);
    gui_comp_set_color(mp_text, 0, 0, 0, 0);
    gui_comp_set_text(mp_text, 2, "MP");
    gui_comp_set_font_size(mp_text, 16);
    gui_comp_set_font(mp_text, FONT_MONOSPACE);

    gui_comp_attach(player_mana, current_mana);
    gui_comp_attach(player_mana, mp_text);

    return player_mana;
}

static GUIComp* create_player_souls(void)
{
    GUIComp* player_souls = gui_comp_create(0, 40, STAT_POINT_WIDTH, 20);
    player_souls->update = update_player_souls;
    gui_comp_set_color(player_souls, 200, 200, 200, 255);

    GUIComp* current_souls = gui_comp_create(0, 0, STAT_POINT_WIDTH, 20);
    gui_comp_set_color(current_souls, 255, 255, 255, 255);

    GUIComp* sp_text = gui_comp_create(0, 0, STAT_POINT_WIDTH, 20);
    gui_comp_set_text_align(sp_text, ALIGN_CENTER, ALIGN_CENTER);
    gui_comp_set_color(sp_text, 0, 0, 0, 0);
    gui_comp_set_text(sp_text, 2, "SP");
    gui_comp_set_font_size(sp_text, 16);
    gui_comp_set_font(sp_text, FONT_MONOSPACE);

    gui_comp_attach(player_souls, current_souls);
    gui_comp_attach(player_souls, sp_text);

    return player_souls;
}

static GUIComp* create_boss_health()
{
    GUIComp* boss_health = gui_comp_create(0, 0, STAT_POINT_WIDTH, 20);
    boss_health->update= update_boss_health;
    gui_comp_set_align(boss_health, ALIGN_RIGHT, ALIGN_TOP);
    gui_comp_set_color(boss_health, 255, 0, 0, 255);

    GUIComp* current_health = gui_comp_create(0, 0, STAT_POINT_WIDTH, 20);
    gui_comp_set_align(current_health, ALIGN_RIGHT, ALIGN_TOP);
    gui_comp_set_color(current_health, 0, 255, 0, 255);

    GUIComp* hp_text = gui_comp_create(0, 0, STAT_POINT_WIDTH, 20);
    gui_comp_set_align(hp_text, ALIGN_RIGHT, ALIGN_TOP);
    gui_comp_set_text_align(hp_text, ALIGN_CENTER, ALIGN_CENTER);
    gui_comp_set_color(hp_text, 0, 0, 0, 0);
    gui_comp_set_text(hp_text, 2, "HP");
    gui_comp_set_font_size(hp_text, 16);
    gui_comp_set_font(hp_text, FONT_MONOSPACE);

    gui_comp_attach(boss_health, current_health);
    gui_comp_attach(boss_health, hp_text);

    return boss_health;
}

static void load_preset_debug(GUIComp* root)
{
    GUIComp* player_stats = gui_comp_create(20, 20, 400, 50);
    gui_comp_set_color(player_stats, 0, 0, 0, 0);

    GUIComp* player_health = create_player_health();
    GUIComp* player_mana = create_player_mana();
    GUIComp* player_souls = create_player_souls();

    gui_comp_attach(player_stats, player_mana);
    gui_comp_attach(player_stats, player_health);
    gui_comp_attach(player_stats, player_souls);
    gui_comp_attach(root, player_stats);

    GUIComp* boss_stats = gui_comp_create(20, 20, 400, 50);
    gui_comp_set_align(boss_stats, ALIGN_RIGHT, ALIGN_TOP);
    
    GUIComp* boss_health = create_boss_health();

    gui_comp_attach(boss_stats, boss_health);
    gui_comp_attach(root, boss_stats);

    GUIComp* textbox = gui_comp_create(0, 50, 400, 50);
    gui_comp_set_color(textbox, 255, 255, 255, 0);
    gui_comp_set_valign(textbox, ALIGN_BOTTOM);
    gui_comp_set_font_size(textbox, 16);
    gui_comp_set_font(textbox, FONT_MONOSPACE);
    textbox->key = keyfunc;
    gui_comp_set_clickable(textbox, true);
    gui_comp_attach(root, textbox);

    GUIComp* weapon_info = gui_comp_create(20, 80, 52, 52);
    gui_comp_set_color(weapon_info, 199, 199, 199, 255);
    GUIComp* weapon_tex = gui_comp_create(2, 2, 48, 48);
    gui_comp_set_color(weapon_tex, 255, 255, 255, 255);
    gui_set_event_comp(GUI_COMP_WEAPON_INFO, weapon_tex);
    gui_comp_attach(weapon_info, weapon_tex);
    gui_comp_attach(root, weapon_info);
}

// **************************************************

extern GUIContext gui_context;

void gui_preset_load(GUIPreset preset)
{
    gui_comp_destroy_children(gui_context.root);
    switch (preset) {
        case GUI_PRESET_DEBUG:
            load_preset_debug(gui_context.root); break;
        default:
            break;
    }
}
