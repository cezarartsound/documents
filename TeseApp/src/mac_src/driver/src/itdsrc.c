#include <stdint.h>  /* uint*_t types */
#include <errno.h>   /* errno variable */
#include <string.h>  /* memcpy function */
#include <mqueue.h>  /* POSIX queues */
#include <stdbool.h> /* bool type */
#include <stdio.h>
#include <stdlib.h>  /* malloc function */
#include <time.h>    /* clock_gettime */
#include <pthread.h> /* POSIX threads */
#include <unistd.h>  /* sleep */

#include "itdsrc_daemon.h"
#include "itdsrc.h"

static int  itdsrc_receive_event(struct itdsrc_dev *dev, struct itdsrc_event *event);

struct itdsrc_dev {
   pthread_t event_handler_thread;
   void (*event_handler)(void *priv, const struct itdsrc_event *);
	void *priv;
	int id;
};

void *_itdsrc_event_handler(void *arg)
{
	int retval;
	struct itdsrc_dev *dev;
	struct itdsrc_event itdsrc_event;

	dev = (struct itdsrc_dev *)arg;

   while(0==0) {
      retval = itdsrc_receive_event(dev, &itdsrc_event);

      if (retval != 0) {
         if (retval == ENOENT) { // this means the device was disconnected
				itdsrc_event.event = ITDSRC_EVENT_DISCONNECTED;
				// it is up to the upstream user to stop this API
				// disonnection event will be sent until stoped
				dev->event_handler(dev->priv, &itdsrc_event);
				sleep(1);
			} else if (retval != ETIMEDOUT) { // timeout occurs normally during execution (no event occured) 
				fprintf(stderr, "event_handler: %s\n", strerror(retval));
			}
		} else {
			dev->event_handler(dev->priv, &itdsrc_event);
		}
	}
}

int  itdsrc_init(int dev_id, struct itdsrc_dev **dev, void (*event_handler)(void *priv, const struct itdsrc_event *), void *priv)
{
	int retval;

	if (dev == NULL) return EINVAL;
	if ((dev_id > 1) || (dev_id < 0)) return EINVAL; // support only two devices for now

	*dev = malloc(sizeof(struct itdsrc_dev));
	if (*dev == NULL) {
		return ENOMEM;
	}

	((*dev)->id) = dev_id;
	((*dev)->priv) = priv;
	((*dev)->event_handler) = event_handler;

	/* launch thread that handles the events */
	retval = pthread_create(&((*dev)->event_handler_thread), NULL, _itdsrc_event_handler, (void *)*dev);
	if (retval != 0) {
		//fprintf(stderr, "failed to create event handler thread: %s\n", strerror(retval));
		free(*dev);
		return retval;
	}

	return 0;
}

void itdsrc_stop(struct itdsrc_dev *dev)
{
	int retval;

	if (dev == NULL) return;

	retval = pthread_cancel(dev->event_handler_thread);
	if (retval != 0) {
		fprintf(stderr, "failed to cancel event handler thread: %s\n", strerror(retval));
		return;
	}
	retval = pthread_join(dev->event_handler_thread, NULL);
	if (retval != 0) {
		fprintf(stderr, "failed to join event handler thread\n: %s\n", strerror(retval));
		return;
	}

	free(dev);
}

int  itdsrc_request_tx(struct itdsrc_dev *dev, uint8_t handle, const uint8_t *packet, uint16_t packet_size, uint8_t power, uint8_t bitrate, uint8_t backoff)
{
	int retval;
	struct itdsrc_request request;
	char writer_queue_name[100];
	mqd_t writer_queue;
	bool writer_queue_open;

	if (dev == NULL) return EINVAL;
	if (packet_size > ITDSRC_MAX_PACKET_SIZE) return EINVAL;
	if (power > ITDSRC_MAX_POWER) return EINVAL;
	if (bitrate >= ITDSRC_BITRATE_INVALID) return EINVAL;
	if (packet == NULL) return EINVAL;

	retval = 0;
	writer_queue_open = false;

	// open the writer queue
	sprintf(writer_queue_name, "%s_%d", ITDSRC_WRITER_QUEUE_NAME, dev->id);
	writer_queue = mq_open(writer_queue_name, O_WRONLY);
	if (writer_queue == -1) {
		retval = errno;
		goto cleanup;
	}
	writer_queue_open = true;

	request.handle = handle;
	request.op = ITDSRC_OP_TX;
	request.packet_size = packet_size;
	request.power = power;
	request.bitrate = bitrate;
	request.backoff = backoff;
	memcpy(request.packet, packet, packet_size);

	retval = mq_send(writer_queue, (const char *)&(request), sizeof(struct itdsrc_request), 0);
	if (retval == -1) {
		retval = errno;
		goto cleanup;
	}

	// clean exit here
	retval = 0;

cleanup:
	if (writer_queue_open) mq_close(writer_queue);

	return retval;
}

int  itdsrc_request_tx_discard(struct itdsrc_dev *dev, uint8_t handle, uint8_t packet_id)
{
	int retval;
	struct itdsrc_request request;
	char writer_queue_name[100];
	mqd_t writer_queue;
	bool writer_queue_open;

	if (dev == NULL) return EINVAL;

	retval = 0;
	writer_queue_open = false;

	// open the writer queue
	sprintf(writer_queue_name, "%s_%d", ITDSRC_WRITER_QUEUE_NAME, dev->id);
	writer_queue = mq_open(writer_queue_name, O_WRONLY);
	if (writer_queue == -1) {
		retval = errno;
		goto cleanup;
	}
	writer_queue_open = true;

	request.handle = handle;
	request.op = ITDSRC_OP_TX_DISCARD;
	request.packet_id = packet_id;

	retval = mq_send(writer_queue, (const char *)&(request), sizeof(struct itdsrc_request), 0);
	if (retval == -1) {
		retval = errno;
		goto cleanup;
	}

	// clean exit here
	retval = 0;

cleanup:
	if (writer_queue_open) mq_close(writer_queue);

	return retval;
}

int  itdsrc_request_rx(struct itdsrc_dev *dev, uint8_t handle, uint8_t packet_id, uint16_t packet_size)
{
	int retval;
	struct itdsrc_request request;
	char writer_queue_name[100];
	mqd_t writer_queue;
	bool writer_queue_open;

	if (dev == NULL) return EINVAL;
	if (packet_size > ITDSRC_MAX_PACKET_SIZE) return EINVAL;

	retval = 0;
	writer_queue_open = false;

	// open the writer queue
	sprintf(writer_queue_name, "%s_%d", ITDSRC_WRITER_QUEUE_NAME, dev->id);
	writer_queue = mq_open(writer_queue_name, O_WRONLY);
	if (writer_queue == -1) {
		retval = errno;
		goto cleanup;
	}
	writer_queue_open = true;

	request.handle = handle;
	request.op = ITDSRC_OP_RX;
	request.packet_size = packet_size;
	request.packet_id = packet_id;

	retval = mq_send(writer_queue, (const char *)&(request), sizeof(struct itdsrc_request), 0);
	if (retval == -1) {
		retval = errno;
		goto cleanup;
	}

	// clean exit here
	retval = 0;

cleanup:
	if (writer_queue_open) mq_close(writer_queue);

	return retval;
}

int  itdsrc_request_rx_discard(struct itdsrc_dev *dev, uint8_t handle, uint8_t packet_id)
{
	int retval;
	struct itdsrc_request request;
	char writer_queue_name[100];
	mqd_t writer_queue;
	bool writer_queue_open;

	if (dev == NULL) return EINVAL;

	retval = 0;
	writer_queue_open = false;

	// open the writer queue
	sprintf(writer_queue_name, "%s_%d", ITDSRC_WRITER_QUEUE_NAME, dev->id);
	writer_queue = mq_open(writer_queue_name, O_WRONLY);
	if (writer_queue == -1) {
		retval = errno;
		goto cleanup;
	}
	writer_queue_open = true;

	request.handle = handle;
	request.op = ITDSRC_OP_RX_DISCARD;
	request.packet_id = packet_id;

	retval = mq_send(writer_queue, (const char *)&(request), sizeof(struct itdsrc_request), 0);
	if (retval == -1) {
		retval = errno;
		goto cleanup;
	}

	// clean exit here
	retval = 0;

cleanup:
	if (writer_queue_open) mq_close(writer_queue);

	return retval;
}

int  itdsrc_request_change_channel(struct itdsrc_dev *dev, uint8_t handle, uint8_t channel_id)
{
	int retval;
	struct itdsrc_request request;
	char writer_queue_name[100];
	mqd_t writer_queue;
	bool writer_queue_open;

	if (dev == NULL) return EINVAL;
	// check if channel id is valid

	retval = 0;
	writer_queue_open = false;

	// open the writer queue
	sprintf(writer_queue_name, "%s_%d", ITDSRC_WRITER_QUEUE_NAME, dev->id);
	writer_queue = mq_open(writer_queue_name, O_WRONLY);
	if (writer_queue == -1) {
		retval = errno;
		goto cleanup;
	}
	writer_queue_open = true;

	request.handle = handle;
	request.op = ITDSRC_OP_CHANGE_CHANNEL;
	request.channel_id = channel_id;

	retval = mq_send(writer_queue, (const char *)&(request), sizeof(struct itdsrc_request), 0);
	if (retval == -1) {
		retval = errno;
		goto cleanup;
	}

	// clean exit here
	retval = 0;

cleanup:
	if (writer_queue_open) mq_close(writer_queue);

	return retval;
}

int itdsrc_receive_packet(struct itdsrc_dev *dev, uint8_t *packet, uint16_t packet_size)
{
	int retval;
	uint8_t _packet[ITDSRC_MAX_PACKET_SIZE];
	char reader_queue_name[100];
	mqd_t reader_queue;
	bool reader_queue_open;

	if (dev == NULL) return EINVAL;

	retval = 0;
	reader_queue_open = false;

	// open the reader queue
	sprintf(reader_queue_name, "%s_%d", ITDSRC_READER_QUEUE_NAME, dev->id);
	reader_queue = mq_open(reader_queue_name, O_RDONLY);
	if (reader_queue == -1) {
		retval = errno;
		goto cleanup;
	}
	reader_queue_open = true;

	retval = mq_receive(reader_queue, (char *)_packet, ITDSRC_MAX_PACKET_SIZE, NULL);
	if (retval == -1) {
		retval = errno;
		goto cleanup;
	}

	memcpy(packet, _packet, packet_size);

	// clean exit here
	retval = 0;

cleanup:
	if (reader_queue_open) mq_close(reader_queue);

	return retval;
}

int  itdsrc_receive_event(struct itdsrc_dev *dev, struct itdsrc_event *event)
{
	int retval;
	char bookie_queue_name[100];
	mqd_t bookie_queue;
	struct timespec t;
	bool bookie_queue_open;

	if (dev == NULL) return EINVAL;
	if (event == NULL) return EINVAL;

	retval = 0;
	bookie_queue_open = false;

	// open the bookie queue
	sprintf(bookie_queue_name, "%s_%d", ITDSRC_BOOKIE_QUEUE_NAME, dev->id);
	bookie_queue = mq_open(bookie_queue_name, O_RDONLY);
	if (bookie_queue == -1) {
		retval = errno;
		fprintf(stderr, "bookie queue failed to open\n");
		goto cleanup;
	}
	bookie_queue_open = true;

	retval = clock_gettime(CLOCK_REALTIME, &t);
	if (retval == -1) {
		retval = errno;
		goto cleanup;
	}

	t.tv_sec++; // wait for, at most, one second
	retval = mq_timedreceive(bookie_queue, (char *)event, sizeof(struct itdsrc_event), NULL, &t);
	if (retval == -1) {
		retval = errno;
		goto cleanup;
	}

	// clean exit here
	retval = 0;

cleanup:
	if (bookie_queue_open) mq_close(bookie_queue);

	return retval;
}
