###############################################
# Below nothing interesting for non developer #
###############################################

LDFLAGS += -lFB -ljpeg

all:fbjpeg

install:libFB.so.0.0 tests fbjpeg
	cp fbjpeg $(BIN_PATH)

fbjpeg: fbjpeg.c ../FBlib.h
	gcc $(CFLAGS) -o fbjpeg fbjpeg.c -L.. $(LDFLAGS)

clean:
	rm -f fbjpeg

