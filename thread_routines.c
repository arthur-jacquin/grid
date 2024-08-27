#include <semaphore.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include "thread_management.h"

// TEMPLATE FOR THREADS START ROUTINE:
void *
controller_routine(void *sem)
{
    // INITIALIZATION
    // at least initialize sem, and declare as initialized at the end
    sem_init(sem, 0, 0);
    declare_as_initialized();
    if (should_terminate()) {
        goto cleanup;
    }

    // MAIN LOOP
    while (1) {
        // wait for tasks
        sem_wait(sem);
        // check all possible tasks, always starting by checking if the thread
        // should terminate, then process the event
        if (should_terminate()) {
            goto cleanup;
        // } else if (...) {
        }
    }

cleanup:
    // DEINITIALIZATION
    // whatever cleanup is required

    return NULL;
}

// TEST ROUTINE:
void *
state_manager_routine(void *sem)
{
    sem_init(sem, 0, 0);
    sleep(2);
    declare_as_initialized();
    printf("Everything is initialized\n");
    sleep(2);
    request_termination(0);
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
