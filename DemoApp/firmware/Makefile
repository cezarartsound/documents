CC=gcc
LDFLAGS=$(shell pkg-config --libs libusb-1.0)
CFLAGS=$(shell pkg-config --cflags libusb-1.0)

FXLOAD_BIN=fxload
FXLOAD_SRC=fxload-usb/fxload.c fxload-usb/ezusb.c
FXLOAD_HDR=fxload-usb/ezusb.h

RULES=99-itdsrc.rules
FW=fw.ihx

all: fxload-usb

fxload-usb: ${FXLOAD_BIN}

${FXLOAD_BIN}: ${FXLOAD_SRC} ${FXLOAD_HDR}
	${CC} ${CFLAGS} ${FXLOAD_SRC} ${LDFLAGS} -o $@

clean:
	rm ${FXLOAD_BIN}

install: ${FXLOAD_BIN} ${FW} ${RULES}
	mkdir -p ${DESTDIR}/usr/bin ${DESTDIR}/usr/lib/firmware/itdsrc/ ${DESTDIR}/etc/udev/rules.d/
	install ${FXLOAD_BIN} ${DESTDIR}/usr/bin/
	install ${FW} ${DESTDIR}/usr/lib/firmware/itdsrc/
	install ${RULES} ${DESTDIR}/etc/udev/rules.d/
	

uninstall:
	rm ${DESTDIR}/usr/bin/${FXLOAD_BIN}
	rm ${DESTDIR}/usr/lib/firmware/itdsrc/${FW}
	rm ${DESTDIR}/etc/udev/rules.d/${RULES}
	rmdir -p --ignore-fail-on-non-empty ${DESTDIR}/usr/bin ${DESTDIR}/usr/lib/firmware/itdsrc/ ${DESTDIR}/etc/udev/rules.d/
