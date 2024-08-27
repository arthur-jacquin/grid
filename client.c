#include <stddef.h>

#include "clic.h"
#include "thread_management.h"

int
main(int argc, char *argv[])
{
    int exit_status;

    capture_signals();

    // parse command line arguments
    clic_init("grid-client", VERSION, "GPLv3", "spreadsheet editor", 0, 0);
    // TODO
    clic_parse(argc, (const char **) argv, NULL);

    // init
    // TODO

    // spawn and join threads
    spawn_threads();
    exit_status = join_threads();

    // deinit
    // TODO
    return exit_status;
}
