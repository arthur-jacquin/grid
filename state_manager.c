#include <semaphore.h>
#include <stddef.h>
#include <unistd.h>

#include "client.h"
#include "pthread_queue.h"
#include "thread_management.h"
#include "types.h"

static void process_view_request(struct view_request view_request);

static void
process_view_request(struct view_request view_request)
{
    // TODO
    static int nb;
    struct cell_content cell_update;

    for (int i = 0; i < 10; i++, nb++) {
        cell_update = (struct cell_content) {
            .address = (struct address) {
                .row = nb/12,
                .col = nb%12,
            },
            .state = nb % 7 == 1,
        };
        // pthread_queue_push(&cell_updates, &cell_update);
        // sleep(1);
    }
}

void *
state_manager_routine(void *sem)
{
    while (1) {
        sem_wait(sem);
        if (should_terminate()) {
            goto cleanup;
        } else if (pthread_queue_is_non_empty(&write_requests)) {
            // TODO
            struct write_request write_request;
            pthread_queue_pop(&write_requests, &write_request);
            sleep(1); // simulate a non-trivial operation
        } else if (pthread_queue_is_non_empty(&view_requests)) {
            // TODO
            struct view_request view_request;
            pthread_queue_pop(&view_requests, &view_request);
            process_view_request(view_request);
            free(view_request.hits);
        } else if (pthread_queue_is_non_empty(&local_modifs)) {
            // TODO
            void *local_modif;
            pthread_queue_pop(&local_modifs, &local_modif);
            free(local_modif);
        }
    }

cleanup:
    return NULL;
}
