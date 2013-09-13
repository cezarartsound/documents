
all: 
	make -C userspacedriver
	make -C userspacemac
	cp userspacedriver/build/itdsrc.o .
	cp userspacemac/build/itdsrc_mac.o .

clean:
	make -C userspacedriver clean
	make -C userspacemac clean
	rm userspacedriver/build/*
	rm userspacemac/build/*
