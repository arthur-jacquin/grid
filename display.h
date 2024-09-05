#ifndef DISPLAY_H
#define DISPLAY_H

#include "types.h"

struct cell_display display_cell(const struct cell_content *cell);
void draw_cell(const struct cell_content *cell);

int init_termbox(void);
void move_to_cursor(void);
int set_force(int x, int y);
int set_term_size(int width, int height);

void print_cell_data(void);
void print_command_status_line(void);
void print_grid(void);
void print_invalid_term_size(void);
void refresh_terminal(void);

#endif // DISPLAY_H
