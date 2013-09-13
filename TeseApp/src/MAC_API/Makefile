


all:
	make -C userspacedriver
	make -C userspacemac
	cp userspacedriver/build/itdsrc.o .
	cp userspacemac/build/itdsrc_mac.o .
	cp userspacedriver/build/itdsrc_daemon .
	cp userspacedriver/build/itdsrc_daemon_loopback .
	
clean:
	make -C userspacedriver clean
	make -C userspacemac clean
	rm itdsrc.o itdsrc_mac.o itdsrc_daemon itdsrc_daemon_loopback
	