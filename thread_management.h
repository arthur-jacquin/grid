#ifndef THREAD_MANAGEMENT_H
#define THREAD_MANAGEMENT_H

enum thread_id {
    CONTROLLER,
    STATE_MANAGER,
    CACHE_MANAGER,
    SENDER,
    RECEIVER,
};
#define THREAD_NB   5

void capture_signals(void);
int join_threads(void);
void spawn_threads(void);

void post_to(enum thread_id thread);
void request_termination(int exit_status);
int should_terminate(void);

#endif // THREAD_MANAGEMENT_H
