#include "internal.h"
#include "../renderer.h"
#include <string.h>

#define FLOATS_PER_COMP 13

GUIContext gui_context;

void align_comp_position_x(i32* position_x, u8 halign, i32 size_x, i32 x, i32 w)
{
    if      (halign == ALIGN_LEFT)       *position_x += x;
    else if (halign == ALIGN_CENTER_POS) *position_x += (size_x - w) / 2 + x;
    else if (halign == ALIGN_CENTER_NEG) *position_x += (size_x - w) / 2 - x;
    else if (halign == ALIGN_RIGHT)      *position_x += size_x - w - x;
}

void align_comp_position_y(i32* position_y, u8 valign, i32 size_y, i32 y, i32 h)
{
    if      (valign == ALIGN_TOP)        *position_y += y;
    else if (valign == ALIGN_CENTER_POS) *position_y += (size_y - h) / 2 + y;
    else if (valign == ALIGN_CENTER_NEG) *position_y += (size_y - h) / 2 - y;
    else if (valign == ALIGN_BOTTOM)     *position_y += size_y - h - y;
}

static void resize_data_buffer(i32 num_comps)
{
    gui_context.data_swap.capacity += FLOATS_PER_COMP * num_comps;
    if (gui_context.data_swap.buffer == NULL) {
        gui_context.data_swap.buffer = malloc(gui_context.data_swap.capacity * sizeof(GLfloat));
        assert(gui_context.data_swap.buffer != NULL);
    } else {
        GLfloat* buf = realloc(gui_context.data_swap.buffer, gui_context.data_swap.capacity * sizeof(GLfloat)); 
        assert(buf != NULL);
        gui_context.data_swap.buffer = buf;
    }
}

static void push_text_data(GUIComp* comp, i32 pos_x, i32 pos_y)
{
    if (comp->text == NULL)
        return;

    i32 cx, cy, cw, ch;     // comp position and size
    f32 u1, v1, u2, v2;     // bitmap coordinates
    i32 a1, b1, a2, b2;     // glyph bounding box
    i32 x, y, w, h;         // pixel coordinates
    i32 ascent, descent;    // highest and lowest glyph offsets
    i32 line_gap;           // gap between lines
    i32 adv, lsb, kern;     // advance, left side bearing, kerning
    u8  ha, va;             // horizontal and vertical alignment
    u8  justify;            // branchless justify
    i32 font_size;          // font_size = ascent - descent
    FontEnum font;          // font
    i32 num_spaces;         // count whitespace for horizontal alignment
    f32 dy;                 // change in y for vertical alignment
    i32 vbo_idx;            // vbo index of first glyph
    i32 length;             // index in text, length of text
    char* text;             // text, equal to comp->text
    i32 location;           // active texture slot of bitmap

    register i32 ox, oy, test_ox;    // glyph origin
    register i32 prev_test_ox;       // edge case
    register i32 left, right, mid;   // pointers for word
    
    gui_comp_get_position(comp, &cx, &cy);
    gui_comp_get_size(comp, &cw, &ch);
    gui_comp_get_text_align(comp, &ha, &va);
    gui_comp_get_font(comp, &font);
    gui_comp_get_font_size(comp, &font_size);

    text = comp->text;
    length = strlen(text);
    
    justify = 0;
    if (ha == ALIGN_JUSTIFY) {
        ha = ALIGN_LEFT;
        justify = 1;
    }
    
    font_info(font, font_size, &ascent, &descent, &line_gap, &location);
    
    left = right = 0;
    ox = 0;
    oy = ascent;
    resize_data_buffer(length);
    vbo_idx = gui_context.data_swap.length;
    while (right < length) {
        
        while (right < length && (text[right] == ' ' || text[right] == '\t' || text[right] == '\n'))
            right++;

        left = right;
        prev_test_ox = test_ox = 0;
        num_spaces = 0;
        while (right < length && text[right] != '\n' && test_ox <= cw) {
            font_char_hmetrics(font, font_size, text[right], &adv, &lsb);
            font_char_kern(font, font_size, text[right], text[right+1], &kern);
            prev_test_ox = test_ox;
            test_ox += adv + kern;
            num_spaces += text[right] == ' ';
            right++;
        }

        mid = right;
        if (test_ox > cw) {
            while (mid > left && text[mid-1] != ' ') {
                font_char_hmetrics(font, font_size, text[mid-1], &adv, &lsb);
                font_char_kern(font, font_size, text[mid-1], text[mid], &kern);
                test_ox -= adv + kern;
                mid--;
            }
            while (mid > left && text[mid-1] == ' ') {
                font_char_hmetrics(font, font_size, text[mid-1], &adv, &lsb);
                font_char_kern(font, font_size, text[mid-1], text[mid], &kern);
                test_ox -= adv + kern;
                num_spaces -= text[mid-1] == ' ';
                mid--;
            }
        }

        if (mid == left) {
            if (left == right) {
                test_ox = cw;
                right = left + 1;
            } else {
                test_ox = prev_test_ox;
                right = right - 1;
            }
        }
        else {
            right = mid;
        }

        if (left == right)
            right++;

        if (text[right-1] != ' ') {
            font_char_hmetrics(font, font_size, text[right-1], &adv, &lsb);
            font_char_bbox(font, font_size, text[right-1], &a1, &b1, &a2, &b2);
            test_ox -= adv - (a2 + a1);
        }
        
        ox = ha * (cw - test_ox) / 2.0f;

        while (left < right) {
            font_char_hmetrics(font, font_size, text[left], &adv, &lsb);
            font_char_bbox(font, font_size, text[left], &a1, &b1, &a2, &b2);
            font_char_bmap(font, font_size, text[left], &u1, &v1, &u2, &v2);
            font_char_kern(font, font_size, text[left], text[left+1], &kern);

            x = cx + ox + lsb;
            y = cy + ch - oy - b2;
            w = a2 - a1;
            h = b2 - b1;

            if (text[left] != '\0' && text[left] != ' ') {
                #define A gui_context.data_swap.buffer[gui_context.data_swap.length++]
                A = x; A = y; A = w; A = h;
                A = 255; A = 255; A = 255; A = 255;
                A = u1; A = v1; A = u2; A = v2;
                A = location;
                gui_context.data_swap.instance_count++;
                #undef A
            }   

            ox += adv + kern;
            if (justify && text[left] == ' ')
                ox += (cw - test_ox) / num_spaces;

            left++;
        }

        oy += ascent - descent + line_gap;
    }

    if (va == ALIGN_TOP)
        return;

    oy -= ascent - descent + line_gap;
    dy = va * (ch - oy) / 2;
//    window_pixel_to_screen_y(va * (ch - oy) / 2, &dy);

    while (vbo_idx < gui_context.data_swap.length) {
        gui_context.data_swap.buffer[vbo_idx + 1] -= dy;
        vbo_idx += FLOATS_PER_COMP;
    }
}
static void push_comp_data(GUIComp* comp, i32 x, i32 y)
{
    i32 w, h, loc;
    f32 u1, v1, u2, v2;
    u8 r, g, b, a;
    if (gui_context.data_swap.length >= gui_context.data_swap.capacity)
        resize_data_buffer(5);

    gui_comp_get_size(comp, &w, &h);
    gui_comp_get_color(comp, &r, &g, &b, &a);
    texture_info(gui_comp_tex(comp), &u1, &v1, &u2, &v2, &loc);

    #define A gui_context.data_swap.buffer[gui_context.data_swap.length++]
    A = x; A = y; A = w; A = h;
    A = r; A = g; A = b; A = a;
    A = u1; A = v1; A = u2; A = v2;
    A = loc;
    #undef A

    gui_context.data_swap.instance_count++;

    if (gui_comp_is_text(comp))
        push_text_data(comp, x, y);
}

static void gui_update_helper(GUIComp* comp, i32 position_x, i32 position_y, i32 size_x, i32 size_y)
{
    i32 x, y, w, h;
    u8 halign, valign;
    gui_comp_get_bbox(comp, &x, &y, &w, &h);
    gui_comp_get_align(comp, &halign, &valign);
    
    align_comp_position_x(&position_x, halign, size_x, x, w);
    align_comp_position_y(&position_y, valign, size_y, y, h);

    push_comp_data(comp, position_x, position_y);
    for (i32 i = 0; i < gui_comp_num_children(comp); i++)
        gui_update_helper(comp->children[i], position_x, position_y, w, h);
}

static void gui_update(void)
{
    gui_context.data_swap.instance_count = 0;
    gui_context.data_swap.length = 0;
    gui_update_helper(gui_context.root, 0, 0, 0, 0);

    GUIData tmp;
    pthread_mutex_lock(&gui_context.mutex);
    tmp = gui_context.data;
    gui_context.data = gui_context.data_swap;
    gui_context.data_swap = tmp;
    pthread_mutex_unlock(&gui_context.mutex);
}

static void* gui_loop(void* vargp)
{
    gui_comp_init();
    while (!gui_context.kill_thread)
    {
        gui_update();
        sleep(20);
    }
    gui_comp_cleanup();
    return NULL;
}

void gui_init(void)
{
    gui_render_init();
    pthread_mutex_init(&gui_context.mutex, NULL);
    pthread_create(&gui_context.thread_id, NULL, gui_loop, NULL);
}

void gui_cleanup(void)
{
    gui_context.kill_thread = true;
    pthread_join(gui_context.thread_id, NULL);
    pthread_mutex_destroy(&gui_context.mutex);
    gui_render_cleanup();
}

