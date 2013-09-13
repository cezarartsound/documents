#include "itdsrc_mac.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> /* sleep function, getopt */
#include <stdbool.h>
#include <signal.h> /* handle signals */

#define TIME_PER_FRAME 100000 // in us

void indication_cb(const struct ma_unitdata_indication *indication);
void status_indication_cb(const struct ma_unitdata_status_indication *indication);
void xstatus_indication_cb(const struct ma_unitdatax_status_indication *indication);

void print_usage(void);
void signal_handler(int signal);

static bool done;
static int rx_counter, rx_counter_good, rx_counter_bad;
static int speed;
static bool rip;
static bool tip;
static uint8_t mac_addr[6];

int main (int argc, char **argv)
{
	int retval;
	int optch;
	struct ma_dev *dev;
	char *optstring = "hd:p:c:m:b:r:";
	int id, backoff, count, modulation, power, retransmit_id;
	bool retransmit;
	uint8_t retransmit_mac_addr[6];
	uint8_t data[10];
	int data_length = 1;
	int i;

	id = -1;
	backoff = 0;
	count = 10;
	modulation = 3;
	power = 0;
	retransmit = false;

	done = false;

	rx_counter = 0;
	rx_counter_good = 0;
	rx_counter_bad = 0;
	
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

	retval = ma_init(0, &dev, &indication_cb, &status_indication_cb, &xstatus_indication_cb);
	if (retval != 0) {
		fprintf(stderr, "ERROR: failed to initialize mac layer: %s.\n", strerror(retval));
		return -1;
	}

	printf("OBU initialized and ready\n");
	printf("  ID          : %d\n", id);
	printf("  Backoff     : %d\n", backoff);
	printf("  Frame count : %d\n", count);
	printf("  Modulation  : %d\n", modulation);
	printf("  Power       : %d\n", power);
	if (retransmit) printf("  Retransmit  : true (%d)\n", retransmit_id);
	else printf("  Retransmit  : false \n");
	printf("\n");

	// register Ctrl-C to terminate
	signal(SIGINT, signal_handler);

	printf("Press Ctrl-C to finish.\n");
	printf("\n");
	printf("SPEED (GOOD/TOTAL/EXPECTED)\n");

	// just wait until the Ctrl-C signal arrives
	// sleep is to prevent process from hogging the CPU
	// it will be imediately interrupted on SIGINT
	while(!done) {
		usleep(10000); // 10ms sleep

		if (rip) {
			usleep(count*TIME_PER_FRAME); // wait for reception to finish
			printf("%4d: %3d/%3d/%3d\n", speed, rx_counter_good, rx_counter, count);

			// reset the system
			rip = false;
			rx_counter = 0;
			rx_counter_good = 0;

			// send frames to the other obu
			if (retransmit) {
				data[0] = speed;
				for (i=0; i < count; ++i) {
					tip = true;
					retval = ma_unitdatax_request(dev, mac_addr, retransmit_mac_addr, data, (uint16_t) data_length, 0, 0, 0, (uint8_t)modulation, (uint8_t)power, 0);

					if (retval != 0) {
						printf("ma_unitdatax_request failed: %s\n", strerror(retval));
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
	printf("Stoping\n");

	ma_stop(dev);

	return 0;
}

void indication_cb(const struct ma_unitdata_indication *indication)
{
	//int i;

	// check to see if device was diconnected
	if(indication->reception_status == 32) {
		done = true;
		return;
	}

	// check if frame is for this one (only last byte matters
	if (mac_addr[5] == (indication->destination_address)[5]) {
		rip = true;
		rx_counter++;
		speed = indication->data[0];
		if (indication->reception_status == 0) rx_counter_good++;
	}

	//printf("source: ");
	//for (i = 0; i < 6; ++i) {
	//		printf("%02X:", indication->source_address[i]);
	//}
	//printf("\n");
	//printf("destination: ");
	//for (i = 0; i < 6; ++i) {
	//		printf("%02X:", indication->destination_address[i]);
	//}
	//printf("\n");
	//printf("data: ");
	//for (i = 0; i < indication->data_length; ++i) {
	//		printf("%02X:", indication->data[i]);
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
	printf("obu -d <device_id> [-h]\n");
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
