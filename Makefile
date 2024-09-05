# grid - spreadsheet editor
# See LICENSE file for copyright and license details.

include config.mk

SRC = client.c
LIB = \
	cache_manager.c \
	controller.c \
	display.c \
	pthread_queue.c \
	thread_management.c \
	thread_routines.c \
	types.c
OBJ = ${SRC:.c=.o}
LIBOBJ = ${LIB:.c=.o} clic.o termbox2.o
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
	${CC} -c ${CFLAGS} -o $@ -x c $< -DCLIC_IMPL

termbox2.o: %.o: %.h
	${CC} -c ${CFLAGS} -o $@ -x c $< -DTB_IMPL

${EXE}: %: %.o ${LIBOBJ}
	${CC} ${LDFLAGS} -o $@ $< ${LIBOBJ}

clean:
	rm -f ${EXE} ${OBJ} ${LIBOBJ}

.PHONY: all options clean dist install uninstall
