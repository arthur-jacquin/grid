#include <semaphore.h>
#include <stddef.h>
#include <unistd.h>

#include "client.h"
#include "pthread_queue.h"
#include "thread_management.h"
#include "types.h"

void *
state_manager_routine(void *sem)
{
    int nb = 0;

    while (1) {
        sem_wait(sem);
        if (should_terminate()) {
            goto cleanup;
        } else if (pthread_queue_is_non_empty(&write_requests)) {
            // TODO
            struct write_request write_request;
            pthread_queue_pop(&write_requests, &write_request, NULL);
            sleep(1); // simulate a non-trivial operation
        } else if (pthread_queue_is_non_empty(&view_requests)) {
            // TODO
            struct view_request view_request;
            pthread_queue_pop(&view_requests, &view_request, NULL);
            free(view_request.hits);
        } else if (pthread_queue_is_non_empty(&local_modifs)) {
            // TODO
            void *local_modif;
            pthread_queue_pop(&local_modifs, NULL, (const void **) &local_modif);
            free(local_modif);
        }
    }

cleanup:
    return NULL;
}
