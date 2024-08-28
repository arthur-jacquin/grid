#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

#include "types.h"

void get_area(struct area area, int *hits, struct cell_display *dest,
    struct address focus, int *focus_hit, struct cell_content *focus_dest);
void get_cell(struct address focus, int *hit, struct cell_content *dest);

#endif // CACHE_MANAGER_H
