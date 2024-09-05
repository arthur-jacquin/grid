#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <unistd.h>

#include "config.h"
#include "client.h"
#include "display.h"
#include "pthread_queue.h"
#include "termbox2.h"
#include "thread_management.h"
#include "types.h"

int cursor_content_found;
struct cell_content cursor_content;
struct cursor_pos cursor;

static void process_event(struct tb_event ev);

static int wait_for_resize;

static void
process_event(struct tb_event ev)
{
    // TODO
    struct write_request write_request;

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
        case TB_KEY_ARROW_UP:
        case TB_KEY_ARROW_DOWN:
            cursor.row += ev.key == TB_KEY_ARROW_UP ? -1 : 1;
            move_to_cursor();
            break;
        case TB_KEY_ARROW_LEFT:
        case TB_KEY_ARROW_RIGHT:
            cursor.col += ev.key == TB_KEY_ARROW_LEFT ? -1 : 1;
            move_to_cursor();
            break;
        }
        break;
    case TB_EVENT_RESIZE:
        wait_for_resize = set_term_size(ev.w, ev.h);
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

    wait_for_resize = init_termbox();
    refresh_terminal();

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
            if (ev.type == TB_EVENT_KEY && ev.key == TB_KEY_CTRL_C) {
                kill(getpid(), SIGINT);
                break;
            } else if (wait_for_resize && ev.type != TB_EVENT_RESIZE) {
                break;
            }
            process_event(ev);
            refresh_terminal();
            break;
        }
    }

cleanup:
    tb_shutdown();
    return NULL;
}
