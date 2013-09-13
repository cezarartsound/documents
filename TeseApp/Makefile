CC = gcc
CFLAGS= -g -O0 -pthread -Wall #-Wextra 
MAIN_IN = src/App.c
MAIN_OUT = build/App

#FB_LIB = -lFB
FB_LIB = -Lbuild -lFB -lncurses -ljpeg -lpng
LIBS = -lpthread $(FB_LIB) -lm

VPATH = src:src/so_tcplib:src/NetworkLayer:src/RoadView:src/libFabio/event/:src/GPS:src/GPS/SerialPort:src/libFabio/input/:src/libFabio/widget:src/MAC_API:build/:objects/

OBJECTS =  itdsrc_mac.o itdsrc.o SerialPort.o GPS.o so_tcplib.o event.o input.o widget.o Coor.o RoadView.o 

FBLIB = libFB.a
FBLIB_PATH = src/libFB-0.2.4

%.o: %.c
	$(CC) $(CFLAGS) -c $(LIBS) -o $*.o $<

all: $(FBLIB) $(OBJECTS) 
	$(CC) $(CFLAGS) -o $(MAIN_OUT) $(MAIN_IN) $(OBJECTS) $(LIBS)
	rm -R -f $(OBJECTS) $(MAIN) $(GRAPHLIB)

$(FBLIB):
	make -C $(FBLIB_PATH)
	cp $(FBLIB_PATH)/$(FBLIB) build/
	rm $(FBLIB_PATH)/$(FBLIB)

clean:
	rm -f $(OBJECTS)
	rm -f $(MAIN_OUT)
	rm -f build/$(FBLIB)  
	
