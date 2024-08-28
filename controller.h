#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "types.h"

struct cell_display display_cell(const struct cell_content *cell);
void draw_cell(const struct cell_content *cell);

#endif // CONTROLLER_H
