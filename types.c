#include <stdio.h>

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

int
address_in_view(struct address address, struct view view)
{
    return get_view_index(view, address) >= 0;
}

struct address
address_of_cursor(struct cursor_pos cursor)
{
    return (struct address) {
        .sheet_id = cursor.sheet_id,
        .row = cursor.row,
        .col = cursor.col,
    };
}

int
col_name(int x, char buf[])
{
    if (x < 26) {
        buf[0] = 'A' + x;
        return 1;
    } else {
        buf[0] = 'A' + x/26 - 1;
        buf[1] = 'A' + x%26;
        return 2;
    }
}

struct address
get_view_address(struct view view, int index)
{
    int length, i, j;

    length = get_view_length(view);
    i = index/length;
    j = index%length;
    return (struct address) {
        .sheet_id = view.sheet_id,
        .row = i < view.yforce ? i : view.ymin + i - view.yforce,
        .col = j < view.xforce ? j : view.xmin + j - view.xforce,
    };
}

int
get_view_index(struct view view, struct address address)
{
    // return a negative result if address is not in view
    int same_sheet, row_in_view, col_in_view, d, i, j;

    same_sheet = address.sheet_id == view.sheet_id;
    row_in_view = address.row < view.yforce ||
        ((d = address.row - view.ymin) >= 0 && d < view.ylen);
    col_in_view = address.col < view.xforce ||
        ((d = address.col - view.xmin) >= 0 && d < view.xlen);
    if (same_sheet && row_in_view && col_in_view) {
        i = address.row < view.yforce ? address.row :
            view.yforce + address.row - view.ymin;
        j = address.col < view.xforce ? address.col :
            view.xforce + address.col - view.xmin;
        return i*get_view_length(view) + j;
    } else {
        return -1;
    }
}

int
get_view_length(struct view view)
{
    return (view.xforce + view.xlen)*(view.yforce + view.ylen);
}

int
row_name(int y, char buf[])
{
    return sprintf(buf, "%d", y);
}

int
view_equal(struct view a, struct view b)
{
    return a.sheet_id == b.sheet_id &&
        a.xforce == b.xforce && a.xlen == b.xlen && a.xmin == b.xmin &&
        a.yforce == b.yforce && a.ylen == b.ylen && a.ymin == b.ymin;
}
