#include <pthread.h>

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

extern struct cursor_pos cursor;

static int compute_spacing_values(void);

static char cell_buf[CELL_WIDTH + 1]; // TODO: dynamic size
static int cell_width = CELL_WIDTH; // TODO: move to runtime settings
static int nb_visible_cols, nb_visible_rows;
static int term_height, term_width;
static int xforce, xmin, xpad, yforce, ymin, ypad;
static pthread_mutex_t tb_mutex = PTHREAD_MUTEX_INITIALIZER;

struct cell_display
display_cell(const struct cell_content *cell)
{
    // TODO
}

int
init_termbox(void)
{
    // return a non-null result if terminal is too small
    tb_init();
    tb_set_clear_attrs(TB_COLOR_FG_DEFAULT, TB_COLOR_BG_DEFAULT);
#ifdef TB_MOUSE_SUPPORT
    tb_set_input_mode(tb_set_input_mode(TB_INPUT_CURRENT) | TB_INPUT_MOUSE);
#endif // TB_MOUSE_SUPPORT
    tb_set_output_mode(TB_OUTPUT_MODE);
    return set_term_size(tb_width(), tb_height());
}

void
move_to_cursor(void)
{
    // TODO: manage sheet changes

    // ensure adress is valid
    cursor.col = MAX(-1, MIN(cursor.col, NB_COLUMNS - 1));
    cursor.row = MAX(-1, cursor.row);

    // move view
    if (cursor.col < xforce + xpad) {
        xmin = xforce;
    } else if (cursor.col >= NB_COLUMNS - xpad) {
        xmin = NB_COLUMNS - nb_visible_cols;
    } else {
        xmin = MIN(xmin, cursor.col - xpad);
        xmin = MAX(xmin, cursor.col + xpad + 1 - nb_visible_cols);
    }
    if (cursor.row < yforce + ypad) {
        ymin = yforce;
    } else {
        ymin = MIN(ymin, cursor.row - ypad);
        ymin = MAX(ymin, cursor.row + ypad + 1 - nb_visible_rows);
    }

    // TODO: if change in view (sheet, *min, *force), query cache manager

    // redraw grid
    print_grid();
}

int
set_force(int x, int y)
{
    // return a non-null result if terminal is too small
    int invalid_term_size;

    // detect invalid terminal size
    xforce = x;
    yforce = y;
    invalid_term_size = compute_spacing_values();

    // redraw grid
    print_grid();

    return invalid_term_size;
}

int
set_term_size(int width, int height)
{
    // return a non-null result if terminal is too small
    int invalid_term_size;

    // TODO: realloc and init view

    // detect invalid terminal size
    term_height = height;
    term_width = width;
    invalid_term_size = compute_spacing_values();

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
    int x, y;

    pthread_mutex_lock(&tb_mutex);

    // corner
#if ROWS_NB_WIDTH
    memset(cell_buf, ' ', ROWS_NB_WIDTH);
    cell_buf[ROWS_NB_WIDTH] = '\0';
    tb_print(0, CELL_DATA_HEIGHT, 0, cursor.col < 0 || cursor.row < 0 ?
        TB_COLOR_BG_CURSOR : TB_COLOR_BG_HEADERS, cell_buf);
#endif // ROWS_NB_WIDTH

    // columns header
    memset(cell_buf, ' ', cell_width);
    for (int i = 0; i < xforce + nb_visible_cols; i++) {
        x = i < xforce ? i : xmin + i - xforce;
        col_name(x, cell_buf + cell_width/2 - 1);
        tb_print(ROWS_NB_WIDTH + i*cell_width, CELL_DATA_HEIGHT,
            TB_COLOR_FG_HEADERS,
            x == cursor.col ? TB_COLOR_BG_CURSOR : TB_COLOR_BG_HEADERS,
            cell_buf);
    }

    // row numbers and cells
    for (int i = 0; i < yforce + nb_visible_rows; i++) {
        y = i < yforce ? i : ymin + i - yforce;
#if ROWS_NB_WIDTH
        tb_printf(0, CELL_DATA_HEIGHT + 1 + i, TB_COLOR_FG_HEADERS,
            y == cursor.row ? TB_COLOR_BG_CURSOR : TB_COLOR_BG_HEADERS,
            "%*d ", ROWS_NB_WIDTH - 1, (y + 1)%ROWS_NB_MODULUS);
#endif // ROWS_NB_WIDTH
        for (int j = 0; j < xforce + nb_visible_cols; j++) {
            x = j < xforce ? j : xmin + j - xforce;
            // TODO: print cell
            memset(cell_buf, ' ', cell_width);
            tb_print(ROWS_NB_WIDTH + j*cell_width, CELL_DATA_HEIGHT + 1 + i,
                TB_COLOR_FG_HEADERS,
                x == cursor.col && y == cursor.row ? TB_COLOR_BG_CURSOR :
                    TB_COLOR_BG_DEFAULT,
                cell_buf);
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
compute_spacing_values(void)
{
    int invalid_term_size;

    nb_visible_cols = (term_width - ROWS_NB_WIDTH) / cell_width - xforce;
    nb_visible_rows = term_height - (CELL_DATA_HEIGHT + 2 + yforce);
    invalid_term_size = nb_visible_cols < 1 || nb_visible_rows < 1 ||
        term_width < CELL_DATA_MINIMUM_WIDTH ||
        term_width < COMMAND_MINIMUM_WIDTH + STATUS_WIDTH;
    xpad = MIN(XPAD, (nb_visible_cols - 1)/2);
    ypad = MIN(YPAD, (nb_visible_rows - 1)/2);
    move_to_cursor();

    return invalid_term_size;
}
