#ifndef TYPES_H
#define TYPES_H

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

// TODO
struct area_request {};
struct cell_display {};
struct cursor_pos {};
struct validation {};
struct write_request {
    int id; // XXX: for testing purpose
};

int address_equal(struct address a, struct address b);
int address_in_area(struct address address, struct area area);

#endif // TYPES_H
