all: font_and_keyb mouse fbfract
CFLAGS += -I..
LDFLAGS += -lFB

font_and_keyb: font_and_keyb.c
	gcc $(CFLAGS) -o font_and_keyb -L.. font_and_keyb.c $(LDFLAGS)

mouse: mouse.c
	gcc $(CFLAGS) -o mouse -L.. mouse.c $(LDFLAGS)

fbfract: fbfract.c
	gcc $(CFLAGS) -o fbfract -L.. fbfract.c $(LDFLAGS)

clean:
	rm -f font_and_keyb mouse
