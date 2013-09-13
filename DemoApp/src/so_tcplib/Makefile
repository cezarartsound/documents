CC=gcc 
LDFLAGS+=-pthread -lrt

LIB=build/libtcplib.a

all: lib

lib: setup ${LIB}

${LIB}: build/so_tcplib.o
	ar -rcs -o $@ $^

build/%.o: %.c
	${CC} ${CFLAGS} -c $^ -o $@ ${LDFLAGS}

setup: build/

build/:
	mkdir -p build

clean:
	rm -f ./build/*.o

distclean:
	rm -rf ./build
