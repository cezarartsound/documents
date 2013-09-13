CC = gcc
CFLAGS= -g -O0 -pthread -Wall #-Wextra 

LIBS = -lusb-1.0

VPATH = src:build:include

OBJECTS =  cleanup_queues itdsrc itdsrc_daemon itdsrc_protocol itdsrc_daemon_loopback hostlink_protocol


%.o: %.c
	$(CC) $(CFLAGS) -c $(LIBS) -o $*.o $<

all: $(OBJECTS) 

clean:
	rm -f $(OBJECTS)
	
