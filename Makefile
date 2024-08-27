# grid - spreadsheet editor
# See LICENSE file for copyright and license details.

include config.mk

SRC =
LIB =
OBJ = ${SRC:.c=.o}
LIBOBJ = ${LIB:.c=.o}
EXE = ${SRC:.c=}

all: options ${EXE}

options:
	@echo grid build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

config.h:
	cp config.def.h $@

${OBJ} ${LIB:.c=.o}: %.o: %.c config.h
	${CC} -c ${CFLAGS} -o $@ $<

clic.o: %.o: %.h
	${CC} -c ${CFLAGS} -o $@ $< -DCLIC_IMPL

termbox2.o: %.o: %.h
	${CC} -c ${CFLAGS} -o $@ $< -DTB_IMPL

${EXE}: %: %.o ${LIBOBJ}
	${CC} ${LDFLAGS} -o $@ $< ${LIBOBJ}

clean:
	rm -f ${EXE} ${OBJ} ${LIBOBJ}

.PHONY: all options clean dist install uninstall
