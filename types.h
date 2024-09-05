#ifndef TYPES_H
#define TYPES_H

#include "config.h"
#include "termbox2.h"

#define MAX(A, B)   ((A) > (B) ? (A) : (B))
#define MIN(A, B)   ((A) < (B) ? (A) : (B))

// TODO: reorder
typedef int sheet_id;
struct address {
    sheet_id sheet_id;
    int row, col;
};
struct area {
    sheet_id sheet_id;
    int row, col;
    int row_span, col_span;
};
struct cell_content {
    struct address address;
    int state; // XXX: for testing purposes
};
struct cell_display {
    char ch[CELL_WIDTH + 1];
    uintattr_t fg;
};
struct cursor_pos {
    sheet_id sheet_id;
    int row, col;
    int anchor_row, anchor_col;
};
struct view {
    sheet_id sheet_id;
    // the xforce first columns and xlen columns starting at xmin are included
    // the same goes for y* and rows
    int xforce, xlen, xmin, yforce, ylen, ymin;
    // cells and hits are expected to be either NULL or of length
    // (xforce + xlen)*(yforce + ylen), and must be used with a row-major order
    struct cell_display *cells;
    int *hits;
};
struct view_request {
    struct view view; // hits and buf should be ignored
    int *hits, nb_hits;
};

// TODO
struct validation {
    int id; // XXX: for testing purpose
};
struct write_request {
    int id; // XXX: for testing purpose
};

int address_equal(struct address a, struct address b);
int address_in_area(struct address address, struct area area);
int address_in_view(struct address address, struct view view);
struct address address_of_cursor(struct cursor_pos cursor);
int col_name(int x, char buf[]);
struct address get_view_address(struct view view, int index);
int get_view_index(struct view view, struct address address);
int get_view_length(struct view view);
int row_name(int y, char buf[]);
int view_equal(struct view a, struct view b);

#endif // TYPES_H
