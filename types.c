#include "types.h"

int
address_equal(struct address a, struct address b)
{
    return a.sheet_id == b.sheet_id && a.row == b.row && a.col == b.col;
}

int
address_in_area(struct address address, struct area area)
{
    int same_sheet, row_in_range, col_in_range, d;

    same_sheet = address.sheet_id == area.sheet_id;
    row_in_range = (d = address.row - area.row) >= 0 && d < area.row_span;
    col_in_range = (d = address.col - area.col) >= 0 && d < area.col_span;
    return same_sheet && row_in_range && col_in_range;
}
