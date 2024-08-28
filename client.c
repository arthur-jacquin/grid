#include <stddef.h>

#include "clic.h"
#include "pthread_queue.h"
#include "thread_management.h"
#include "types.h"

struct pthread_queue
    area_requests = PTHREAD_QUEUE_INITIALIZER(STATE_MANAGER,
        sizeof(struct area_request)),
    approved_modifs = PTHREAD_QUEUE_INITIALIZER(STATE_MANAGER, 0),
    cell_updates = PTHREAD_QUEUE_INITIALIZER(CACHE_MANAGER,
        sizeof(struct cell_content)),
    cursor_pos = PTHREAD_QUEUE_INITIALIZER(SENDER, sizeof(struct cursor_pos)),
    local_modifs = PTHREAD_QUEUE_INITIALIZER(STATE_MANAGER, 0),
    modif_attempts = PTHREAD_QUEUE_INITIALIZER(SENDER, 0),
    validations = PTHREAD_QUEUE_INITIALIZER(STATE_MANAGER,
        sizeof(struct validation)),
    write_requests = PTHREAD_QUEUE_INITIALIZER(STATE_MANAGER,
        sizeof(struct write_request));

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
