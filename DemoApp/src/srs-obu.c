/*
 * IamHere.c
 *
 *  Created on: 12 de Jul de 2012
 *      Author: fabio32883
 */

#define _GNU_SOURCE 1
#define _THREAD_SAFE 1

#include <stdio.h>
#include <unistd.h> /* exit, write, read, ... */
#include <stdlib.h> /* atoi, ... */
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <signal.h> /* signal declaration */
#include <stdbool.h>

#include "vga/vga.h"
#include "window/window.h"
#include "itdsrc_mac.h"

#define TIME_PER_FRAME 100000 // time per frame in us

time_t now;
int last_min;

char data[100];
uint16_t data_length;

static bool done;
static bool rip;
int rx_good;
static bool tip;
static uint8_t mac_addr[6];

void clock_event(void * v){
	now = time(0);

	int min = (now / 60) % 60;;
	if(min != last_min){
		last_min = min;

		struct tm  tstruct;
		char       buf[80];
		tstruct = *localtime(&now);
		strftime(buf, sizeof(buf), "%H:%M", &tstruct);

		VGA_fill_rect(490,445,570,475, COLOR_WHITE);
		VGA_drawText(buf,500,465,COLOR_BLACK);
	}
}

void OBU_data(char * str) {
	VGA_fill_rect(190,215,410,275,COLOR_WHITE);
	VGA_drawText(str,230,255,COLOR_BLACK);
}

void RSU_data(char * str) {
	VGA_fill_rect(362,210,462,270,COLOR_WHITE);
	VGA_drawText(str,405,250,COLOR_BLACK);
}

void OBU_init() {
	VGA_init();
	VGA_setFontSize(24); // máxima
	event_init();

	VGA_drawImage("back.bmp",0,0);

	event_add(clock_event,0);
}

void indication_cb(const struct ma_unitdata_indication *indication);
void status_indication_cb(const struct ma_unitdata_status_indication *indication);
void xstatus_indication_cb(const struct ma_unitdatax_status_indication *indication);

void print_usage(void);
void signal_handler(int signal);

int main (int argc, char **argv)
{
	int retval;
	int optch;
	struct ma_dev *dev;
	char *optstring = "hd:p:c:m:b:r:";
	int id, backoff, count, modulation, power, retransmit_id;
	bool retransmit;
	uint8_t retransmit_mac_addr[6];
	int i;
	bool do_retransmit;

	id = -1;
	backoff = 0;
	count = 10;
	modulation = 3;
	power = 0;
	retransmit = false;

	done = false;

	do {
	    optch = getopt(argc, argv, optstring);
	
	    switch (optch) {
	        case 'h':
	            print_usage();
	            return 0;
	            break;
	
	        case 'b':
	            sscanf(optarg, "%d", &backoff);
	            break;
	
	        case 'c':
	            sscanf(optarg, "%d", &count);
	            break;
	
	        case 'd':
	            sscanf(optarg, "%d", &id);
	            break;
	
	        case 'm':
	            sscanf(optarg, "%d", &modulation);
	            break;
	
	        case 'p':
	            sscanf(optarg, "%d", &power);
	            break;

	        case 'r':
	            sscanf(optarg, "%d", &retransmit_id);
				retransmit = true;
	            break;
	
	        case -1:
	            break;
	
	        default:
	            print_usage();
	            return -1;
	            break;
	    }
	} while (optch != -1);

	if ((backoff > 7) || (backoff < 0)) {
		fprintf(stderr, "Backoff must be positive and not exceed %d\n", 7);
		print_usage();
		return -1;
	}

	if ((count > 99) || (count < 0)) {
		fprintf(stderr, "Count must be positive and not exceed %d\n", 99);
		print_usage();
		return -1;
	}
	
	if ((id < 0) || (id > 1)) {
	    fprintf(stderr, "OBU id must be specified and be 0 or 1\n");
	    print_usage();
		return -1;
	}

	if ((modulation > 7) || (modulation < 0)) {
		fprintf(stderr, "Modulation must be positive and not exceed %d\n", 7);
		print_usage();
		return -1;
	}

	if ((power > 63) || (power < 0)) {
		fprintf(stderr, "Power must be positive and not exceed %d\n", 7);
		print_usage();
		return -1;
	}
	
	if (retransmit) {
		if ((retransmit_id < 0) || (retransmit_id > 1)) {
			fprintf(stderr, "Retransmit OBU id must be 0 or 1\n");
			print_usage();
			return -1;
		}
	}
	retransmit_mac_addr[0] = 0;
	retransmit_mac_addr[1] = 0;
	retransmit_mac_addr[2] = 0;
	retransmit_mac_addr[3] = 0;
	retransmit_mac_addr[4] = 0;
	retransmit_mac_addr[5] = (uint8_t) retransmit_id;

	mac_addr[0] = 0;
	mac_addr[1] = 0;
	mac_addr[2] = 0;
	mac_addr[3] = 0;
	mac_addr[4] = 0;
	mac_addr[5] = (uint8_t) id;

	rip = false;
	rx_good = 0;

	retval = ma_init(0, &dev, &indication_cb, &status_indication_cb, &xstatus_indication_cb);
	if (retval != 0) {
		fprintf(stderr, "ERROR: failed to initialize mac layer: %s.\n", strerror(retval));
		return -1;
	}

	// register Ctrl-C to terminate
	signal(SIGINT, signal_handler);

	//printf("Press Ctrl-C to finish.\n");

	// INIT PROGRAM
	OBU_init();

	// just wait until the Ctrl-C signal arrives
	// sleep is to prevent process from hogging the CPU
	// it will be imediately interrupted on SIGINT
	while(!done) {
		usleep(10000); // 10ms sleep

		if (rip) {
			usleep(count*TIME_PER_FRAME); // wait for reception to finish

			do_retransmit = false;
			if (rx_good > 0) {
				OBU_data(data);
			} else {
				OBU_data("BAD");
			}

			if (retransmit && (rx_good > 0)) do_retransmit = true;

			// reset the system
			rip = false;
			rx_good = 0;

			// send frames to the other obu
			if (do_retransmit) {
				for (i=0; i < count; ++i) {
					tip = true;
					retval = ma_unitdatax_request(dev, mac_addr,
					retransmit_mac_addr, (uint8_t *) data, (uint16_t) data_length, 0, 0, 0, (uint8_t)modulation, (uint8_t)power, 0);

					if (retval != 0) {
						printf("ma_unitdatax_request failed: %s\n", strerror(retval));
						goto cleanup;
					} else {
						while(tip) {
							usleep(1000);
							if (done) goto cleanup;
						}
					}
				}
			}
		}
	}

cleanup:
	ma_stop(dev);

	return 0;
}

void indication_cb(const struct ma_unitdata_indication *indication)
{
	int i;

	// stop when device disconnection is detected
	if (indication->reception_status == 32) {
		raise(SIGINT);
		return;
	}

	rip = true;
	// only process good frames
	if (indication->reception_status == 0) {
		rx_good++;
		// check if frame is for this one (only last byte matters
		if (mac_addr[5] == (indication->destination_address)[5]) {
			// ensure the string is null terminated
			if (!tip) {
				strncpy(data, (char *)indication->data, (size_t) indication->data_length);
				data_length = indication->data_length;
			}
		}
	}

	//printf("DST: ");
	//for (i=0; i < 6; ++i) {
	//	printf("%2X ", indication->destination_address[i]);
	//}
	//printf("\n");
	//printf("SRC: ");
	//for (i=0; i < 6; ++i) {
	//	printf("%2X ", indication->source_address[i]);
	//}
	//printf("\n");
	//printf("FRAME(%d)\n", indication->data_length);
	//for (i=0; i < indication->data_length; ++i) {
	//	printf("%2X ", indication->data[i]);
	//}
	//printf("\n");
}

void status_indication_cb(const struct ma_unitdata_status_indication *indication)
{
	//printf("Status indication: %u.\n", indication->transmission_status);
}

void xstatus_indication_cb(const struct ma_unitdatax_status_indication *indication)
{
	tip = false;
	//printf("XStatus indication: %u.\n", indication->transmission_status);
}

void print_usage(void)
{
    printf("isel_obu -d <device_id> [-h]\n");
    printf("  -h : this help\n");
	printf("  -b : backoff slots (0)\n");
	printf("  -c : frame count (10)\n");
    printf("  -d <device_id> : id of the device (must be 0 or 1)\n");
	printf("  -m : modulation (3)\n");
	printf("  -p : transmission power (0)\n");
	printf("  -r : retransmit id (no retransmit)\n");
}

void signal_handler(int signal)
{
	done = true;
}
