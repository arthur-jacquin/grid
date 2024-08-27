#include <pthread.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "pthread_queue.h"
#include "thread_management.h"

void
pthread_queue_destroy(struct pthread_queue *queue)
{
    pthread_mutex_lock(&queue->mutex);
    for (struct pthread_queue_elem *elem = queue->first_in, *next;
        elem && (next = elem->next, 1); elem = next) {
        if (queue->data_size == 0) {
            free((void *) elem->payload);
        }
        free(elem);
    }
    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);
}

void
pthread_queue_init(struct pthread_queue *queue, enum thread_id consumer_thread,
    size_t data_size)
{
    *queue = (struct pthread_queue) {
        .consumer_thread = consumer_thread,
        .data_size = data_size,
    };
    pthread_mutex_init(&queue->mutex, NULL);
}

int
pthread_queue_is_non_empty(struct pthread_queue *queue)
{
    pthread_mutex_lock(&queue->mutex);
    int res = queue->first_in ? 1 : 0;
    pthread_mutex_unlock(&queue->mutex);
    return res;
}

int
pthread_queue_pop(struct pthread_queue *queue, void *dest, const void **payload)
{
    int res;
    struct pthread_queue_elem *next;

    pthread_mutex_lock(&queue->mutex);
    if (queue->first_in) {
        res = 0;
        if (queue->data_size == 0) {
            *payload = queue->first_in->payload;
        } else {
            memcpy(dest, queue->first_in + sizeof(*queue->first_in),
                queue->data_size);
        }
        next = queue->first_in->next;
        if (!next) {
            queue->last_in = NULL;
        }
        free(queue->first_in);
        queue->first_in = next;
    } else {
        res = -1;
    }
    pthread_mutex_unlock(&queue->mutex);
    return res;
}

void
pthread_queue_push(struct pthread_queue *queue, const void *src)
{
    struct pthread_queue_elem *new = malloc(sizeof(*new) + queue->data_size);
    new->next = NULL;
    if (queue->data_size == 0) {
        new->payload = src;
    } else {
        memcpy(new + sizeof(*new), src, queue->data_size);
    }
    pthread_mutex_lock(&queue->mutex);
    if (queue->last_in) {
        queue->last_in->next = new;
    } else {
        queue->first_in = new;
    }
    queue->last_in = new;
    pthread_mutex_unlock(&queue->mutex);
    post_to(queue->consumer_thread);
}
