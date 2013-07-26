########################################################
# You should look at and change these before compiling #
########################################################
# Install paths
# Where to install libraries
LIB_PATH=/usr/lib
# Where to install binaries
BIN_PATH=/usr/bin
# Where to install includes
INC_PATH=/usr/include

# Other paths
# Kernel sources (only includes are needed)
KERN_SRC=/usr/src/linux
# Default font file (see README.fonts for help)
FONT_PATH=\"/usr/share/consolefonts/iso01.f14.psf.gz\"
# FONT_PATH=\"/usr/share/kbd/consolefonts/tcvn8x16.psf.gz\"
# FONT_PATH=\"/usr/share/kbd/consolefonts/lat9-16.psf.gz\"
# FONT_PATH=\"/usr/share/kbd/consolefonts/lat7a-16.psf.gz\" # pq n da ???

# C compiler to use
CC=gcc

# Debug options
# Clear if you want no debugging
DEBUG_OPT=-g -Wall

# Other compiler flags
# For Intel Pentium 2 or greater
OPTIMIZE=-O2
# For other pentiums (mmx and standard) and k6 1 & 2
# OPTIMIZE=-O2 -march=i586 -mcpu=i586
# Everything else
# OPTIMIZE=-O2

###############################################
# Below nothing interesting for non developer #
###############################################
CFLAGS=$(DEBUG_OPT) $(OPTIMIZE) -I$(KERN_SRC)/include -DDEFAULT_FONT=$(FONT_PATH)
OBJS=FButil.o FBdraw.o FBinit.o FBkeyb.o FBfont.o FBfabio.o

LDFLAGS=-lncurses -lm -ljpeg -lpng

all: libFB.a

libFB.a: $(OBJS)
	$(CC) -shared -Wl,-soname,libFB.so.0 -o libFB.so.0.0 $(OBJS) $(LDFLAGS)
	ar -rcs libFB.a $(OBJS)
	rm -f $(OBJS)

#all:libFB.so.0.0 tests examples fbjpeg

libFB.so.0.0: $(OBJS)
	$(CC) -shared -Wl,-soname,libFB.so.0 -o libFB.so.0.0 $(OBJS) $(LDFLAGS)
	ln -s -f libFB.so.0.0 libFB.so
	ln -s -f libFB.so.0.0 libFB.so.0

install:libFB.so.0.0 tests fbjpeg
	cp libFB.so.0.0 $(LIB_PATH)
	ln -s -f $(LIB_PATH)/libFB.so.0.0 $(LIB_PATH)/libFB.so.0
	ln -s -f $(LIB_PATH)/libFB.so.0.0 $(LIB_PATH)/libFB.so
	/sbin/ldconfig
	cp FBlib.h $(INC_PATH)
	$(MAKE) -C fbjpeg install

FBdraw.o: FBdraw.c FBlib.h FBpriv.h
	$(CC) -o FBdraw.o -c -fPIC $(CFLAGS) FBdraw.c

FButil.o: FButil.c FBlib.h FBpriv.h
	$(CC) -o FButil.o -c -fPIC $(CFLAGS) FButil.c

FBinit.o: FBinit.c FBlib.h FBpriv.h
	$(CC) -o FBinit.o -c -fPIC $(CFLAGS) FBinit.c
	
FBkeyb.o: FBkeyb.c keys.h
	$(CC) -o FBkeyb.o -c -fPIC $(CFLAGS) FBkeyb.c

FBfont.o: FBfont.c
	$(CC) -o FBfont.o -c -fPIC $(CFLAGS) FBfont.c
	
FBfabio.o: FBfabio.c
	$(CC) -o FBfabio.o -c -fPIC $(CFLAGS) FBfabio.c
	
clean:
	rm -f libFB.a
	rm -f $(OBJS)
	rm -f libFB.so.0.0 libFB.so libFB.so.0
	$(MAKE) -C tests clean
	$(MAKE) -C examples clean
	$(MAKE) -C fbjpeg clean

clean_bak:
	rm *~
	rm *.bak

tests:
	$(MAKE) -C tests

examples:
	$(MAKE) -C examples

fbjpeg: 
	$(MAKE) -C fbjpeg

.PHONY: tests examples fbjpeg
export CFLAGS BIN_PATH LDFLAGS
