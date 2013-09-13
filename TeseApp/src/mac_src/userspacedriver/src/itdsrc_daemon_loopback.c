#include <stdio.h>
#include <pthread.h>    /* POSIX threads */
#include <unistd.h>	    /* STDIN_FILENO */
#include <signal.h>     /* handle signals, such as Ctrl-C */
#include <stdlib.h>	    /* size_t */
#include <fcntl.h>	    /* For O_* constants */
#include <sys/stat.h>	/* For mode constants */
#include <mqueue.h>     /* POSIX message queues */
#include <stdbool.h>	/* bool type */
#include <errno.h>      /* errno variable and error constants */
#include <string.h>     /* strerror function */

#include "log.h"

#include "itdsrc_daemon.h"
#include "itdsrc.h"

#define INTER_DAEMON_QUEUE_NAME "/itdsrc_inter_daemon_queue"

static void print_usage(void);
static void signal_handler(int signal);
static void *rx_handler(void *arg);

static bool done;
static mqd_t writer_queue, reader_queue, bookie_queue;
static bool writer_queue_created, reader_queue_created, bookie_queue_created;
static mqd_t inter_daemon_my_queue, inter_daemon_your_queue;
static bool inter_daemon_my_queue_created, inter_daemon_your_queue_created;
static pthread_t rx_handler_thread;
static bool rx_handler_launched;

int main(int argc, char **argv)
{
	int retval;
	struct mq_attr writer_queue_attributes, reader_queue_attributes, bookie_queue_attributes;
	struct mq_attr inter_daemon_my_queue_attributes;
	char *optstring = "hvd:";
	int optch;
	bool verbose;
	struct itdsrc_request request;
	struct itdsrc_event event;
	int device_id;
	char inter_daemon_my_queue_name[100];
	char inter_daemon_your_queue_name[100];
	char writer_queue_name[100];
	char reader_queue_name[100];
	char bookie_queue_name[100];

	done = false;

	verbose = false;
	device_id = -1;

	do {
		optch = getopt(argc, argv, optstring);

		switch (optch) {
			case 'h':
				print_usage();
				return EXIT_SUCCESS;
				break;

			case 'v':
				verbose = true;
				break;

			case 'd':
				sscanf(optarg, "%d", &device_id);
				break;

			case -1:
				break;

			default:
				print_usage();
				return EXIT_FAILURE;
				break;
		}
	} while (optch != -1);

	if (verbose) {
		LOG_INIT(stdout, LOG_LEVEL_DEBUG);
	} else {
		LOG_INIT(stdout, LOG_LEVEL_ERROR);
	}

	if ((device_id < 0) || (device_id > 1)) {
		fprintf(stderr, "Device id must be 0 or 1\n");
		print_usage();
		goto cleanup;
	}

	// calculate the names of the queues
	strcpy(inter_daemon_my_queue_name, INTER_DAEMON_QUEUE_NAME);
	strcpy(inter_daemon_your_queue_name, INTER_DAEMON_QUEUE_NAME);
	if (device_id == 0) {
		strcat(inter_daemon_my_queue_name, "_0_1");
		strcat(inter_daemon_your_queue_name, "_1_0");
	} else {
		strcat(inter_daemon_my_queue_name, "_1_0");
		strcat(inter_daemon_your_queue_name, "_0_1");
	}

	strcpy(writer_queue_name, ITDSRC_WRITER_QUEUE_NAME);
	if (device_id == 0) {
		strcat(writer_queue_name, "_0");
	} else {
		strcat(writer_queue_name, "_1");
	}

	strcpy(reader_queue_name, ITDSRC_READER_QUEUE_NAME);
	if (device_id == 0) {
		strcat(reader_queue_name, "_0");
	} else {
		strcat(reader_queue_name, "_1");
	}

	strcpy(bookie_queue_name, ITDSRC_BOOKIE_QUEUE_NAME);
	if (device_id == 0) {
		strcat(bookie_queue_name, "_0");
	} else {
		strcat(bookie_queue_name, "_1");
	}


	// register Ctrl-C to terminate
	signal(SIGINT, signal_handler);
	//retval = signal(SIGINT, signal_handler);
	//if (retval == SIG_ERR) {
	//	LOG_ERROR("failed to register signal handler: %s", strerror(errno));
	//	goto cleanup;
	//}

	inter_daemon_my_queue_created = false;
	inter_daemon_your_queue_created = false;
	writer_queue_created = false;
	reader_queue_created = false;
	bookie_queue_created = false;
	rx_handler_launched = false;

	// queue attributes for the inter demon queue
	inter_daemon_my_queue_attributes.mq_flags = 0;
	inter_daemon_my_queue_attributes.mq_maxmsg = 10;
	inter_daemon_my_queue_attributes.mq_msgsize = ITDSRC_MAX_PACKET_SIZE;
	inter_daemon_my_queue_attributes.mq_curmsgs = 0;

	// create my inter demon queue
	inter_daemon_my_queue = mq_open(inter_daemon_my_queue_name, O_RDONLY | O_CREAT | O_EXCL, 0666, &inter_daemon_my_queue_attributes);
	if (inter_daemon_my_queue == -1) {
		LOG_ERROR("failed to create my inter daemon message queue: %s", strerror(errno));
		goto cleanup;
	}
	inter_daemon_my_queue_created = true;

	// try to connect to the other daemon queue
	while (!inter_daemon_your_queue_created) {
		inter_daemon_your_queue = mq_open(inter_daemon_your_queue_name, O_WRONLY);
		if (inter_daemon_your_queue == -1) {
			printf("Turn the other daemon on (Ctrl-C exits)\n");
		} else {
			inter_daemon_your_queue_created = true;
		}
		if (done) goto cleanup;
		sleep(1);
	}

	// queue attributes for the writer queue
	writer_queue_attributes.mq_flags = 0;
	writer_queue_attributes.mq_maxmsg = 10;
	writer_queue_attributes.mq_msgsize = sizeof(struct itdsrc_request);
	writer_queue_attributes.mq_curmsgs = 0;

	// create the writer queue
	writer_queue = mq_open(writer_queue_name, O_RDONLY | O_CREAT | O_EXCL, 0666, &writer_queue_attributes);
	if (writer_queue == -1) {
		LOG_ERROR("failed to create writer message queue: %s", strerror(errno));
		goto cleanup;
	}
	writer_queue_created = true;

	// queue attributes for the reader queue
	reader_queue_attributes.mq_flags = 0;
	reader_queue_attributes.mq_maxmsg = 10;
	reader_queue_attributes.mq_msgsize = ITDSRC_MAX_PACKET_SIZE;
	reader_queue_attributes.mq_curmsgs = 0;

	// create the reader queue
	reader_queue = mq_open(reader_queue_name, O_WRONLY | O_CREAT | O_EXCL, 0666, &reader_queue_attributes);
	if (reader_queue == -1) {
		LOG_ERROR("failed to create reader message queue: %s", strerror(errno));
		goto cleanup;
	}
	reader_queue_created = true;

	// queue attributes for the writer queue
	bookie_queue_attributes.mq_flags = 0;
	bookie_queue_attributes.mq_maxmsg = 10;
	bookie_queue_attributes.mq_msgsize = sizeof(struct itdsrc_event);
	bookie_queue_attributes.mq_curmsgs = 0;

	// create the queue to be used for transmission status
	bookie_queue = mq_open(bookie_queue_name, O_WRONLY | O_CREAT | O_EXCL, 0666, &bookie_queue_attributes);
	if (bookie_queue == -1) {
		LOG_ERROR("failed to create bookie message queue: %s", strerror(errno));
		goto cleanup;
	}
	bookie_queue_created = true;

	/* launch thread that handles the rx frames */
	retval = pthread_create(&rx_handler_thread, NULL, rx_handler, NULL);
	if (retval != 0) {
	    //fprintf(stderr, "failed to create event handler thread: %s\n", strerror(retval));
	    goto cleanup;
	}
	rx_handler_launched = true;

	printf("Press Ctrl-C to terminate\n");

	while (!done) {
		LOG_DEBUG("waiting for writer queue");

		retval = mq_receive(writer_queue, (char *)&request, sizeof(struct itdsrc_request), NULL);
		if (retval == -1) {
			// EBADF means that the queue was closed, which hapens when Ctrl-C
			// is pressed
			if (errno != EBADF) LOG_ERROR("failed to read from writer queue: %s", strerror(errno));
			continue;
		}

		switch (request.op) {
			case ITDSRC_OP_TX:
				LOG_DEBUG("operation (%d) just been forwarded to lower layers", request.handle);
				event.handle = request.handle;
				event.event = ITDSRC_EVENT_CMD_RECEIVED;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}

				LOG_DEBUG("tx requested (%d)", request.handle);
				event.handle = request.handle;
				event.event = ITDSRC_EVENT_TX_REQUESTED;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}

				LOG_DEBUG("tx (%d) just ended", request.handle);
				event.handle = 0;
				event.event = ITDSRC_EVENT_TX_FINISHED;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}

				// Put the packet in the other inter deamon queue
				retval = mq_send(inter_daemon_your_queue, (const char *)&(request.packet), request.packet_size, 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in my daemon queue: %s", strerror(errno));
				}
				break;

			case ITDSRC_OP_TX_DISCARD:
				LOG_DEBUG("operation (%d) just been forwarded to lower layers", request.handle);
				event.handle = request.handle;
				event.event = ITDSRC_EVENT_CMD_RECEIVED;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}

				LOG_DEBUG("discard tx (%d) succeeded", request.handle);
				event.handle = request.handle;
				event.event = ITDSRC_EVENT_TX_DISCARDED;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				break;

			case ITDSRC_OP_RX:
				LOG_DEBUG("operation (%d) just been forwarded to lower layers", request.handle);
				event.handle = request.handle;
				event.event = ITDSRC_EVENT_CMD_RECEIVED;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}

				LOG_DEBUG("rx requested (%d)", request.handle);
				event.handle = request.handle;
				event.event = ITDSRC_EVENT_RX_REQUESTED;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}

				// message is already in the reader queue because it is
				// automatically palced there when it arrives, in order to
				// simplify implementation

				break;

			case ITDSRC_OP_RX_DISCARD:
				LOG_DEBUG("operation (%d) just been forwarded to lower layers", request.handle);
				event.handle = request.handle;
				event.event = ITDSRC_EVENT_CMD_RECEIVED;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}

				LOG_DEBUG("rx discard (%d) done", request.handle);
				event.handle = request.handle;
				event.event = ITDSRC_EVENT_RX_DISCARDED;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				break;

			case ITDSRC_OP_CHANGE_CHANNEL:
				//LOG_DEBUG("operation (%d) just been forwarded to lower layers", status.fp_handle);
				//event.handle = request.handle;
				//event.event = ITDSRC_EVENT_CMD_RECEIVED;
				//event.packet_id = 0;
				//event.packet_size = 0;
				//retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				//if (retval == -1) {
				//	LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				//}

				//LOG_DEBUG("channel change done (%d)", status.tx_handle);
				//event.handle = request.handle;
				//event.event = ITDSRC_EVENT_CHANGE_CHANNEL_DONE;
				//event.packet_id = 0;
				//event.packet_size = 0;
				//retval = mq_send(bookie_queue, (const char *)&(event), //	sizeof(struct itdsrc_event), 0);
				//if (retval == -1) {
				//	LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				//}
				//break;
				break;

			default:
				LOG_DEBUG("operation %d is not valid", request.op);
				event.handle = request.handle;
				event.event = ITDSRC_EVENT_INVALID;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				break;
		}
	}

cleanup:
	LOG_DEBUG("cleaning up...");

	if (rx_handler_launched) {
		retval = pthread_join(rx_handler_thread, NULL);
		if (retval != 0) {
			fprintf(stderr, "failed to join rx handler thread\n: %s\n", strerror(retval));
		}
	}

	if (inter_daemon_my_queue_created) {
		retval = mq_unlink(inter_daemon_my_queue_name);
		if (retval == -1) {
			LOG_ERROR("failed to unlink inter daemon queue: %s", strerror(errno));
		}
	}

	if (writer_queue_created) {
		retval = mq_unlink(writer_queue_name);
		if (retval == -1) {
			LOG_ERROR("failed to unlink writer queue: %s", strerror(errno));
		}
	}

	if (reader_queue_created) {
		retval = mq_unlink(reader_queue_name);
		if (retval == -1) {
			LOG_ERROR("failed to unlink reader queue: %s", strerror(errno));
		}
	}

	if (bookie_queue_created) {
		retval = mq_unlink(bookie_queue_name);
		if (retval == -1) {
			LOG_ERROR("failed to unlink bookie queue: %s", strerror(errno));
		}
	}

	return 0;
}

void print_usage(void)
{
	printf("itdsrc-daemon-loopback -d <device_id> [-h] [-v]\n");
	printf("  -h : this help\n");
	printf("  -v : enable debug messages\n");
	printf("  -d <device_id> : id of the device to emulate (must be 0 or 1)\n");
}

void signal_handler(int signal)
{
	int retval;

	LOG_DEBUG("terminating...");

	if (inter_daemon_my_queue_created) mq_close(inter_daemon_my_queue);
	if (inter_daemon_your_queue_created) mq_close(inter_daemon_your_queue);
	if (writer_queue_created) mq_close(writer_queue);
	if (reader_queue_created) mq_close(reader_queue);
	if (bookie_queue_created) mq_close(bookie_queue);

	if (rx_handler_launched) {
		retval = pthread_cancel(rx_handler_thread);
		if (retval != 0) {
		    fprintf(stderr, "failed to cancel rx handler thread: %s\n", strerror(retval));
		    return;
		}
	}
	
	done = true;
}

void *rx_handler(void *arg)
{
	struct itdsrc_event event;
	uint8_t packet[ITDSRC_MAX_PACKET_SIZE];
	int retval;

	while (0==0) {
		retval = mq_receive(inter_daemon_my_queue, (char *)packet, ITDSRC_MAX_PACKET_SIZE, NULL);
		if (retval == -1) {
			if (errno != EBADF) LOG_ERROR("failed to read from inter daemon queue: %s", strerror(errno));
			continue;
		}

		event.packet_size = retval;

		// notify and place the packet in the reader queue
		event.handle = 0;
		event.event = ITDSRC_EVENT_RX_DETECTED;
		event.packet_id = 0;
		retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
		if (retval == -1) {
			LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
		}
		LOG_DEBUG("rx event signaled");
		retval = mq_send(reader_queue, (const char *)packet, event.packet_size, 0);
		if (retval == -1) {
			LOG_ERROR("failed to write in reader queue: %s", strerror(errno));
		}
		LOG_DEBUG("rx data placed on the reader queue");
	}

	return NULL;
}
