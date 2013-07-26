PATH_CC = /opt/eldk-5.0/armv7a/sysroots/i686-oesdk-linux/usr/bin/armv7a-linux-gnueabi/
#CC = $(PATH_CC)arm-linux-gnueabi-gcc
CC = gcc

FLAGS =  -Wall -O2
LIBS = -lpthread -lm
#MAIN = src/ezdib/main
#MAIN = src/widgettest
MAIN = src/vgatest
#MAIN = src/windowtest
VPATH = src/vga/ezdib src/vga/font src/vga src/widget src/event src/input src/window
OBJECTS = my_math.o ezdib.o font.o vga.o input.o widget.o event.o window.o
LIBOUT = libGraphicInterface

%.o: %.c
	$(CC) -c $(LIBS) -o $*.o $<

all: $(MAIN).c $(OBJECTS)
	$(CC) $(FLAGS) -o $(MAIN) $(MAIN).c $(OBJECTS) $(LIBS)
	ar -rcs $(LIBOUT).a $(OBJECTS)
	rm -f $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(MAIN)
	
