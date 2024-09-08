#include <pthread.h>
#include <stdlib.h>

#include "cache_manager.h"
#include "config.h"
#include "display.h"
#include "termbox2.h"
#include "types.h"

#define CELL_DATA_HEIGHT                    3
#define CELL_DATA_MINIMUM_WIDTH             8
#define COMMAND_MINIMUM_WIDTH               3
#define STATUS_WIDTH                        0 // TODO: set it with ifdef status options

#define NB_COLUMNS                          (26 * 27)
#if ROWS_NB_WIDTH < 2
#undef ROWS_NB_WIDTH
#define ROWS_NB_WIDTH                       0
#elif ROWS_NB_WIDTH == 2
#define ROWS_NB_MODULUS                     10
#elif ROWS_NB_WIDTH == 3
#define ROWS_NB_MODULUS                     100
#else
#define ROWS_NB_MODULUS                     1000
#endif // ROWS_NB_WIDTH

extern int cursor_content_found;
extern struct cell_content cursor_content;
extern struct cursor_pos cursor;

static int enforce_view_changes(void);
static uintattr_t get_cell_bg(int x, int y);
static void transfer_view_knowledge(struct view *old, struct view *new);

static char cell_buf[CELL_WIDTH + 1];
static int tb_initialized;
static int term_height, term_width, xpad, ypad;
static pthread_mutex_t tb_mutex = PTHREAD_MUTEX_INITIALIZER;
static const struct cell_display missing_cell = {
    .ch = "  miss  ",
    .fg = TB_COLOR_FG_MISS,
};
static struct view view;

// hits are cells buffers should be of length get_view_length(view)
static int *hits = NULL;
static struct cell_display *cells = NULL;

struct cell_display
display_cell(const struct cell_content *cell)
{
    // TODO
    struct cell_display res;

    memset(res.ch, ' ', CELL_WIDTH);
    res.ch[CELL_WIDTH] = '\0';
    res.ch[0] = 'r';
    if (cell->state) {
        res.ch[0] = 'S';
    }
    res.fg = TB_COLOR_FG_DEFAULT;
    return res;
}

void
draw_cell(const struct cell_content *cell)
{
    int is_ready;

    // return early if interface isn't initialized
    pthread_mutex_lock(&tb_mutex);
    is_ready = tb_initialized;
    pthread_mutex_unlock(&tb_mutex);
    if (!is_ready) {
        return;
    }

    // TODO
}

void
deinit_termbox(void)
{
    tb_shutdown();
    free(cells); cells = NULL;
    free(hits); hits = NULL;
}

int
init_termbox(void)
{
    // return a non-null result if terminal is too small
    int invalid_term_size;

    tb_init();
    tb_set_clear_attrs(TB_COLOR_FG_DEFAULT, TB_COLOR_BG_DEFAULT);
#ifdef TB_MOUSE_SUPPORT
    tb_set_input_mode(tb_set_input_mode(TB_INPUT_CURRENT) | TB_INPUT_MOUSE);
#endif // TB_MOUSE_SUPPORT
    tb_set_output_mode(TB_OUTPUT_MODE);
    invalid_term_size = set_term_size(tb_width(), tb_height());

    pthread_mutex_lock(&tb_mutex);
    tb_initialized = 1;
    pthread_mutex_unlock(&tb_mutex);

    return invalid_term_size;
}

void
move_to_cursor(void)
{
    static struct view old_view = {.sheet_id = -1};

    // TODO: manage sheet changes (view.sheet, view.*force)

    // ensure address is valid
    cursor.col = MAX(-1, MIN(cursor.col, NB_COLUMNS - 1));
    cursor.row = MAX(-1, cursor.row);

    // move view
    if (cursor.col < view.xforce + xpad) {
        view.xmin = view.xforce;
    } else if (cursor.col >= NB_COLUMNS - xpad) {
        view.xmin = NB_COLUMNS - view.xlen;
    } else {
        view.xmin = MIN(view.xmin, cursor.col - xpad);
        view.xmin = MAX(view.xmin, cursor.col + xpad + 1 - view.xlen);
    }
    if (cursor.row < view.yforce + ypad) {
        view.ymin = view.yforce;
    } else {
        view.ymin = MIN(view.ymin, cursor.row - ypad);
        view.ymin = MAX(view.ymin, cursor.row + ypad + 1 - view.ylen);
    }

    // if change in view, realloc and init buffers, query cache manager
    if (!view_equal(old_view, view)) {
        transfer_view_knowledge(&old_view, &view);
        get_view(view, cells, hits, address_of_cursor(cursor),
            &cursor_content, &cursor_content_found);
    }

    // remember new view
    old_view = view;

    // redraw grid
    print_grid();
}

int
set_force(int x, int y)
{
    // return a non-null result if terminal is too small
    int invalid_term_size;

    // detect invalid terminal size
    view.xforce = x;
    view.yforce = y;
    invalid_term_size = enforce_view_changes();

    // redraw grid
    print_grid();

    return invalid_term_size;
}

int
set_term_size(int width, int height)
{
    // return a non-null result if terminal is too small
    int invalid_term_size;

    // detect invalid terminal size
    term_height = height;
    term_width = width;
    invalid_term_size = enforce_view_changes();

    // redraw everything
    tb_clear();
    if (invalid_term_size) {
        print_invalid_term_size();
    } else {
        print_cell_data();
        print_grid();
        print_command_status_line();
    }

    return invalid_term_size;
}

void
print_cell_data(void)
{
    // TODO
}

void
print_command_status_line(void)
{
    // TODO
}

void
print_grid(void)
{
    int index, x, y;
    const struct cell_display *cell;

    pthread_mutex_lock(&tb_mutex);

    // corner
#if ROWS_NB_WIDTH
    memset(cell_buf, ' ', ROWS_NB_WIDTH);
    cell_buf[ROWS_NB_WIDTH] = '\0';
    tb_print(0, CELL_DATA_HEIGHT, 0, cursor.col < 0 || cursor.row < 0 ?
        TB_COLOR_BG_CURSOR : TB_COLOR_BG_HEADERS, cell_buf);
#endif // ROWS_NB_WIDTH

    // columns header
    memset(cell_buf, ' ', CELL_WIDTH);
    for (int i = 0; i < view.xforce + view.xlen; i++) {
        x = i < view.xforce ? i : view.xmin + i - view.xforce;
        col_name(x, cell_buf + CELL_WIDTH/2 - 1);
        tb_print(ROWS_NB_WIDTH + i*CELL_WIDTH, CELL_DATA_HEIGHT,
            TB_COLOR_FG_HEADERS,
            x == cursor.col ? TB_COLOR_BG_CURSOR : TB_COLOR_BG_HEADERS,
            cell_buf);
    }

    // row numbers and cells
    index = 0;
    for (int i = 0; i < view.yforce + view.ylen; i++) {
        y = i < view.yforce ? i : view.ymin + i - view.yforce;
#if ROWS_NB_WIDTH
        tb_printf(0, CELL_DATA_HEIGHT + 1 + i, TB_COLOR_FG_HEADERS,
            y == cursor.row ? TB_COLOR_BG_CURSOR : TB_COLOR_BG_HEADERS,
            "%*d ", ROWS_NB_WIDTH - 1, (y + 1)%ROWS_NB_MODULUS);
#endif // ROWS_NB_WIDTH
        for (int j = 0; j < view.xforce + view.xlen; j++) {
            x = j < view.xforce ? j : view.xmin + j - view.xforce;
            cell = hits[index] ? &cells[index] : &missing_cell;
            tb_print(ROWS_NB_WIDTH + j*CELL_WIDTH, CELL_DATA_HEIGHT + 1 + i,
                cell->fg, get_cell_bg(x, y), cell->ch);
            index++;
        }
    }

    pthread_mutex_unlock(&tb_mutex);
}

void
print_invalid_term_size(void)
{
    // TODO
}

void
refresh_terminal(void)
{
    pthread_mutex_lock(&tb_mutex);
    tb_present();
    pthread_mutex_unlock(&tb_mutex);
}

static int
enforce_view_changes(void)
{
    int invalid_term_size;

    view.xlen = (term_width - ROWS_NB_WIDTH)/CELL_WIDTH - view.xforce;
    view.ylen = term_height - (CELL_DATA_HEIGHT + 2 + view.yforce);
    invalid_term_size = view.xlen < 1 || view.ylen < 1 ||
        term_width < CELL_DATA_MINIMUM_WIDTH ||
        term_width < COMMAND_MINIMUM_WIDTH + STATUS_WIDTH;
    xpad = MIN(XPAD, (view.xlen - 1)/2);
    ypad = MIN(YPAD, (view.ylen - 1)/2);
    move_to_cursor();

    return invalid_term_size;
}

static uintattr_t
get_cell_bg(int x, int y)
{
    return x == cursor.col && y == cursor.row ? TB_COLOR_BG_CURSOR :
        TB_COLOR_BG_DEFAULT;
}

static void
transfer_view_knowledge(struct view *old, struct view *new)
{
    // use old buffers to create new ones
    int view_length;
    struct cell_display *new_cells;
    int *new_hits;

    // allocate new buffers
    view_length = get_view_length(*new);
    new_cells = malloc(view_length*sizeof(*new_cells));
    new_hits = calloc(view_length, sizeof(*new_hits));

    // copy known cells
    if (old->sheet_id != new->sheet_id) {
        goto end_copy;
    }
    // TODO
end_copy:

    // destroy old buffers
    free(cells); cells = new_cells;
    free(hits); hits = new_hits;
}
