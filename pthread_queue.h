#ifndef PTHREAD_QUEUE_H
#define PTHREAD_QUEUE_H

#include <pthread.h>
#include <stddef.h>

#include "thread_management.h"

#define PTHREAD_QUEUE_INITIALIZER(CONSUMER_THREAD, DATA_SIZE) \
    { \
        .consumer_thread = (CONSUMER_THREAD), \
        .mutex = PTHREAD_MUTEX_INITIALIZER, \
        .data_size = (DATA_SIZE), \
    }

struct pthread_queue_elem {
    struct pthread_queue_elem *next;
    const void *indirect_payload;
    char direct_payload[];
};
struct pthread_queue {
    enum thread_id consumer_thread;
    pthread_mutex_t mutex;
    size_t data_size;
    // if data_size == 0, uses a pointer to the payload (memory
    // management is left to the publisher and consumer threads)
    // else, directly stores the payload in the flexible array member
    struct pthread_queue_elem *first_in, *last_in;
};

void pthread_queue_destroy(struct pthread_queue *queue);
void pthread_queue_init(struct pthread_queue *queue,
    enum thread_id consumer_thread, size_t data_size);
int pthread_queue_is_non_empty(struct pthread_queue *queue);
int pthread_queue_pop(struct pthread_queue *queue, void *dest);
void pthread_queue_push(struct pthread_queue *queue, const void *src);

#endif // PTHREAD_QUEUE_H
