# grid version
VERSION = 0.0.0

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# terminal color support
# the default configuration file uses TB_OUTPUT_NORMAL mode by default, and
# provides colorschemes for TB_OUTPUT_{NORMAL,256,TRUECOLOR} modes
# -DTB_OPT_ATTR_W=32 is needed for TB_OUTPUT_TRUECOLOR mode support
TBFLAGS  = -DTB_OUTPUT_MODE=TB_OUTPUT_NORMAL
#TBFLAGS  = -DTB_OUTPUT_MODE=TB_OUTPUT_256
#TBFLAGS  = -DTB_OPT_ATTR_W=32 -DTB_OUTPUT_MODE=TB_OUTPUT_TRUECOLOR

# flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -DVERSION=\"${VERSION}\" ${TBFLAGS}
#CFLAGS   = -g -std=c99 -pedantic -Wall -O0 ${CPPFLAGS}
CFLAGS   = -std=c99 -pedantic -Wall -Wno-deprecated-declarations -Os ${CPPFLAGS}

# compiler and linker
CC = cc
