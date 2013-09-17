CC = gcc
CFLAGS= -g -O0 -pthread -Wall #-Wextra 
MAIN_IN = src/App.c
MAIN_OUT = build/App

#FB_LIB = -lFB
FB_LIB = -Lbuild -lFB -ljpeg -lpng -lncurses -lrt
LIBS = -lpthread $(FB_LIB) -lm

VPATH = src:src/so_tcplib:src/NetworkLayer:src/RoadView:src/libFabio/event/:src/GPS:src/GPS/SerialPort:src/libFabio/input/:src/libFabio/widget:src/MAC_API/build/:build/:objects/

FBLIB_PATH = src/libFB-0.2.4
FBLIB = libFB.a

MAC_API_PATH = src/MAC_API
MAC_API = itdsrc.o itdsrc_mac.o
MAC_API_EX = itdsrc_daemon
MAC_API_EX_LOOP = itdsrc_daemon_loopback

OBJECTS = SerialPort.o GPS.o so_tcplib.o event.o input.o widget.o Coor.o RoadView.o 

%.o: %.c
	$(CC) $(CFLAGS) -c $(LIBS) -o $*.o $<

all: $(FBLIB) $(MAC_API) $(OBJECTS) 
	$(CC) $(CFLAGS) -o $(MAIN_OUT) $(MAIN_IN) $(OBJECTS) $(MAC_API) $(LIBS)
	rm -R -f $(OBJECTS) $(MAIN) $(MAC_API)

$(FBLIB):
	make -C $(FBLIB_PATH)
	cp $(FBLIB_PATH)/$(FBLIB) build/

$(MAC_API):
	make -C $(MAC_API_PATH)
	cp -a $(MAC_API_PATH)/build/*.o .
	cp $(MAC_API_PATH)/build/$(MAC_API_EX) ./build
	cp $(MAC_API_PATH)/build/$(MAC_API_EX_LOOP) ./build
	
clean:
	rm -f $(OBJECTS)
	rm -f $(MAIN_OUT)
	rm -f build/$(MAC_API_EX)
	rm -f build/$(MAC_API_EX_LOOP)
	rm -f build/$(FBLIB)  
	make -C $(FBLIB_PATH) clean
	make -C $(MAC_API_PATH) clean
	
