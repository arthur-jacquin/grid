// TODO: explore by most recent first or linearly ?

#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#include "cache_manager.h"
#include "client.h"
#include "config.h"
#include "controller.h"
#include "pthread_queue.h"
#include "thread_management.h"
#include "types.h"

typedef int cache_id;
struct cache_metadata {
    int valid;
    cache_id prev, next;
    // doubly linked list, from the least to the most recent element
    // ends of the list are detected with *_recently_used, and have undefined
    // prev/next field
    struct address address;
};

static cache_id find_address(struct address address);
static void link(cache_id a, cache_id b);
static void process_cell_update(struct cell_content *cell_update);
static int should_send_to_controller(struct address address);
static int should_store(struct address address);
static cache_id store(struct cell_content *cell, cache_id index);
static void use(cache_id index);

static cache_id least_recently_used, most_recently_used;
static int is_full, nb_cached_cell;
static pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct area last_requested_area = {.sheet_id = -1};
static struct cache_metadata metadata[CACHE_SIZE];
static struct cell_content content_cache[CACHE_SIZE];
static struct cell_display display_cache[CACHE_SIZE];

void
get_area(struct area area, int *hits, struct cell_display *dest,
    struct address focus, int *focus_hit, struct cell_content *focus_dest)
{
    // hits and dest are expected to be of length area.row_span*area.col_span
    // results are stored in a row-major order

    int index_in_buffer, nb_hits, nb_cells;
    struct address address;

    pthread_mutex_lock(&cache_mutex);

    // init
    last_requested_area = area;
    nb_hits = 0;
    nb_cells = area.row_span * area.col_span;
    memset(hits, 0, nb_cells * sizeof(int));
    *focus_hit = 0;

    // explore the cache to fill the dest buffer with found cells
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!metadata[i].valid) {
            continue;
        }
        if (address_in_area(address = metadata[i].address, area)) {
            nb_hits++;
            index_in_buffer = (address.row - area.row)*area.col_span +
                (address.col - area.col);
            hits[index_in_buffer] = 1;
            dest[index_in_buffer] = display_cache[i];
            use(i);
        }
        if (address_equal(focus, address)) {
            *focus_hit = 1;
            *focus_dest = content_cache[i];
        }
    }

    // issue requests to state manager for unfound cells
    // TODO: more granular area requests ? or detect movements ?
    if (nb_hits < nb_cells) {
        pthread_queue_push(&area_requests, &area);
    }

    pthread_mutex_unlock(&cache_mutex);
}

void
get_cell(struct address focus, int *hit, struct cell_content *dest)
{
    int index;

    pthread_mutex_lock(&cache_mutex);
    if ((index = find_address(focus)) < 0) {
        *hit = 0;
    } else {
        *hit = 1;
        *dest = content_cache[index];
    }
    pthread_mutex_unlock(&cache_mutex);
}

static cache_id
find_address(struct address address)
{
    // return index if address is found, else a negative number
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (metadata[i].valid && address_equal(address, metadata[i].address)) {
            return i;
        }
    }
    return -1;
}

static void
link(cache_id a, cache_id b)
{
    metadata[a].next = b;
    metadata[b].prev = a;
}

static void
process_cell_update(struct cell_content *cell_update)
{
    cache_id index;
    struct address address;

    address = cell_update->address;
    if ((index = find_address(address)) >= 0 || should_store(address)) {
        store(cell_update, index);
    }
    if (should_send_to_controller(address)) {
        draw_cell(cell_update);
    }
}

static int
should_send_to_controller(struct address address)
{
    return address_in_area(address, last_requested_area);
}

// TODO: better predictions by storing a history of requested area ?
static int
should_store(struct address address)
{
    return address_in_area(address, last_requested_area);
}

static cache_id
store(struct cell_content *cell, cache_id index)
{
    // if index >= 0, refresh the existing value, else insert cell in the cache
    // and set as most recently used element
    // return the index used
    if (index < 0) {
        if (is_full) {
            // replace the least recently used element
            index = least_recently_used;
            use(index);
        } else {
            // initialize and insert at the end of the list
            index = nb_cached_cell++;
            is_full = nb_cached_cell == CACHE_SIZE;
            metadata[index].valid = 1;
            link(most_recently_used, index);
            most_recently_used = index;
        }
        metadata[index].address = cell->address;
    }
    content_cache[index] = *cell;
    display_cache[index] = display_cell(cell);
    return index;
}

static void
use(cache_id index)
{
    // keep the metadata up to date with the use of index
    // index must already be in the doubly linked list
    if (index == most_recently_used) {
        return;
    }
    if (index == least_recently_used) {
        least_recently_used = metadata[index].next;
    } else {
        link(metadata[index].prev, metadata[index].next);
    }
    link(most_recently_used, index);
    most_recently_used = index;
}

void *
cache_manager_routine(void *sem)
{
    struct cell_content cell_update;

    while (1) {
        sem_wait(sem);
        if (should_terminate()) {
            goto cleanup;
        } else if (pthread_queue_is_non_empty(&cell_updates)) {
            pthread_queue_pop(&cell_updates, &cell_update, NULL);
            pthread_mutex_lock(&cache_mutex);
            process_cell_update(&cell_update);
            pthread_mutex_unlock(&cache_mutex);
        }
    }

cleanup:
    return NULL;
}
