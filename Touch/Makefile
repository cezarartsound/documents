CC = gcc

FLAGS =  -Wall -O2
LIBS = 
MAIN = src/touchtest
VPATH = 
OBJECTS = 

%.o: %.c
	$(CC) -c $(LIBS) -o $*.o $<

all: $(MAIN).c $(OBJECTS)
	$(CC) $(FLAGS) -o $(MAIN) $(MAIN).c $(OBJECTS) $(LIBS)
	#rm -R -f $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(MAIN)
	
