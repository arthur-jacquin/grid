#include "termbox2.h"

// features
#define TB_MOUSE_SUPPORT            0
#ifndef TB_OUTPUT_MODE
#define TB_OUTPUT_MODE              TB_OUTPUT_NORMAL
#endif // TB_OUTPUT_MODE

// performance
#define CACHE_SIZE                  (1 << 10)

// spacing
#define CELL_WIDTH                  8
#define ROWS_NB_WIDTH               4
#define XPAD                        1
#define YPAD                        3

// colors
#if TB_OUTPUT_MODE == TB_OUTPUT_NORMAL
// TODO

#elif TB_OUTPUT_MODE == TB_OUTPUT_256
// TODO

#elif TB_OUTPUT_MODE == TB_OUTPUT_TRUECOLOR
#define TB_COLOR_BG_CURSOR          0x285477
#define TB_COLOR_BG_DEFAULT         TB_DEFAULT
#define TB_COLOR_BG_HEADERS         0x303030
#define TB_COLOR_FG_DEFAULT         TB_DEFAULT
#define TB_COLOR_FG_HEADERS         TB_HI_BLACK
#endif // TB_OUTPUT_MODE
