# grid version
VERSION = 0.0.0

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# terminal color support
# a default colorscheme is provided for TB_OUTPUT_{NORMAL,256,TRUECOLOR} modes
TBFLAGS  = -DTB_OUTPUT_MODE=TB_OUTPUT_NORMAL
#TBFLAGS  = -DTB_OUTPUT_MODE=TB_OUTPUT_256
#TBFLAGS  = -DTB_OPT_ATTR_W=32 -DTB_OUTPUT_MODE=TB_OUTPUT_TRUECOLOR

# flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -DVERSION=\"${VERSION}\" ${TBFLAGS}
#CFLAGS   = -g -std=c99 -pedantic -Wall -O0 ${CPPFLAGS}
CFLAGS   = -std=c99 -pedantic -Wall -Wno-deprecated-declarations -Os ${CPPFLAGS}

# compiler and linker
CC = cc
