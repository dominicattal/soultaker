#include "../internal.h"

static void test_hover(GUIComp* comp, bool status)
{
    if (status == HOVER_OFF)
        gui_comp_set_color(comp, 255, 255, 255, 255);
    else
        gui_comp_set_color(comp, 127, 127, 127, 255);
}

static void test_hover2(GUIComp* comp, bool status)
{
    if (status == HOVER_ON)
        gui_comp_set_color(comp, 255, 0, 0, 255);
    else
        gui_comp_set_color(comp, 0, 255, 0, 255);
}

static void test_click(GUIComp* comp, i32 button, i32 action, i32 mods)
{
    gui_comp_set_color(comp, rand() % 255, rand() % 255, rand() % 255, 255);
}

void load_preset_test(GUIComp* root)
{
    GUIComp* base = gui_comp_create(30, 30, 700, 700);
    base->hover_func = test_hover;
    base->click_func = test_click;
    gui_comp_set_clickable(base, true);
    gui_comp_set_hoverable(base, true);
    gui_comp_set_color(base, 255, 255, 255, 255);
    gui_comp_attach(root, base);
    for (i32 i = 0; i < 16; i++) {
        GUIComp* comp = gui_comp_create(50, 50, 50, 50);
        gui_comp_set_color(comp, 0, 0, 255, 255);
        gui_comp_set_align(comp, i % 4, i / 4);
        gui_comp_attach(base, comp);
        gui_comp_set_hoverable(comp, true);
        gui_comp_set_clickable(comp, true);
        comp->hover_func = test_hover2;
        comp->click_func = test_click;
        GUIComp* comp2 = gui_comp_create(0, 0, 4, 4);
        gui_comp_set_align(comp2, ALIGN_CENTER, ALIGN_CENTER);
        gui_comp_set_color(comp2, 255, 255, 255, 255);
        gui_comp_attach(comp, comp2);
    }

    GUIComp* textbox = gui_comp_create(0, 30, 250, 700);
    gui_comp_set_halign(textbox, ALIGN_RIGHT);
    gui_comp_set_is_text(textbox, true);
    gui_comp_set_text(textbox, 100, "The quick brown fox jumps over the lazy dog");
    gui_comp_set_color(textbox, 255, 255, 255, 255);
    gui_comp_set_font_size(textbox, 16);
    gui_comp_set_font(textbox, FONT_MOJANGLES);
    gui_comp_attach(root, textbox);
}
