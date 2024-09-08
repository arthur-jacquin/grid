#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>

#include "thread_management.h"

void *controller_routine(void *sem);
void *state_manager_routine(void *sem);
void *cache_manager_routine(void *sem);
void *sender_routine(void *sem);
void *receiver_routine(void *sem);

static void spawn_thread(enum thread_id thread_id,
    void *(*start_routine) (void *), const pthread_attr_t *attr);
static void *signal_handling_routine(void *arg);

static int termination_exit_status, termination_requested;
static pthread_mutex_t termination_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t pthread_ids[THREAD_NB], signal_handling_pthread_id;
static sem_t thread_sems[THREAD_NB];
static sigset_t sigmask;

void
capture_signals(void)
{
    // should be called as soon as possible, before any other thread creation
    pthread_attr_t attr;
    sigset_t sigset;

    // block all signals, store the initial signal mask
    sigfillset(&sigset);
    pthread_sigmask(SIG_BLOCK, &sigset, &sigmask);

    // spawn a dedicated thread to detect and manage signals
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&signal_handling_pthread_id, &attr, signal_handling_routine,
        &sigmask);
    pthread_attr_destroy(&attr);
}

int
join_threads(void)
{
    // spawn_threads() must have been called before
    for (int i = 0; i < THREAD_NB; i++) {
        pthread_join(pthread_ids[i], NULL);
    }
    pthread_mutex_lock(&termination_mutex);
    int exit_status = termination_exit_status;
    pthread_mutex_unlock(&termination_mutex);
    return exit_status;
}

void
spawn_threads(void)
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    spawn_thread(CONTROLLER, controller_routine, &attr);
    spawn_thread(STATE_MANAGER, state_manager_routine, &attr);
    spawn_thread(CACHE_MANAGER, cache_manager_routine, &attr);
    spawn_thread(SENDER, sender_routine, &attr);
    spawn_thread(RECEIVER, receiver_routine, &attr);
    pthread_attr_destroy(&attr);
}

void
post_to(enum thread_id thread)
{
    sem_post(&thread_sems[thread]);
}

void
request_termination(int exit_status)
{
    int already_requested;

    // return early if termination is already requested
    pthread_mutex_lock(&termination_mutex);
    already_requested = termination_requested;
    pthread_mutex_unlock(&termination_mutex);
    if (already_requested) {
        return;
    }

    // store the termination request, prevent other threads
    pthread_mutex_lock(&termination_mutex);
    termination_requested = 1;
    termination_exit_status = exit_status;
    pthread_mutex_unlock(&termination_mutex);
    for (int i = 0; i < THREAD_NB; i++) {
        post_to(i);
    }
}

int
should_terminate(void)
{
    pthread_mutex_lock(&termination_mutex);
    int res = termination_requested;
    pthread_mutex_unlock(&termination_mutex);
    return res;
}

static void
spawn_thread(enum thread_id thread_id, void *(*start_routine) (void *),
    const pthread_attr_t *attr)
{
    sem_init(&thread_sems[thread_id], 0, 0);
    pthread_create(&pthread_ids[thread_id], attr, start_routine,
        &thread_sems[thread_id]);
}

static void *
signal_handling_routine(void *sigmask)
{
    int sig;
    sigset_t sigset;

    // add signals to the specified sigmask for custom handling
    sigset = * (sigset_t *) sigmask;
    sigaddset(&sigset, SIGINT);
    pthread_sigmask(SIG_SETMASK, &sigset, NULL);
    while (1) {
        sigwait(&sigset, &sig);
        switch (sig) {
        case SIGINT:
            // try a clean termination
            if (!should_terminate()) {
                request_termination(EXIT_FAILURE);
            } else {
                exit(EXIT_FAILURE);
            }
            break;
        }
    }
    return NULL;
}
