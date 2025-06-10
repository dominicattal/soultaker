#include "../internal.h"
#include "../../game.h"
#include "../../state.h"
#include <string.h>

typedef struct {
    f32 timer;
} CompData;

static CompData* create_comp_data(void)
{
    CompData* data = st_malloc(sizeof(CompData));
    data->timer = 0;
    return data;
}

static void update_info(GUIComp* comp, f32 dt)
{
    CompData* data = comp->data;
    data->timer -= dt;
    if (data->timer < 0) {
        data->timer = 0.5;
        vec3 position = camera_get_position();
        vec3 facing = camera_get_facing();
        f32 pitch = camera_get_pitch();
        f32 zoom = camera_get_zoom();
        f32 yaw = camera_get_yaw();
        char string[400];
        char buf[100];
        for (i32 i = 0; i < 300; i++)
            string[i] = '\0';
        for (i32 i = 0; i < 100; i++)
            buf[i] = '\0';
        sprintf(buf, "position: (%.3f %.3f %.3f)\n", position.x, position.y, position.z);
        strncpy(string, buf, 100);
        sprintf(buf, "facing: (%.3f %.3f %.3f)\n", facing.x, facing.y, facing.z);
        strncat(string, buf, 100);
        sprintf(buf, "pitch: %-7.3f yaw: %-7.3f zoom: %-7.3f\n", pitch, yaw, zoom);
        strncat(string, buf, 100);
        sprintf(buf, "Main: %-6.3f GUI: %-6.3f Game: %-6.3f\n", global_context.dt * 1000, gui_dt() * 1000, game_dt() * 1000);
        strncat(string, buf, 100);
        string[399] = '\n';
        gui_comp_set_text(comp, 400, string);
    }
}

void load_preset_debug(GUIComp* root)
{
    GUIComp* info = gui_comp_create(0, 0, 400, 70);
    gui_comp_set_color(info, 255, 255, 255, 100);
    gui_comp_set_is_text(info, true);
    gui_comp_set_font_size(info, 16);
    gui_comp_set_font(info, FONT_MONOSPACE);
    info->update_func = update_info;
    info->data = create_comp_data(); 

    GUIComp* textbox = gui_comp_create(0, 50, 400, 70);
    gui_comp_set_color(textbox, 255, 255, 255, 100);
    gui_comp_set_is_text(textbox, true);
    gui_comp_set_valign(textbox, ALIGN_BOTTOM);
    gui_comp_set_font_size(textbox, 16);
    gui_comp_set_font(textbox, FONT_MONOSPACE);
    gui_comp_set_text(textbox, 30, "testing 123");
    gui_set_typing_comp(textbox);

    gui_comp_attach(root, info);
    gui_comp_attach(root, textbox);
}
