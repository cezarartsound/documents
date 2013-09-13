#include <libusb-1.0/libusb.h>
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

#include "hostlink_protocol.h"
#include "itdsrc_protocol.h"
#include "log.h"
#include "itdsrc_daemon.h"
#include "itdsrc.h"

#define BULK_OUT_EP 0x02
#define BULK_IN_EP 0x86
#define INT_IN_EP 0x88

#define BULK_OUT_TRANSFER_SIZE (3*ITDSRC_MAX_PACKET_SIZE)
#define BULK_IN_TRANSFER_SIZE (ITDSRC_MAX_PACKET_SIZE)
#define INT_IN_TRANSFER_SIZE 11
#define INT_IN_TIMEOUT_MS 1000 // interrupt in timeout, in miliseconds

static void print_usage(void);
static void signal_handler(int signal);

static void *writer(void *arg);
static void *bookie(void *arg);

static pthread_t writer_thread, bookie_thread;
static mqd_t writer_queue, reader_queue, bookie_queue;

int main(int argc, char **argv)
{
	int retval;
	struct mq_attr writer_queue_attributes, reader_queue_attributes, bookie_queue_attributes;
	bool libusb_inited, device_claimed, device_opened;
	bool writer_queue_created, reader_queue_created, bookie_queue_created;
	char *optstring = "hv";
	int optch;
	bool verbose;
//	int device_id;
	char writer_queue_name[100];
	char reader_queue_name[100];
	char bookie_queue_name[100];
	libusb_device_handle *dev;

	verbose = false;

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

//			case 'd':
//				sscanf(optarg, "%d", &device_id);
//				break;
//
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

//	if ((device_id < 0) || (device_id > 1)) {
//		fprintf(stderr, "Device id must be 0 or 1\n");
//		print_usage();
//		goto cleanup;
//	}

	libusb_inited = false;
	device_opened = false;
	device_claimed = false;
	writer_queue_created = false;
	reader_queue_created = false;
	bookie_queue_created = false;

	// register Ctrl-C to terminate
	signal(SIGINT, signal_handler);
	//retval = signal(SIGINT, signal_handler);
	//if (retval == SIG_ERR) {
	//  LOG_ERROR("failed to register signal handler: %s", (errno));
	//  goto cleanup;
	//}

	sprintf(writer_queue_name, ITDSRC_WRITER_QUEUE_NAME "_0");

	sprintf(reader_queue_name, ITDSRC_READER_QUEUE_NAME "_0");

	sprintf(bookie_queue_name, ITDSRC_BOOKIE_QUEUE_NAME "_0");

	// queue attributes for the writer queue
	writer_queue_attributes.mq_flags = 0;
	writer_queue_attributes.mq_maxmsg = 10;
	writer_queue_attributes.mq_msgsize = sizeof(struct itdsrc_request);
	writer_queue_attributes.mq_curmsgs = 0;

	// create the writer queue
	writer_queue = mq_open(writer_queue_name, O_RDWR | O_CREAT | O_EXCL, 0666, &writer_queue_attributes);
	if (writer_queue == -1) {
		LOG_ERROR("failed to create writer message queue: %s", strerror(errno));
		goto cleanup;
	}
	writer_queue_created = true;

	// queue attributes for the writer queue
	reader_queue_attributes.mq_flags = 0;
	reader_queue_attributes.mq_maxmsg = 10;
	reader_queue_attributes.mq_msgsize = ITDSRC_MAX_PACKET_SIZE;
	reader_queue_attributes.mq_curmsgs = 0;

	// create the reader queue
	reader_queue = mq_open(reader_queue_name, O_RDWR | O_CREAT | O_EXCL, 0666, &reader_queue_attributes);
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
	bookie_queue = mq_open(bookie_queue_name, O_RDWR | O_CREAT | O_EXCL, 0666, &bookie_queue_attributes);
	if (bookie_queue == -1) {
		LOG_ERROR("failed to create bookie message queue: %s", strerror(errno));
		goto cleanup;
	}
	bookie_queue_created = true;

	/* initialize the thing */
	retval = libusb_init(NULL);
	if (retval != 0) {
		LOG_ERROR("libusb_init(NULL) failed: %s", libusb_error_name(retval));
		goto cleanup;
	}
	libusb_inited = true;

	/* open device */
	dev = libusb_open_device_with_vid_pid(NULL, 0x04b4, 0x1004);
	if (dev == NULL) {
		LOG_ERROR("libusb_open(NULL, 0x04b4, 0x1004) failed");
		printf("Is the device connected and with loaded firmware?\n");
		goto cleanup;
	}
	device_opened = true;

	/* ensure the communication channels are clean */
	retval = libusb_reset_device(dev);
	if (retval != 0) {
		LOG_ERROR("libusb_reset_device(dev) failed: %s", libusb_error_name(retval));
		goto cleanup;
	}

	/* ensure no one else is using the device */
	retval = libusb_claim_interface(dev, 0);
	if (retval != 0) {
		LOG_ERROR("libusb_claim_interface(dev, 0) failed: %s", libusb_error_name(retval));
		goto cleanup;
	}
	device_claimed = true;

	/* launch the threads */
	retval = pthread_create(&writer_thread, NULL, writer, (void *)dev);
	if (retval != 0) {
		LOG_ERROR("failed to create writer thread: %s", strerror(retval));
		goto cleanup;
	}
	retval = pthread_create(&bookie_thread, NULL, bookie, (void *)dev);
	if (retval != 0) {
		LOG_ERROR("failed to create bookie thread: %s", strerror(retval));
		goto cleanup;
	}

	/* wait for termination and clean up */
	pthread_join(writer_thread, NULL);
	LOG_DEBUG("joined writer thread");
	pthread_join(bookie_thread, NULL);
	LOG_DEBUG("joined bookie thread");

cleanup:
	LOG_DEBUG("claening up");

	if (device_claimed) {
		retval = libusb_release_interface(dev, 0);
		if (retval != 0) {
			LOG_ERROR("libusb_release_interface(dev, 0) failed: %s", libusb_error_name(retval));
		}
	}

	if (device_opened) {
		libusb_close(dev);
	}

	if (libusb_inited) {
		libusb_exit(NULL);
	}

	if (writer_queue_created) {
		retval = mq_unlink(writer_queue_name);
		if (retval == -1) {
			LOG_ERROR("failed to unlink writer queue: %s", strerror(errno));
		}
	}
	LOG_DEBUG("%s queue unlinked", writer_queue_name);

	if (reader_queue_created) {
		retval = mq_unlink(reader_queue_name);
		if (retval == -1) {
			LOG_ERROR("failed to unlink reader queue: %s", strerror(errno));
		}
	}
	LOG_DEBUG("%s queue unlinked", reader_queue_name);

	if (bookie_queue_created) {
		retval = mq_unlink(bookie_queue_name);
		if (retval == -1) {
			LOG_ERROR("failed to unlink bookie queue: %s", strerror(errno));
		}
	}
	LOG_DEBUG("%s queue unlinked", bookie_queue_name);

	return 0;
}

void print_usage(void)
{
	printf("itdsrc-daemon [-h] [-v]\n");
	printf("  -h : this help\n");
	printf("  -v : enable debug messages\n");
}

void signal_handler(int signal)
{
	LOG_DEBUG("canceling all threads...");

	pthread_cancel(writer_thread);
	LOG_DEBUG("writer thread canceled");
	pthread_cancel(bookie_thread);
	LOG_DEBUG("bookie thread canceled");

	LOG_DEBUG("all threads canceled");
}

void *writer(void *arg)
{
	libusb_device_handle *dev;
	uint8_t buf[BULK_OUT_TRANSFER_SIZE];
	size_t buf_size;
	int buf_max_size;
	int count;
	struct itdsrc_request request;
	struct itdsrc_cmd cmd;
	uint8_t header[10];
	size_t header_size;
	int retval;

	buf_max_size = BULK_OUT_TRANSFER_SIZE;
	
	dev = (libusb_device_handle *)arg;

	while (0==0) {
		pthread_testcancel();

		LOG_DEBUG("waiting for writer queue");

		retval = mq_receive(writer_queue, (char *)&request, sizeof(struct itdsrc_request), NULL);
		if (retval == -1) {
			LOG_ERROR("failed to read from writer queue: %s", strerror(errno));
		}
		
		// convert from request to cmd
		cmd.handle      = request.handle;
		cmd.op          = request.op;
		cmd.packet_id   = request.packet_id;
		cmd.packet_size = request.packet_size;
		cmd.power       = request.power;
		cmd.bitrate     = request.bitrate;
		cmd.backoff     = request.backoff;
		cmd.channel_id  = request.channel_id;

		itdsrc_cmd_to_header(cmd, header, &header_size);

		if (cmd.op == ITDSRC_OP_TX) {
			stuff_packet(header, header_size, request.packet, request.packet_size, buf, buf_max_size, &buf_size);
		} else {
			stuff_packet(header, header_size, NULL, 0, buf, buf_max_size, &buf_size);
		}
		
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

		retval = libusb_bulk_transfer(dev, BULK_OUT_EP, buf, buf_size, &count, 1000);
		if (((size_t)count != buf_size) || retval != 0) {
			LOG_ERROR("failed to send the full thing (%d of %zd): %s", count, buf_size, libusb_error_name(retval));
		} else {
			LOG_DEBUG("sent new request(%d) to device", cmd.op);
		}

		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}

	return NULL;
}

void *bookie(void *arg)
{
	libusb_device_handle *dev;
	uint8_t buf[INT_IN_TRANSFER_SIZE];
	struct itdsrc_event event;
	struct itdsrc_status status;
	int retval;
	int count;
	uint8_t bufb[BULK_IN_TRANSFER_SIZE];

	dev = (libusb_device_handle *)arg;

	while (0==0) {
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		pthread_testcancel();
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

		retval = libusb_interrupt_transfer(dev, INT_IN_EP, buf, INT_IN_TRANSFER_SIZE, &count, INT_IN_TIMEOUT_MS);
		if (retval != 0) {
			// print only when error is not timeout
			if (retval != -7) {
				LOG_ERROR("libusb_interrupt_transfer() failed %d: %s", retval, libusb_error_name(retval));
				raise(SIGINT);
				break;
			}
			//sleep(1);
			continue;
		}
		if (count != INT_IN_TRANSFER_SIZE) {
			LOG_ERROR("interrupt transfer received %d bytes, should have received %d", count, INT_IN_TRANSFER_SIZE);
			continue;
		}

		parse_status(buf, count, &status);

		/* Frame processor */
		switch (status.fp_status) {
			case 0:
				// nothing to do here
				break;

			case 1:
				// operation just been forwarded to lower layers
				LOG_DEBUG("operation (%d) just been forwarded to lower layers", status.fp_handle);
				event.handle = status.fp_handle;
				event.event = ITDSRC_EVENT_CMD_RECEIVED;
				event.status = 0;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				break;

			default:
				LOG_DEBUG("unrecognized fp status received: %d", status.fp_status);

				event.handle = 0;
				event.event = ITDSRC_EVENT_INVALID;
				event.status = 0;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				break;
		}

		switch (status.tx_status) {
			case 0:
				// nothing to do here
				break;

			case 1:
				// discard tx succeeded
				LOG_DEBUG("discard tx (%d) succeeded", status.tx_handle);

				event.handle = status.tx_handle;
				event.event = ITDSRC_EVENT_TX_DISCARDED;
				event.status = 0;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				break;

			case 2:
				// tx requested
				LOG_DEBUG("tx requested (%d)", status.tx_handle);

				event.handle = status.tx_handle;
				event.event = ITDSRC_EVENT_TX_REQUESTED;
				event.status = 0;
				event.packet_id = status.tx_packet_id;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				break;
			
			case 3:
				// tx just ended
				LOG_DEBUG("tx (%d) just ended", status.tx_handle);

				event.handle = 0;
				event.event = ITDSRC_EVENT_TX_FINISHED;
				event.status = 0;
				event.packet_id = status.tx_packet_id;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				break;

			case 4:
				// tx failed
				LOG_DEBUG("tx failed (%d)", status.tx_handle);

				event.handle = status.tx_handle;
				event.event = ITDSRC_EVENT_TX_FAILED;
				event.status = 0;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				break;

			//case 5:
			//	// change channel succeeded
			//	LOG_DEBUG("channel change done (%d)", status.tx_handle);

			//	event.handle = status.tx_handle;
			//	event.event = ITDSRC_EVENT_CHANGE_CHANNEL_DONE;
			//	event.packet_id = 0;
			//	event.packet_size = 0;
			//	retval = mq_send(bookie_queue, (const char *)&(event), //	sizeof(struct itdsrc_event), 0);
			//	if (retval == -1) {
			//		LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
			//	}
			//	break;

			default:
				LOG_DEBUG("unrecognized tx status received: %d", status.tx_status);

				event.handle = 0;
				event.event = ITDSRC_EVENT_INVALID;
				event.status = 0;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				break;
		}

		switch (status.rx_op_status) {
			case 0:
				// do nothing
				break;

			case 1:
				// rx discard request processed with success
				LOG_DEBUG("rx discard (%d) done", status.rx_op_handle);

				event.handle = status.rx_op_handle;
				event.event = ITDSRC_EVENT_RX_DISCARDED;
				event.status = 0;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				break;

			case 2:
				// rx request processed with success
				LOG_DEBUG("rx requested (%d)", status.rx_op_handle);

				event.handle = status.rx_op_handle;
				event.event = ITDSRC_EVENT_RX_REQUESTED;
				event.status = 0;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}

				retval = libusb_bulk_transfer(dev, BULK_IN_EP, bufb, BULK_IN_TRANSFER_SIZE, &count, 1000);
				if (retval != 0) {
					// print only when error is not timeout
					//if (retval != -7) {
						LOG_ERROR("libusb_bulk_transfer() failed %d: %s", retval, libusb_error_name(retval));
					//}
					//sleep(1);
					break;;
				}
				retval = mq_send(reader_queue, (const char *)bufb, count, 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				LOG_DEBUG("read packet from device");
				break;
		}

		switch (status.rx_rx_status) {
			case 0:
				// nothing to do
				break;

			case 2:
				// new packet received
				LOG_DEBUG("rx detected");

				event.handle = 0;
				event.event = ITDSRC_EVENT_RX_DETECTED;
				event.status = 0;
				event.packet_id = status.rx_rx_packet_id;
				event.packet_size = status.rx_rx_packet_size;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				break;

			default:
				// this code is under the dafault clause because the error code
				// goes on the upper nibble of the rx_rx_status byte

				// rx error detected
				LOG_DEBUG("rx error detected");

				event.status = 0;

				// also store the reason
				if ((status.rx_rx_status & 0x80) != 0) {
					event.status = ITDSRC_RX_ERROR_CRC;
				}
				if ((status.rx_rx_status & 0x40) != 0) {
					event.status = ITDSRC_RX_ERROR_CARRIER_LOST;
				}
				if ((status.rx_rx_status & 0x20) != 0) {
					event.status = ITDSRC_RX_ERROR_PARITY;
				}
				if ((status.rx_rx_status & 0x10) != 0) {
					event.status = ITDSRC_RX_ERROR_RATE;
				}

				event.handle = 0;
				event.event = ITDSRC_EVENT_RX_FAILED;
				event.packet_id = 0;
				event.packet_size = 0;
				retval = mq_send(bookie_queue, (const char *)&(event), sizeof(struct itdsrc_event), 0);
				if (retval == -1) {
					LOG_ERROR("failed to write in bookie queue: %s", strerror(errno));
				}
				break;
		}
	}

	return NULL;
}

