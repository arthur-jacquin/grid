#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

#include "types.h"

void get_view(struct view view, struct address address, int *hit,
    struct cell_content *dest);
void get_cell(struct address address, int *hit, struct cell_content *dest);

#endif // CACHE_MANAGER_H
