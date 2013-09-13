CC=gcc 
override CFLAGS+=-Wall -Wextra -Wno-unused-parameter -Wno-format-extra-args -I ./include/ -I ../userspacedriver/include
override LDFLAGS+=-pthread -lrt

ITDSRC_MAC_LIB=build/libitdsrc_mac.a

all: itdsrc_mac

itdsrc_mac: setup ${ITDSRC_MAC_LIB}

${ITDSRC_MAC_LIB}: build/itdsrc_mac.o
	ar -rcs -o $@ $^

build/itdsrc_mac.o: ./src/itdsrc_mac.c ./include/itdsrc_mac.h
	${CC} ${CFLAGS} -fpic -c $< ${LDFLAGS} -o $@

examples: setup build/tx_stub build/rx_stub

build/%: examples/%.c
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS} -l itdsrc -l itdsrc_mac

setup: build/

build/:
	mkdir -p build

install: itdsrc_mac
	install -d ${DESTDIR}/usr/lib ${DESTDIR}/usr/include
	install build/libitdsrc_mac.so ${DESTDIR}/usr/lib/
	install include/itdsrc_mac.h ${DESTDIR}/usr/include/

clean:
	rm -rf ./build

distclean:
	rm -rf ./build
