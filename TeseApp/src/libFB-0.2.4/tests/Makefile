EXECUTABLES=test test1 test2 test3 test4 test5 test6 fbdb fbstat
CFLAGS += -I..
LDFLAGS += -lFB

all: $(EXECUTABLES) ../libFB.so.0.0

test: test.c
	gcc $(CFLAGS) -o test -L.. test.c $(LDFLAGS)

test1: test1.c
	gcc $(CFLAGS) -o test1 -L.. test1.c $(LDFLAGS)

test2: test2.c
	gcc $(CFLAGS) -o test2 -L.. test2.c $(LDFLAGS)
	
test3: test3.c
	gcc $(CFLAGS) -o test3 -L.. test3.c $(LDFLAGS)

test4: test4.c
	gcc $(CFLAGS) -o test4 -L.. test4.c $(LDFLAGS)
	
test5: test5.c
	gcc $(CFLAGS) -o test5 -L.. test5.c $(LDFLAGS) 
	
test6: test6.c
	gcc $(CFLAGS) -o test6 -L.. test6.c $(LDFLAGS) 

fbdb: fbdb.c
	gcc $(CFLAGS) -o fbdb -L.. fbdb.c $(LDFLAGS)

fbstat: fbstat.c
	gcc $(CFLAGS) -o fbstat -L.. fbstat.c $(LDFLAGS)

clean:
	rm -f $(EXECUTABLES) 
