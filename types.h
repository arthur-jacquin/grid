#ifndef TYPES_H
#define TYPES_H

#define MAX(A, B)   ((A) > (B) ? (A) : (B))
#define MIN(A, B)   ((A) < (B) ? (A) : (B))

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
};
struct cursor_pos {
    sheet_id sheet_id;
    int row, col;
    int anchor_row, anchor_col;
};

// TODO
struct area_request {};
struct cell_display {};
struct validation {};
struct write_request {
    int id; // XXX: for testing purpose
};

int address_equal(struct address a, struct address b);
int address_in_area(struct address address, struct area area);
int col_name(int x, char buf[]);
int row_name(int y, char buf[]);

#endif // TYPES_H
