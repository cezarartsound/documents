CC=gcc 
override CFLAGS+=-Wall -Wextra -Wno-unused-parameter -Wno-format-extra-args -I ./include/ -I ./src/
override LDFLAGS+=-pthread -lrt -lusb

DAEMON_BIN=./build/itdsrc_daemon
DAEMON_OBJS=./build/itdsrc_daemon.o
DAEMON_OBJS+=./build/hostlink_protocol.o
DAEMON_OBJS+=./build/itdsrc_protocol.o
DAEMON_INC=./src/itdsrc_daemon.h
DAEMON_INC+=./src/itdsrc_protocol.h
DAEMON_INC+=./src/hostlink_protocol.h
DAEMON_INC+=./src/log.h
DAEMON_CFLAGS=${CFLAGS}
DAEMON_CFLAGS+=$(shell pkg-config --cflags libusb-1.0)
DAEMON_LDFLAGS=${LDFLAGS}
DAEMON_LDFLAGS+=$(shell pkg-config --libs libusb-1.0)

DAEMON_LOOPBACK_BIN=./build/itdsrc_daemon_loopback
DAEMON_LOOPBACK_OBJS=./build/itdsrc_daemon_loopback.o
DAEMON_LOOPBACK_INC=./src/itdsrc_daemon.h
DAEMON_LOOPBACK_INC+=./src/log.h

ITDSRC_LIB=./build/libitdsrc.a
ITDSRC_LIB_OBJS=./build/itdsrc.o

all: daemon daemon_loopback lib

cleanup_queues: ./src/itdsrc_daemon.h ./include/itdsrc.h ./src/cleanup_queues.c
	${CC} ${CFLAGS} ./src/cleanup_queues.c -o ./build/cleanup_queues ${LDFLAGS}

###
daemon: setup ${DAEMON_BIN}

${DAEMON_BIN}: ${DAEMON_OBJS}
	${CC} ${DAEMON_CFLAGS} $^ -o $@ ${DAEMON_LDFLAGS}

###
daemon_loopback: setup ${DAEMON_LOOPBACK_BIN}

${DAEMON_LOOPBACK_BIN}: ${DAEMON_LOOPBACK_OBJS}
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS}

###
lib: setup ${ITDSRC_LIB}

${ITDSRC_LIB}: ${ITDSRC_LIB_OBJS}
	ar -rcs -o $@ $^

###
setup: ./build/

./build/:
	mkdir -p ./build/

install: daemon daemon_loopback lib
	install -d ${DESTDIR}/usr/lib ${DESTDIR}/usr/bin ${DESTDIR}/usr/include
	install build/libitdsrc.so ${DESTDIR}/usr/lib/
	install include/itdsrc.h ${DESTDIR}/usr/include/
	install build/itdsrc_daemon build/itdsrc_daemon_loopback ${DESTDIR}/usr/bin/

clean:
	rm ./build/*

distclean:
	rm -rf ./build

build/itdsrc.o: src/itdsrc.c
	${CC} ${CFLAGS} -c -fpic $< -o $@

build/%.o: src/%.c
	${CC} ${CFLAGS} -c $< -o $@
