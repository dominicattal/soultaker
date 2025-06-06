#include "state.h"

int main(int argc, char** argv)
{
    atexit(state_cleanup);
    log_init(argv[0]);
    state_init();
    state_loop();
    return 0;
}
