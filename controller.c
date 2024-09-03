#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <unistd.h>

#include "config.h"
#include "client.h"
#include "controller.h"
#include "pthread_queue.h"
#include "termbox2.h"
#include "thread_management.h"
#include "types.h"

static int ready;
static pthread_mutex_t ready_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t tb_write_mutex = PTHREAD_MUTEX_INITIALIZER;

static void init_termbox(void);
static int is_ready(void);
static void process_event(struct tb_event ev);

struct cell_display
display_cell(const struct cell_content *cell)
{
    // TODO
}

void
draw_cell(const struct cell_content *cell)
{
    // TODO
    static int nb;

    if (!is_ready()) return;
    pthread_mutex_lock(&tb_write_mutex);
    tb_printf(0, 3, TB_DEFAULT, TB_DEFAULT, "draw_cell() call nb %d", ++nb);
    tb_present();
    pthread_mutex_unlock(&tb_write_mutex);
}

static void
init_termbox(void)
{
    int tb_input_mode;

    tb_init();
#if TB_MOUSE_SUPPORT
    tb_input_mode = tb_set_input_mode(TB_INPUT_CURRENT);
    tb_input_mode |= TB_INPUT_MOUSE;
    tb_set_input_mode(tb_input_mode);
#endif // TB_MOUSE_SUPPORT
    tb_set_output_mode(TB_OUTPUT_MODE);
}

static int
is_ready(void) {
    pthread_mutex_lock(&ready_mutex);
    int res = ready;
    pthread_mutex_unlock(&ready_mutex);
    return res;
}

static void
process_event(struct tb_event ev)
{
    // TODO
    static int nb_events;
    struct write_request write_request;

    tb_printf(0, 0, TB_DEFAULT, TB_DEFAULT, "Events: %d ('%c')", nb_events++,
        ev.ch);
    tb_present();
    switch (ev.type) {
    case TB_EVENT_KEY:
        // key (TB_KEY_*) XOR ch (Unicode codepoint), mod (TB_MOD_*)
        if (ev.ch) switch (ev.ch) {
        case 'q':
            request_termination(0);
            break;
        case 'w':
            pthread_queue_push(&write_requests, &write_request);
            break;
        } else switch (ev.key) {
        case TB_KEY_CTRL_C:
            kill(getpid(), SIGINT);
            break;
        }
        break;
    case TB_EVENT_RESIZE:
        // w, h
        break;
#if TB_MOUSE_SUPPORT
    case TB_EVENT_MOUSE:
        // key (TB_KEY_MOUSE_*), x, y
        switch (ev.key) {
        }
        break;
#endif // TB_MOUSE_SUPPORT
    }
}

void *
controller_routine(void *sem)
{
    int rv;
    struct tb_event ev;

    (void) sem; // ignored, but should_terminate() is still periodically checked

    init_termbox();
    pthread_mutex_lock(&ready_mutex);
    ready = 1;
    pthread_mutex_unlock(&ready_mutex);

    while (1) {
        rv = tb_peek_event(&ev, 100);
        if (should_terminate()) {
            goto cleanup;
        }
        // TODO: manage all possible errors
        switch (rv) {
        case TB_ERR_NO_EVENT:
            break;
        default:
            pthread_mutex_lock(&tb_write_mutex);
            process_event(ev);
            pthread_mutex_unlock(&tb_write_mutex);
        }
    }

cleanup:
    tb_shutdown();
    return NULL;
}
