#include "itdsrc_mac.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>     /* sleep function, getopt */
#include <stdbool.h>
#include <signal.h>     /* handle signals */
#include <termios.h>    /* change terminal behaviour */
#include <sys/select.h> /* pselect */
#include <time.h>       /* clock_gettime */
#include <errno.h>      /* errno variable */

#define TIME_PER_FRAME 100000

void indication_cb(const struct ma_unitdata_indication *indication);
void status_indication_cb(const struct ma_unitdata_status_indication *indication);
void xstatus_indication_cb(const struct ma_unitdatax_status_indication *indication);

int read_uint(unsigned int *n);
bool chisnum(char ch);
void print_usage(void);
void signal_handler(int signal);

static bool done;
static bool tip;

int main (int argc, char **argv)
{
	int retval;
	int optch;
	struct ma_dev *dev;
	char *optstring = "hd:p:c:m:b:s";
	int backoff, count, modulation, power, destination_id;
	uint8_t mac_addr[6];
	uint8_t destination_mac_addr[6];
	uint8_t data[10];
	int data_length;
	int i;
	unsigned int speed;
	struct termios oldt, newt;
	bool terminal_configured;
	bool sequential_mode;

	backoff = 0;
	count = 10;
	modulation = 3;
	power = 0;
	destination_id = -1;

	done = false;
	terminal_configured = false;
	sequential_mode = false;

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
	            sscanf(optarg, "%d", &destination_id);
	            break;
	
	        case 'm':
	            sscanf(optarg, "%d", &modulation);
	            break;
	
	        case 'p':
	            sscanf(optarg, "%d", &power);
	            break;

			case 's':
				sequential_mode = true;
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
	
	if ((destination_id < 0) || (destination_id > 1)) {
	    fprintf(stderr, "Destination id must be specified and be 0 or 1\n");
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
	
	destination_mac_addr[0] = 0;
	destination_mac_addr[1] = 0;
	destination_mac_addr[2] = 0;
	destination_mac_addr[3] = 0;
	destination_mac_addr[4] = 0;
	destination_mac_addr[5] = (uint8_t) destination_id;

	mac_addr[0] = 0;
	mac_addr[1] = 0;
	mac_addr[2] = 0;
	mac_addr[3] = 0;
	mac_addr[4] = 0;
	mac_addr[5] = 11;

	retval = ma_init(0, &dev, &indication_cb, &status_indication_cb, &xstatus_indication_cb);
	if (retval != 0) {
		fprintf(stderr, "ERROR: failed to initialize mac layer: %s.\n", strerror(retval));
		return -1;
	}

	printf("RSU initialized and ready\n");
	printf("  Backoff     : %d\n", backoff);
	printf("  Frame count : %d\n", count);
	printf("  Modulation  : %d\n", modulation);
	printf("  Power       : %d\n", power);
	printf("  Destination : %d\n", destination_id);
	printf("\n");

	// register Ctrl-C to terminate
	signal(SIGINT, signal_handler);

	printf("Press Ctrl-C to finish.\n");
	printf("\n");

	if (!sequential_mode) {
		/* change the terminal behaviour for interaction */
		/* do not echo the input */
		/* pass the keystrokes without the need to press enter */
		retval = tcgetattr(STDIN_FILENO, &oldt);
		if (retval == -1) {
			fprintf(stderr, "ERROR: failed to get terminal configuration: %s.\n", strerror(retval));
			goto cleanup;
		}

		newt = oldt;
		newt.c_lflag &= ~(ICANON | ECHO);

		retval = tcsetattr(STDIN_FILENO, TCSANOW, &newt);
		if (retval == -1) {
			fprintf(stderr, "ERROR: failed to set terminal behaviour: %s.\n", strerror(retval));
			goto cleanup;
		}
		terminal_configured = true;
	}

	speed = 99;
	while(!done) {
		if (sequential_mode) {
			usleep(3*count*TIME_PER_FRAME);
			speed = (speed+1)%100;
		} else {
			printf("Speed: ");
			fflush(stdout);
			retval = read_uint(&speed);
			if (retval != 0) {
				//fprintf(stderr, "Failed to read number: %s.\n", strerror(retval));
				continue;
			}
			if (speed > 99) {
				printf("Speed must be beetwen 0 and 99\n");
				break;
			}
		}

		data[0] = speed;
		data_length = 1;

		printf("Sending %d to OBU %d.\n", speed, destination_id);

		// send frames to the other obu
		for (i=0; i < count; ++i) {
			tip = true;
			retval = ma_unitdatax_request(dev, mac_addr, destination_mac_addr, data, (uint16_t) data_length, 0, 0, 0, (uint8_t)modulation, (uint8_t)power, 0);
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

cleanup:
	printf("\n\n");
	printf("Stoping\n");

	if (terminal_configured) tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	ma_stop(dev);

	return retval;
}

void indication_cb(const struct ma_unitdata_indication *indication)
{
	// check to see if device was diconnected
	if(indication->reception_status == 32) {
		raise(SIGINT);
	}
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
    printf("  -d <device_id> : id of the destination device (must be 0 or 1)\n");
	printf("  -m : modulation (3)\n");
	printf("  -p : transmission power (0)\n");
}

void signal_handler(int signal)
{
	done = true;
}

int read_uint(unsigned int *n)
{
	char ch;
	bool done;
	int count;
	fd_set fset;
	struct timespec ts;
	int retval;

	done = false;
	count = 0;
	*n = 0;

	FD_ZERO(&fset);
	FD_SET(STDIN_FILENO, &fset);

	do {
		retval = clock_gettime(CLOCK_REALTIME, &ts);
		if (retval == -1) return errno; // some error occurred

		// wait for at most one second
		ts.tv_sec += 1;

		retval = pselect(STDIN_FILENO+1, &fset, NULL, NULL, &ts, NULL);
		if (retval == -1) return errno; // some error occurred
		else if (retval == 0) continue; // no data available

		ch = getchar();

		// ignore anything that is not a number or enter (CR)
		// accept backspace to erase
		if (chisnum(ch)) {
			printf("%c", ch); // echo is off, so emulate it
			*n = *n * 10;
			*n = *n + ch - '0';
			count++;
		} else if (ch == 0x7F) { // backspace
			if (count > 0) {
				*n = *n / 10;
				count--;
				printf("[D[K");
			}
		} else if (ch == '\n') { // carriage return
			printf("%c", ch); // echo is off, so emulate it
			done = true;
		}
		fflush(stdout);
	} while (!done);

	return 0;
}

bool chisnum(char ch)
{
	return ((ch >= '0') && (ch <= '9'));
}
