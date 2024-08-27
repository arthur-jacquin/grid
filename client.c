#include "thread_management.h"

int
main(int argc, char *argv[])
{
    int exit_status;

    capture_signals();

    // parse command line arguments
    // TODO

    // init
    // TODO

    // spawn and join threads
    spawn_threads();
    exit_status = join_threads();

    // deinit
    // TODO
    return exit_status;
}
