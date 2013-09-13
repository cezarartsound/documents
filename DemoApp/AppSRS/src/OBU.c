/*
 * IamHere.c
 *
 *  Created on: 12 de Jul de 2012
 *      Author: fabio32883
 */

#define _GNU_SOURCE 1
#define _THREAD_SAFE 1


#include "so_tcplib/so_tcplib.h"




#include <stdio.h>
#include <unistd.h> /* exit, write, read, ... */
#include <stdlib.h> /* atoi, ... */
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "GraphicInterface/vga/vga.h"
#include "GraphicInterface/window/window.h"


#ifndef _BOOL_
#define _BOOL_
typedef enum bool {TRUE, FALSE} bool;
#endif /* _BOOL_ */



time_t now;
int last_min;

void clock_event(void * v){
	now = time(0) - 60*60;

	int min = (now / 60) % 60;;
	if(min != last_min){
		last_min = min;

		struct tm  tstruct;
		char       buf[80];
		tstruct = *localtime(&now);
		strftime(buf, sizeof(buf), "%H:%M", &tstruct);

		VGA_fill_rect(490,390,570,360,COLOR_WHITE);
		VGA_drawText(buf,500,385,COLOR_BLACK);
	}
}

void OBU_data(char * str){
	VGA_fill_rect(70,270,270,210,COLOR_WHITE);
	VGA_drawText(str,110,250,COLOR_BLACK);
}

void RSU_data(char * str){
	VGA_fill_rect(362,270,462,210,COLOR_WHITE);
	VGA_drawText(str,405,250,COLOR_BLACK);
}

void test(){
	OBU_data("14 km/h");
	RSU_data("45 km/h");
}

void OBU_init(){
	VGA_init();
	VGA_setFontSize(24); // máxima
	event_init();

	VGA_drawImage("back.bmp",0,0);

	event_add(clock_event,0);
}

/*
int main(int argc, char **argv)
{
	graphic_interface_init();
	test();
	while(1);
	return 0;
}
/* */


#include "MAC_API/itdsrc_mac.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> // sleep function, getopt
#include <stdbool.h>
#include <signal.h> // handle signals

void indication_cb(const struct ma_unitdata_indication *indication);
void status_indication_cb(const struct ma_unitdata_status_indication *indication);
void xstatus_indication_cb(const struct ma_unitdatax_status_indication *indication);

void print_usage(void);
void signal_handler(int signal);

static bool done;

int main (int argc, char **argv)
{
	int retval;
	int device_id;
	char optch;
	struct ma_dev *dev;
	char *optstring = "hd:";

	device_id = -1;
	done = false;

	do {
	    optch = getopt(argc, argv, optstring);

	    switch (optch) {
	        case 'h':
	            print_usage();
	            return 0;
	            break;

	        case 'd':
	            sscanf(optarg, "%d", &device_id);
	            break;

	        case -1:
	            break;

	        default:
	            print_usage();
	            return -1;
	            break;
	    }
	} while (optch != -1);

	if ((device_id < 0) || (device_id > 1)) {
	    fprintf(stderr, "Device id must be 0 or 1\n");
	    print_usage();
		return -1;
	}

	retval = ma_init(device_id, &dev, &indication_cb, &status_indication_cb, &xstatus_indication_cb);
	if (retval != 0) {
		fprintf(stderr, "ERROR: failed to initialize mac layer: %s.\n", strerror(retval));
		return -1;
	}

	// register Ctrl-C to terminate
	signal(SIGINT, signal_handler);

	printf("Press Ctrl-C to finish.\n");



	// INIT PROGRAM
	OBU_init();

	// just wait until the Ctrl-C signal arrives
	// sleep is to prevent process from hogging the CPU
	// it will be imediately interrupted on SIGINT
	while(!done) sleep(1);

	ma_stop(dev);

	return 0;
}

void indication_cb(const struct ma_unitdata_indication *indication)
{
	int i;

	//printf("Indication here.\n");
	printf("Received a frame from ");
	for (i=0; i<6; ++i) {
		printf("%02X", indication->source_address[i]);
	}
	printf(" to ");
	for (i=0; i<6; ++i) {
		printf("%02X", indication->destination_address[i]);
	}
	printf("\n");

	printf("Load: ");
	for (i=0; i<indication->data_length; ++i) {
		printf("%02X", indication->data[i]);
	}

	if(indication->data_length>3){
		switch(indication->data[0]){
		case(1):RSU_data(indication->data + 1);break;
		case(2):OBU_data(indication->data + 1);break;
		}
	}

	printf("\n");
}

void status_indication_cb(const struct ma_unitdata_status_indication *indication)
{
	printf("Status indication: %u.\n", indication->transmission_status);
}

void xstatus_indication_cb(const struct ma_unitdatax_status_indication *indication)
{
	printf("XStatus indication: %u.\n", indication->transmission_status);
}

void print_usage(void)
{
    printf("main-rx-stub -d <device_id> [-h]\n");
    printf("  -h : this help\n");
    printf("  -d <device_id> : id of the device to connect to (must be 0 or 1)\n");
}

void signal_handler(int signal)
{
	done = true;
}
/* */
