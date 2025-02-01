#include "../renderer.h"
#include "../gui.h"
#include "../game.h"

void renderer_render(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.5f, 0.5f, 0.7f, 1.0f);

    gui_render();
    game_render();
}
