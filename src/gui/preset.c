#include "internal.h"
#include "../game.h"
#include "../state.h"
#include "../window.h"
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

static void load_preset_game(GUIComp* root)
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

static void menu_onclick(GUIComp* comp, i32 button, i32 action, i32 mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        gui_preset_load(GUI_PRESET_MAIN_MENU);
}

typedef struct {
    GUIComp* right_arrow;
    GUIComp* left_arrow;
    i32 current_mode;
} VideoSettingsData;

static const char* video_modes[] = {
    "Windowed",
    "Borderless Fullscreen",
    "Fullscreen"
};

static void video_mode_right_arrow_click(GUIComp* comp, i32 button, i32 action, i32 mods)
{
    const char* mode;
    VideoSettingsData* data = comp->parent->data;
    i32 num_modes = sizeof(video_modes) / sizeof(const char*);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (data->current_mode+1 < num_modes) {
            mode = video_modes[++data->current_mode];
            gui_comp_set_text(comp->parent, strlen(mode), mode);
        }
    }
}

static void video_mode_left_arrow_click(GUIComp* comp, i32 button, i32 action, i32 mods)
{
    const char* mode;
    VideoSettingsData* data = comp->parent->data;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (data->current_mode-1 >= 0) {
            mode = video_modes[--data->current_mode];
            gui_comp_set_text(comp->parent, strlen(mode), mode);
        }
    }
}

static void load_video_settings(GUIComp* root)
{
    GUIComp* video_mode = gui_comp_create(20, 20, 460, 30);
    const char* video_mode_text = "Windowed";
    gui_comp_set_color(video_mode, 0, 255, 0, 255);
    gui_comp_set_text_align(video_mode, ALIGN_CENTER, ALIGN_CENTER);
    gui_comp_set_text(video_mode, strlen(video_mode_text), video_mode_text);
    gui_comp_attach(root, video_mode);
    VideoSettingsData* data = st_malloc(sizeof(VideoSettingsData));
    data->current_mode = 0;
    video_mode->data = data;

    GUIComp* right_arrow = gui_comp_create(20, 0, 25, 25);
    right_arrow->click = video_mode_right_arrow_click;
    gui_comp_set_clickable(right_arrow, true);
    gui_comp_set_align(right_arrow, ALIGN_RIGHT, ALIGN_CENTER);
    gui_comp_set_color(right_arrow, 0, 0, 255, 255);
    gui_comp_attach(video_mode, right_arrow);
    data->right_arrow = right_arrow;

    GUIComp* left_arrow = gui_comp_create(20, 0, 25, 25);
    left_arrow->click = video_mode_left_arrow_click;
    gui_comp_set_clickable(left_arrow, true);
    gui_comp_set_align(left_arrow, ALIGN_LEFT, ALIGN_CENTER);
    gui_comp_set_color(left_arrow, 0, 0, 255, 255);
    gui_comp_attach(video_mode, left_arrow);
    data->left_arrow = left_arrow;
}


static void load_preset_options(GUIComp* root)
{
    GUIComp* menu = gui_comp_create(45, 50, 100, 30);
    menu->click = menu_onclick;
    const char* menu_text = "Main Menu";
    gui_comp_set_clickable(menu, true);
    gui_comp_set_color(menu, 255, 255, 255, 255);
    gui_comp_set_text_align(menu, ALIGN_CENTER, ALIGN_CENTER);
    gui_comp_set_text(menu, strlen(menu_text), menu_text);
    gui_comp_attach(root, menu);

    GUIComp* settings = gui_comp_create(45, 110, 500, 500);
    gui_comp_set_color(settings, 120, 120, 120, 255);
    load_video_settings(settings);
    gui_comp_attach(root, settings);
}

// **************************************************

static void load_save(void)
{
    game_load_starting_area();
    game_resume_loop();
}

static void save_onclick(GUIComp* comp, i32 button, i32 action, i32 mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        gui_preset_load(GUI_PRESET_GAME);
        load_save();
    }
}

static void load_preset_saves(GUIComp* root)
{
    GUIComp* save = gui_comp_create(45, 50, 100, 30);
    save->click = save_onclick;
    const char* save_text = "Save 1";
    gui_comp_set_clickable(save, true);
    gui_comp_set_color(save, 255, 255, 255, 255);
    gui_comp_set_text_align(save, ALIGN_CENTER, ALIGN_CENTER);
    gui_comp_set_text(save, strlen(save_text), save_text);
    gui_comp_attach(root, save);
}

// **************************************************

static void exit_onclick(GUIComp* comp, i32 button, i32 action, i32 mods)
{
    window_close();
}

static void play_onclick(GUIComp* comp, i32 button, i32 action, i32 mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        gui_preset_load(GUI_PRESET_SAVES); 
}

static void options_onclick(GUIComp* comp, i32 button, i32 action, i32 mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        gui_preset_load(GUI_PRESET_OPTIONS);
}

static void load_preset_main_menu(GUIComp* root)
{
    GUIComp* play = gui_comp_create(45, 50, 100, 30);
    play->click = play_onclick;
    const char* play_text = "Play";
    gui_comp_set_clickable(play, true);
    gui_comp_set_color(play, 255, 255, 255, 255);
    gui_comp_set_text_align(play, ALIGN_CENTER, ALIGN_CENTER);
    gui_comp_set_text(play, strlen(play_text), play_text);
    gui_comp_attach(root, play);

    GUIComp* options = gui_comp_create(45, 110, 100, 30);
    options->click = options_onclick;
    const char* options_text = "Options";
    gui_comp_set_clickable(options, true);
    gui_comp_set_color(options, 255, 255, 255, 255);
    gui_comp_set_text_align(options, ALIGN_CENTER, ALIGN_CENTER);
    gui_comp_set_text(options, strlen(options_text), options_text);
    gui_comp_attach(root, options);
    
    GUIComp* exit = gui_comp_create(45, 170, 100, 30);
    exit->click = exit_onclick;
    const char* exit_text = "Exit";
    gui_comp_set_clickable(exit, true);
    gui_comp_set_color(exit, 255, 255, 255, 255);
    gui_comp_set_text_align(exit, ALIGN_CENTER, ALIGN_CENTER);
    gui_comp_set_text(exit, strlen(exit_text), exit_text);
    gui_comp_attach(root, exit);
}

// **************************************************

extern GUIContext gui_context;

void gui_preset_load(GUIPreset preset)
{
    gui_comp_destroy_children(gui_context.root);
    switch (preset) {
        case GUI_PRESET_GAME:
            load_preset_game(gui_context.root); 
            break;
        case GUI_PRESET_MAIN_MENU:
            load_preset_main_menu(gui_context.root);
            break;
        case GUI_PRESET_OPTIONS:
            load_preset_options(gui_context.root);
            break;
        case GUI_PRESET_SAVES:
            load_preset_saves(gui_context.root);
            break;
        default:
            break;
    }
}
