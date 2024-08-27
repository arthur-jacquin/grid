#include <semaphore.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include "client.h"
#include "pthread_queue.h"
#include "thread_management.h"
#include "types.h"

void *
controller_routine(void *sem)
{
    struct write_request write_request;

    sem_init(sem, 0, 0);
    declare_as_initialized();
    if (should_terminate()) {
        goto cleanup;
    }

    // simulate some write requests, and exit
    for (int i = 0; i < 10; i++) {
        if (should_terminate()) {
            goto cleanup;
        }
        write_request.id = i;
        pthread_queue_push(&write_requests, &write_request);
        sleep(1);
    }
    request_termination(0);

cleanup:
    return NULL;
}

void *
state_manager_routine(void *sem)
{
    struct write_request write_request;

    sem_init(sem, 0, 0);
    declare_as_initialized();
    if (should_terminate()) {
        goto cleanup;
    }

    while (1) {
        sem_wait(sem);
        if (should_terminate()) {
            goto cleanup;
        } else if (pthread_queue_is_non_empty(&write_requests)) {
            pthread_queue_pop(&write_requests, &write_request, NULL);
            printf("Received a write request with id %d\n", write_request.id);
            // arbitrarily simulate a long operation
            if (write_request.id == 2) {
                sleep(3);
            }
        }
    }

cleanup:
    return NULL;
}

void *
cache_manager_routine(void *sem)
{
    sem_init(sem, 0, 0);
    declare_as_initialized();
    return NULL;
}

void *
sender_routine(void *sem)
{
    sem_init(sem, 0, 0);
    declare_as_initialized();
    return NULL;
}

void *
receiver_routine(void *sem)
{
    sem_init(sem, 0, 0);
    declare_as_initialized();
    return NULL;
}
