PATH_CC = /opt/eldk-5.0/armv7a/sysroots/i686-oesdk-linux/usr/bin/armv7a-linux-gnueabi/
CC = $(PATH_CC)arm-linux-gnueabi-gcc
#CC = gcc
LIBS = -lpthread -Lsrc/GraphicInterface -lGraphicInterface -pthread -lrt
MAIN = src/RSU

VPATH = src/so_tcplib:src/NetworkLayer:src/MAC_API

OBJECTS =  so_tcplib.o itdsrc.o itdsrc_mac.o

%.o: %.c
	$(CC) -c $(LIBS) -o $*.o $<

all: $(MAIN).c $(OBJECTS)
	$(CC) -o $(MAIN) $(MAIN).c $(OBJECTS) $(LIBS)
	rm -R -f $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(MAIN)
	