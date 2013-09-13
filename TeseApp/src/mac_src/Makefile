CC=gcc 
CFLAGS=-Wall -Wextra -Wno-unused-parameter
LDFLAGS=-pthread -lrt

ITDSRC_LIB=build/libitdsrc.a
ITDSRC_HDR=userspacedriver/include/

ITDSRC_MAC_LIB=build/libitdsrc_mac.a
ITDSRC_MAC_HDR=userspacemac/include/

SRS_CFLAGS=${CFLAGS} -I ${TCPLIB_HDR} -I ${GRAPHX_HDR} -I ${ITDSRC_HDR} -I ${ITDSRC_MAC_HDR}
SRS_LDFLAGS=${LDFLAGS} -L build/ -l itdsrc_mac -l itdsrc

ITDSRC_CFLAGS=${CFLAGS} -I ${ITDSRC_HDR} -I ${ITDSRC_MAC_HDR}
ITDSRC_LDFLAGS=${LDFLAGS} -L build/ -l itdsrc_mac -l itdsrc

all: itdsrc srs

itdsrc: ${ITDSRC_MAC_LIB} ${ITDSRC_LIB}

srs: ${ITDSRC_MAC_LIB} ${ITDSRC_LIB} ${GRAPHX_LIB} ${TCPLIB_LIB}

build/itdsrc-%: itdsrc-%.c
	${CC} ${ITDSRC_CFLAGS} $^ -o $@ ${ITDSRC_LDFLAGS}

${ITDSRC_LIB}: setup
	make -C userspacedriver/
	cp userspacedriver/build/libitdsrc.a build/
	cp userspacedriver/build/itdsrc_daemon build/
	cp userspacedriver/build/itdsrc_daemon_loopback build/

${ITDSRC_MAC_LIB}: setup
	make -C userspacemac/
	cp userspacemac/build/libitdsrc_mac.a build/

setup: build/

build/:
	mkdir -p build

install: itdsrc-obu itdsrc-rsu srs-obu srs-rsu
	install -d ${DESTDIR}/usr/bin
	install build/itdsrc-obu ${DESTDIR}/usr/bin/
	install build/itdsrc-rsu ${DESTDIR}/usr/bin/
	install build/srs-obu ${DESTDIR}/usr/bin/
	install build/srs-rsu ${DESTDIR}/usr/bin/
	install build/itdsrc_daemon ${DESTDIR}/usr/bin/
	install build/itdsrc_daemon_loopback ${DESTDIR}/usr/bin/

clean:
	make -C userspacemac/ clean
	make -C userspacedriver/ clean
	rm -f ./build/*.a

distclean: clean
	rm -rf ./build
