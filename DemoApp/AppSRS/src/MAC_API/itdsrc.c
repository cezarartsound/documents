#include <stdint.h>  /* uint*_t types */
#include <errno.h>   /* errno variable */
#include <string.h>  /* memcpy function */
#include <mqueue.h>  /* POSIX queues */
#include <stdbool.h> /* bool type */
#include <stdio.h>
#include <stdlib.h>  /* malloc function */

#include "itdsrc_daemon.h"
#include "itdsrc.h"

struct itdsrc_dev {
	int id;
	mqd_t writer_queue;
	mqd_t reader_queue;
	mqd_t bookie_queue;
	char writer_queue_name[100];
	char reader_queue_name[100];
	char bookie_queue_name[100];
};

int  itdsrc_init(int dev_id, struct itdsrc_dev **dev)
{
	bool writer_queue_open, reader_queue_open, bookie_queue_open;
	int retval = 0;

	if (dev == NULL) return EINVAL;
	if ((dev_id > 1) || (dev_id < 0)) return EINVAL; // support only two devices for now

	*dev = malloc(sizeof(struct itdsrc_dev));
	if (*dev == NULL) {
		retval = ENOMEM;
		goto cleanup;
	}

	((*dev)->id) = dev_id;

	// calculate the names of the queues
	strcpy(((*dev)->writer_queue_name), ITDSRC_WRITER_QUEUE_NAME);
	if (dev_id == 0) {
		strcat(((*dev)->writer_queue_name), "_0");
	} else {
		strcat(((*dev)->writer_queue_name), "_1");
	}

	strcpy(((*dev)->reader_queue_name), ITDSRC_READER_QUEUE_NAME);
	if (dev_id == 0) {
		strcat(((*dev)->reader_queue_name), "_0");
	} else {
		strcat(((*dev)->reader_queue_name), "_1");
	}

	strcpy(((*dev)->bookie_queue_name), ITDSRC_BOOKIE_QUEUE_NAME);
	if (dev_id == 0) {
		strcat(((*dev)->bookie_queue_name), "_0");
	} else {
		strcat(((*dev)->bookie_queue_name), "_1");
	}

	// open the queues
	writer_queue_open = false;
	reader_queue_open = false;
	bookie_queue_open = false;

	// open the writer queue
	((*dev)->writer_queue) = mq_open((*dev)->writer_queue_name, O_WRONLY);
	if (((*dev)->writer_queue) == -1) {
		//fprintf(stderr, "failed to open writer message queue: %s\n", strerror(errno));
		retval = errno;
		goto cleanup;
	}
	writer_queue_open = true;

	// open the reader queue
	((*dev)->reader_queue) = mq_open((*dev)->reader_queue_name, O_RDONLY);
	if (((*dev)->reader_queue) == -1) {
		//fprintf(stderr, "failed to open reader message queue: %s\n", strerror(errno));
		//LOG_ERROR("failed to open reader message queue: %s", strerror(errno));
		retval = errno;
		goto cleanup;
	}
	reader_queue_open = true;

	((*dev)->bookie_queue) = mq_open((*dev)->bookie_queue_name, O_RDONLY);
	if (((*dev)->bookie_queue) == -1) {
		//fprintf(stderr, "failed to open bookie message queue: %s\n", strerror(errno));
		//LOG_ERROR("failed to open bookie message queue: %s", strerror(errno));
		retval = errno;
		goto cleanup;
	}
	bookie_queue_open = true;

	return retval;

cleanup:
	if (writer_queue_open) mq_close((*dev)->writer_queue);
	if (reader_queue_open) mq_close((*dev)->reader_queue);
	if (bookie_queue_open) mq_close((*dev)->bookie_queue);

	return retval;
}

void itdsrc_stop(struct itdsrc_dev *dev)
{
	if (dev == NULL) return;

	mq_close(dev->writer_queue);
	mq_close(dev->reader_queue);
	mq_close(dev->bookie_queue);

	free(dev);
}

int  itdsrc_request_tx(struct itdsrc_dev *dev, uint8_t handle, const uint8_t *packet, uint16_t packet_size, uint8_t power, uint8_t bitrate, uint8_t backoff)
{
	int retval = 0;
	struct itdsrc_request request;

	if (dev == NULL) return EINVAL;
	if ((dev->writer_queue) == -1) return EINVAL;
	if (packet_size > ITDSRC_MAX_PACKET_SIZE) return EINVAL;
	if (power > ITDSRC_MAX_POWER) return EINVAL;
	if (bitrate >= ITDSRC_BITRATE_INVALID) return EINVAL;
	if (packet == NULL) return EINVAL;

	request.handle = handle;
	request.op = ITDSRC_OP_TX;
	request.packet_size = packet_size;
	request.power = power;
	request.bitrate = bitrate;
	request.backoff = backoff;
	memcpy(request.packet, packet, packet_size);

	retval = mq_send(dev->writer_queue, (const char *)&(request), sizeof(struct itdsrc_request), 0);
	if (retval == -1) return errno;

	return 0;
}

int  itdsrc_request_tx_discard(struct itdsrc_dev *dev, uint8_t handle, uint8_t packet_id)
{
	int retval = 0;
	struct itdsrc_request request;

	if (dev == NULL) return EINVAL;
	if ((dev->writer_queue) == -1) return EINVAL;

	request.handle = handle;
	request.op = ITDSRC_OP_TX_DISCARD;
	request.packet_id = packet_id;

	retval = mq_send(dev->writer_queue, (const char *)&(request), sizeof(struct itdsrc_request), 0);
	if (retval == -1) return errno;

	return 0;
}

int  itdsrc_request_rx(struct itdsrc_dev *dev, uint8_t handle, uint8_t packet_id, uint16_t packet_size)
{
	int retval = 0;
	struct itdsrc_request request;

	if (dev == NULL) return EINVAL;
	if ((dev->writer_queue) == -1) return EINVAL;
	if (packet_size > ITDSRC_MAX_PACKET_SIZE) return EINVAL;

	request.handle = handle;
	request.op = ITDSRC_OP_RX;
	request.packet_size = packet_size;
	request.packet_id = packet_id;

	retval = mq_send(dev->writer_queue, (const char *)&(request), sizeof(struct itdsrc_request), 0);
	if (retval == -1) return errno;

	return 0;
}

int  itdsrc_request_rx_discard(struct itdsrc_dev *dev, uint8_t handle, uint8_t packet_id)
{
	int retval = 0;
	struct itdsrc_request request;

	if (dev == NULL) return EINVAL;
	if ((dev->writer_queue) == -1) return EINVAL;

	request.handle = handle;
	request.op = ITDSRC_OP_RX_DISCARD;
	request.packet_id = packet_id;

	retval = mq_send(dev->writer_queue, (const char *)&(request), sizeof(struct itdsrc_request), 0);
	if (retval == -1) return errno;

	return 0;
}

int  itdsrc_request_change_channel(struct itdsrc_dev *dev, uint8_t handle, uint8_t channel_id)
{
	int retval = 0;
	struct itdsrc_request request;

	if (dev == NULL) return EINVAL;
	if ((dev->writer_queue) == -1) return EINVAL;

	// check if channel id is valid

	request.handle = handle;
	request.op = ITDSRC_OP_CHANGE_CHANNEL;
	request.channel_id = channel_id;

	retval = mq_send(dev->writer_queue, (const char *)&(request), sizeof(struct itdsrc_request), 0);
	if (retval == -1) return errno;

	return 0;
}

int itdsrc_receive_packet(struct itdsrc_dev *dev, uint8_t *packet, uint16_t packet_size)
{
	int retval = 0;
	uint8_t _packet[ITDSRC_MAX_PACKET_SIZE];

	if (dev == NULL) return EINVAL;
	if ((dev->reader_queue) == -1) return EINVAL;

	retval = mq_receive(dev->reader_queue, (char *)_packet, ITDSRC_MAX_PACKET_SIZE, NULL);
	if (retval == -1) return errno;

	memcpy(packet, _packet, packet_size);

	return 0;
}

int  itdsrc_receive_event(struct itdsrc_dev *dev, uint8_t *event, uint8_t *handle, uint8_t *packet_id, uint16_t *packet_size)
{
	int retval = 0;
	struct itdsrc_event _event;

	if (dev == NULL) return EINVAL;
	if ((dev->bookie_queue) == -1) return EINVAL;
	if (event == NULL) return EINVAL;
	if (handle == NULL) return EINVAL;
	if (packet_id == NULL) return EINVAL;
	if (packet_size == NULL) return EINVAL;

	retval = mq_receive(dev->bookie_queue, (char *)&_event, sizeof(struct itdsrc_event), NULL);
	if (retval == -1) return errno;

	*handle      = _event.handle;
	*event       = _event.event;
	*packet_id   = _event.packet_id;
	*packet_size = _event.packet_size;

	return 0;
}
