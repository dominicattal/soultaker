#include "state.h"

int main(int argc, char** argv)
{
    state_init();
    state_loop();
    state_cleanup();
    return 0;
}
