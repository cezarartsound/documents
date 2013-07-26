PATH_CC = /opt/eldk-5.0/armv7a/sysroots/i686-oesdk-linux/usr/bin/armv7a-linux-gnueabi/
#CC = $(PATH_CC)arm-linux-gnueabi-gcc
CC = gcc
LIBS = -lpthread
LIBOUT = libSerialPort
OBJECTS = SerialPort.o


%.o: %.c
	$(CC) -c $(LIBS) -o $*.o $<

all: $(OBJECTS)
	ar -rcs $(LIBOUT).a $(OBJECTS)
	rm -f $(OBJECTS)
	
clean:
	rm -f $(OBJECTS)
	rm -f $(LIBOUT).a
	
	