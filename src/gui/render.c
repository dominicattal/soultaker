#include "internal.h"
#include "../renderer.h"
#include <string.h>

#define FLOATS_PER_COMP 13

typedef struct {
    i32 x, y, w, h;
    u8 r, g, b, a;
    f32 u1, v1, u2, v2;
    i32 location;
} Quad;

extern GUIContext gui_context;
static GLuint s_vao, s_vbo, s_instance_vbo;

static void resize_data_buffer(i32 num_comps)
{
    size_t size;
    gui_context.data_swap.capacity += FLOATS_PER_COMP * (num_comps+100);
    size = gui_context.data_swap.capacity * sizeof(GLfloat);
    if (gui_context.data_swap.buffer == NULL) {
        gui_context.data_swap.buffer = st_malloc(size);
        assert(gui_context.data_swap.buffer != NULL);
    } else {
        GLfloat* buf = st_realloc(gui_context.data_swap.buffer, size); 
        assert(buf != NULL);
        gui_context.data_swap.buffer = buf;
    }
}

static void push_quad_data(Quad* quad)
{
    if (gui_context.data_swap.length + FLOATS_PER_COMP >= gui_context.data_swap.capacity)
        resize_data_buffer(1);
    gui_context.data_swap.buffer[gui_context.data_swap.length++] = quad->x;
    gui_context.data_swap.buffer[gui_context.data_swap.length++] = quad->y;
    gui_context.data_swap.buffer[gui_context.data_swap.length++] = quad->w;
    gui_context.data_swap.buffer[gui_context.data_swap.length++] = quad->h;
    gui_context.data_swap.buffer[gui_context.data_swap.length++] = quad->r;
    gui_context.data_swap.buffer[gui_context.data_swap.length++] = quad->g;
    gui_context.data_swap.buffer[gui_context.data_swap.length++] = quad->b;
    gui_context.data_swap.buffer[gui_context.data_swap.length++] = quad->a;
    gui_context.data_swap.buffer[gui_context.data_swap.length++] = quad->u1;
    gui_context.data_swap.buffer[gui_context.data_swap.length++] = quad->v1;
    gui_context.data_swap.buffer[gui_context.data_swap.length++] = quad->u2;
    gui_context.data_swap.buffer[gui_context.data_swap.length++] = quad->v2;
    gui_context.data_swap.buffer[gui_context.data_swap.length++] = quad->location;
    gui_context.data_swap.instance_count++;
}

static void push_text_data(GUIComp* comp, i32 cx, i32 cy, i32 cw, i32 ch)
{
    if (comp->text == NULL)
        return;

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
    Quad quad;

    register i32 ox, oy, test_ox;    // glyph origin
    register i32 prev_test_ox;       // edge case
    register i32 left, right, mid;   // pointers for word
    
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
                quad.x = x;
                quad.y = y;
                quad.w = w;
                quad.h = h;
                quad.r = 255;
                quad.g = 255;
                quad.b = 255;
                quad.a = 255;
                quad.u1 = u1;
                quad.v1 = v1;
                quad.u2 = u2;
                quad.v2 = v2;
                quad.location = location;
                push_quad_data(&quad);
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

    while (vbo_idx < gui_context.data_swap.length) {
        gui_context.data_swap.buffer[vbo_idx + 1] -= dy;
        vbo_idx += FLOATS_PER_COMP;
    }
}

static void push_comp_data(GUIComp* comp, i32 x, i32 y, i32 w, i32 h)
{
    Quad quad;
    i32 loc;
    f32 u, v, du, dv;
    vec2 pivot, stretch;
    u8 r, g, b, a;

    gui_comp_get_color(comp, &r, &g, &b, &a);
    texture_info(gui_comp_tex(comp), &loc, &u, &v, &du, &dv, &pivot, &stretch);

    quad.x = x; quad.y = y; quad.w = w; quad.h = h;
    quad.r = r; quad.g = g; quad.b = b; quad.a = a;
    quad.u1 = u; quad.v1 = v; quad.u2 = u+du; quad.v2 = v+dv;
    quad.location = loc;

    push_quad_data(&quad);

    if (gui_comp_is_text(comp))
        push_text_data(comp, x, y, w, h);
}

static void gui_update_vertex_data_helper(GUIComp* comp, i32 position_x, i32 position_y, i32 size_x, i32 size_y)
{
    i32 x, y, w, h;
    u8 halign, valign;
    gui_comp_get_bbox(comp, &x, &y, &w, &h);
    gui_comp_get_align(comp, &halign, &valign);
    
    align_comp_position_x(&position_x, halign, size_x, x, w);
    align_comp_position_y(&position_y, valign, size_y, y, h);

    push_comp_data(comp, position_x, position_y, w, h);
    for (i32 i = 0; i < gui_comp_num_children(comp); i++)
        gui_update_vertex_data_helper(comp->children[i], position_x, position_y, w, h);
}

void gui_update_vertex_data(void)
{
    gui_context.data_swap.instance_count = 0;
    gui_context.data_swap.length = 0;
    gui_update_vertex_data_helper(gui_context.root, 0, 0, 0, 0);

    GUIData tmp;
    pthread_mutex_lock(&gui_context.data_mutex);
    tmp = gui_context.data;
    gui_context.data = gui_context.data_swap;
    gui_context.data_swap = tmp;
    pthread_mutex_unlock(&gui_context.data_mutex);
}


void gui_render_init(void)
{
    log_write(INFO, "Initializing GUI buffers...");
    glGenVertexArrays(1, &s_vao);
    glGenBuffers(1, &s_vbo);
    glGenBuffers(1, &s_instance_vbo);

    GLfloat vertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };

    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, s_instance_vbo);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void*)(12 * sizeof(GLfloat)));
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    log_write(INFO, "Initialized GUI buffers");
}

void gui_render(void)
{
    if (gui_context.data.buffer == NULL)
        return;

    shader_use(SHADER_PROGRAM_GUI);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_instance_vbo);

    pthread_mutex_lock(&gui_context.data_mutex);
    GLsizei instance_count = gui_context.data.instance_count;
    glBufferData(GL_ARRAY_BUFFER, gui_context.data.length * sizeof(GLfloat), gui_context.data.buffer, GL_DYNAMIC_DRAW);
    pthread_mutex_unlock(&gui_context.data_mutex);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instance_count);
}

void gui_render_cleanup(void)
{
    log_write(INFO, "Deleting GUI buffers...");
    if (s_vao != 0)
        glDeleteVertexArrays(1, &s_vao);
    if (s_vbo != 0)
        glDeleteBuffers(1, &s_vbo);
    if (s_instance_vbo != 0)
        glDeleteBuffers(1, &s_instance_vbo);
    log_write(INFO, "Deleted GUI buffers");
}

