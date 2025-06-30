#include "state.h"

int main(int argc, char** argv)
{
    log_init(argv[0]);
    state_init();
    state_loop();
    state_cleanup();
    return 0;
}
