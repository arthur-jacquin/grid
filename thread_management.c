#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>

#include "thread_management.h"

void *controller_routine(void *sem);
void *state_manager_routine(void *sem);
void *cache_manager_routine(void *sem);
void *sender_routine(void *sem);
void *receiver_routine(void *sem);

static int all_threads_initialized(void);
static enum thread_id get_thread_id(void);
static void spawn_thread(enum thread_id thread_id,
    void *(*start_routine) (void *), const pthread_attr_t *attr);
static void *signal_handling_routine(void *arg);

static int termination_exit_status, termination_requested;
static int thread_init_states[THREAD_NB];
static pthread_cond_t init_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;
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
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&signal_handling_pthread_id, &attr, signal_handling_routine,
        &sigmask);
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
}

void
declare_as_initialized(void)
{
    // declare the calling thread as initialized and ready to operate
    // block until all threads are initialized or one failed
    pthread_mutex_lock(&init_mutex);
    thread_init_states[get_thread_id()] = 1;
    pthread_cond_broadcast(&init_cond);
    while (!(all_threads_initialized() || should_terminate())) {
        pthread_cond_wait(&init_cond, &init_mutex);
    }
    pthread_mutex_unlock(&init_mutex);
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
    pthread_mutex_lock(&init_mutex);
    if (all_threads_initialized()) {
        for (int i = 0; i < THREAD_NB; i++) {
            post_to(i);
        }
    } else {
        pthread_cond_broadcast(&init_cond);
    }
    pthread_mutex_unlock(&init_mutex);
}

int
should_terminate(void)
{
    pthread_mutex_lock(&termination_mutex);
    int res = termination_requested;
    pthread_mutex_unlock(&termination_mutex);
    return res;
}

static int
all_threads_initialized(void)
{
    // init_mutex should be locked by the caller
    int res = 1;
    for (int i = 0; i < THREAD_NB; i++) {
        res &= thread_init_states[i];
    }
    return res;
}

static enum thread_id
get_thread_id(void)
{
    pthread_t self = pthread_self();
    for (int i = 0; i < THREAD_NB; i++) {
        if (pthread_equal(self, pthread_ids[i])) {
            return i;
        }
    }
    return -1; // unreachable
}

static void
spawn_thread(enum thread_id thread_id, void *(*start_routine) (void *),
    const pthread_attr_t *attr)
{
    pthread_create(&pthread_ids[thread_id], attr, start_routine,
        &thread_sems[thread_id]);
}

static void *
signal_handling_routine(void *sigmask)
{
    int sig, nb_sigint = 0;
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
            if (nb_sigint == 0) {
                request_termination(EXIT_FAILURE);
            } else {
                exit(EXIT_FAILURE);
            }
            nb_sigint++;
            break;
        }
    }
    return NULL;
}
