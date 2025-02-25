#include "../internal.h"

void load_preset_main_menu(GUIComp* root)
{
    GUIComp* test = gui_comp_create(0, 0, 200, 75);
    gui_comp_set_color(test, 255, 255, 255, 255);
    gui_comp_set_valign(test, ALIGN_TOP);
    gui_comp_attach(root, test);

    GUIComp* test2 = gui_comp_create(0, 0, 50, 50);
    gui_comp_set_color(test2, 0, 0, 255, 255);
    gui_comp_set_align(test2, ALIGN_CENTER, ALIGN_CENTER);
    gui_comp_attach(test, test2);
}
