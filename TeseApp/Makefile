CC = gcc
CFLAGS= -g -O0 -pthread -Wall #-Wextra 
MAIN_IN = src/App.c
MAIN_OUT = build/App

#FB_LIB = -lFB
FB_LIB = -Lbuild -lFB -lncurses -ljpeg -lpng -lrt
LIBS = -lpthread $(FB_LIB) -lm

VPATH = src:src/so_tcplib:src/NetworkLayer:src/RoadView:src/libFabio/event/:src/GPS:src/GPS/SerialPort:src/libFabio/input/:src/libFabio/widget:src/MAC_API:build/:objects/

FBLIB_PATH = src/libFB-0.2.4
FBLIB = $(FBLIB_PATH)/libFB.a

MAC_API_PATH = src/MAC_API
MAC_API = $(MAC_API_PATH)/itdsrc.o $(MAC_API_PATH)/itdsrc_mac.o

OBJECTS = $(MAC_API) SerialPort.o GPS.o so_tcplib.o event.o input.o widget.o Coor.o RoadView.o 

%.o: %.c
	$(CC) $(CFLAGS) -c $(LIBS) -o $*.o $<

all: $(FBLIB) $(MAC_API) $(OBJECTS) 
	$(CC) $(CFLAGS) -o $(MAIN_OUT) $(MAIN_IN) $(OBJECTS) $(LIBS)
	rm -R -f $(OBJECTS) $(MAIN) $(GRAPHLIB) $(MAC_API)

$(FBLIB):
	make -C $(FBLIB_PATH)
	cp $(FBLIB) build/

$(MAC_API):
	make -C $(MAC_API_PATH)
	
clean:
	rm -f $(OBJECTS)
	rm -f $(MAIN_OUT)
	rm -f $(MAC_API)
	make -C $(MAC_API_PATH) clean
	make -C $(FBLIB_PATH) clean
	
