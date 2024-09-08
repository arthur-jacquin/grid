// TODO: explore by most recent first or linearly ?

#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "cache_manager.h"
#include "client.h"
#include "config.h"
#include "display.h"
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
static void dllist_link(cache_id a, cache_id b);
static void process_cell_update(struct cell_content *cell_update);
static int should_send_to_controller(struct address address);
static int should_store(struct address address);
static cache_id store(struct cell_content *cell, cache_id index);
static void use(cache_id index);

static cache_id least_recently_used, most_recently_used;
static int is_full, nb_cached_cell;
static pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct cache_metadata metadata[CACHE_SIZE];
static struct cell_content content_cache[CACHE_SIZE];
static struct cell_display display_cache[CACHE_SIZE];
static struct view last_requested_view = {.sheet_id = -1};

void
get_view(struct view view, struct cell_display *cells, int *hits,
    struct address address, struct cell_content *cell, int *hit)
{
    // cells and and hits buffers must be of length get_view_length(view)
    // try to retrieve the cached display of view cells in cells, and the
    // content located at address in cell

    int index, nb_cells;
    struct view_request view_request;

    pthread_mutex_lock(&cache_mutex);

    // init
    last_requested_view = view;
    nb_cells = get_view_length(view);
    view_request = (struct view_request) {
        .view = view,
        .hits = calloc(nb_cells, sizeof(int)),
        .nb_hits = 0,
    };

    // explore the cache to fill the view.cells buffer with found cells
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!metadata[i].valid) {
            continue;
        }
        if ((index = get_view_index(view, metadata[i].address)) >= 0) {
            cells[index] = display_cache[i];
            hits[index] = view_request.hits[index] = 1;
            view_request.nb_hits++;
            use(i);
        }
        if (address_equal(address, metadata[i].address)) {
            *hit = 1;
            *cell = content_cache[i];
        }
    }

    // issue request to state manager for unfound cells
    pthread_queue_push(&view_requests, &view_request);

    pthread_mutex_unlock(&cache_mutex);
}

void
get_cell(struct address address, int *hit, struct cell_content *dest)
{
    int index;

    pthread_mutex_lock(&cache_mutex);
    if ((index = find_address(address)) < 0) {
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
dllist_link(cache_id a, cache_id b)
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
    return address_in_view(address, last_requested_view);
}

// TODO: better predictions by storing a history of requested views ?
static int
should_store(struct address address)
{
    return address_in_view(address, last_requested_view);
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
            dllist_link(most_recently_used, index);
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
        dllist_link(metadata[index].prev, metadata[index].next);
    }
    dllist_link(most_recently_used, index);
    most_recently_used = index;
}

void *
cache_manager_routine(void *sem)
{
    while (1) {
        sem_wait(sem);
        if (should_terminate()) {
            goto cleanup;
        } else if (pthread_queue_is_non_empty(&cell_updates)) {
            struct cell_content cell_update;
            pthread_queue_pop(&cell_updates, &cell_update);
            pthread_mutex_lock(&cache_mutex);
            process_cell_update(&cell_update);
            pthread_mutex_unlock(&cache_mutex);
        }
    }

cleanup:
    return NULL;
}
