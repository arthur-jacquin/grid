#include <semaphore.h>
#include <stddef.h>
#include <unistd.h>

#include "client.h"
#include "controller.h"
#include "pthread_queue.h"
#include "thread_management.h"
#include "types.h"

void *
state_manager_routine(void *sem)
{
    struct write_request write_request;

    while (1) {
        sem_wait(sem);
        if (should_terminate()) {
            goto cleanup;
        } else if (pthread_queue_is_non_empty(&write_requests)) {
            pthread_queue_pop(&write_requests, &write_request, NULL);
            draw_cell(NULL);
            sleep(1); // simulate a non-trivial operation
        }
    }

cleanup:
    return NULL;
}

void *
sender_routine(void *sem)
{
    return NULL;
}

void *
receiver_routine(void *sem)
{
    return NULL;
}
