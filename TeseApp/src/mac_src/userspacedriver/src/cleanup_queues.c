#include <mqueue.h>     /* POSIX message queues */
#include <stdbool.h>	/* bool type */
#include <string.h>     /* strerror function */
#include <stdio.h>
#include <unistd.h>     /* getopt */

#include "itdsrc_daemon.h"
#include "itdsrc.h"

void print_usage(void);

int main(int argc, char **argv)
{
	char *optstring = "hd:";
	char optch;
	int device_id;
	char writer_queue_name[100];
	char reader_queue_name[100];
	char bookie_queue_name[100];

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

	if (device_id < 0) {
		fprintf(stderr, "Device id must be greater than 0.\n");
		print_usage();
		return -1;
	}

	sprintf(writer_queue_name, "%s_%d", ITDSRC_WRITER_QUEUE_NAME, device_id);
	sprintf(reader_queue_name, "%s_%d", ITDSRC_READER_QUEUE_NAME, device_id);
	sprintf(bookie_queue_name, "%s_%d", ITDSRC_BOOKIE_QUEUE_NAME, device_id);

	printf("Deleting %s.\n", writer_queue_name);
	mq_unlink(writer_queue_name);
	printf("Deleting %s.\n", reader_queue_name);
	mq_unlink(reader_queue_name);
	printf("Deleting %s.\n", bookie_queue_name);
	mq_unlink(bookie_queue_name);

	return 0;
}

void print_usage(void)
{
	printf("cleanup-queues -d <device_id> [-h]\n");
	printf("  -h : this help\n");
	printf("  -d : device id\n");
}
