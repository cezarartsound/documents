


all: build/
	make -C userspacedriver
	make -C userspacemac
	cp userspacedriver/build/itdsrc.o ./build
	cp userspacemac/build/itdsrc_mac.o ./build
	cp userspacedriver/build/itdsrc_daemon ./build
	cp userspacedriver/build/itdsrc_daemon_loopback ./build
	
build/:
	mkdir -p build

clean:
	make -C userspacedriver clean
	make -C userspacemac clean
	rm -rf ./build
	
