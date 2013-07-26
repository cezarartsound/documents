CC = gcc
CFLAGS= -g -O0 -pthread -Wall -Wextra -Wno-unused-parameter
MAIN = GPS
MAIN_IN = src/$(MAIN).c
MAIN_OUT = src/$(MAIN)

LIBS = -L src/SerialPort -l SerialPort -pthread

VPATH = src:src/SerialPort

OBJECTS = SerialPort.o

SERIALPORT = libSerialPort.a

%.o: %.c
	$(CC) $(CFLAGS) -c $(LIBS) -o $*.o $<

all: $(OBJECTS) $(SERIALPORT)
	$(CC) $(CFLAGS) -o $(MAIN_OUT) $(MAIN_IN) $(OBJECTS) $(LIBS)
	rm -R -f $(OBJECTS)

$(SERIALPORT):
	make -C src/SerialPort

clean:
	rm -f $(OBJECTS)
	rm -f $(MAIN_OUT)  
	
