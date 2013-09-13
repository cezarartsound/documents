#include "itdsrc_mac.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>    /* POSIX threads */
#include <errno.h>      /* errno variable and error constants */
#include <stdbool.h>    /* bool types */
#include <stdlib.h>     /* malloc and free */
#include <assert.h>     /* assert function */

#include "itdsrc.h"

struct ma_dev {
	struct itdsrc_dev *itdsrc_dev;
	uint8_t rx_handle;
	uint16_t rx_packet_size[256];
	bool tip;   // transmission in progress
	bool lastx; // was last transmission an x type or not?
	void (*indication_cb)(const struct ma_unitdata_indication *);
	void (*status_indication_cb)(const struct ma_unitdata_status_indication *);
	void (*xstatus_indication_cb)(const struct ma_unitdatax_status_indication *);
};

static void event_handler(void *priv, const struct itdsrc_event *event);

int ma_init(
	int dev_id,
	struct ma_dev **dev,
	void (*ma_unitdata_indication_cb)(const struct ma_unitdata_indication *),
	void (*ma_unitdata_status_indication_cb)(const struct ma_unitdata_status_indication *),
	void (*ma_unitdatax_status_indication_cb)(const struct ma_unitdatax_status_indication *)
)
{
	int retval;
	bool itdsrc_inited;

	assert(ma_unitdata_indication_cb != NULL);
	assert(ma_unitdata_status_indication_cb != NULL);
	assert(ma_unitdatax_status_indication_cb != NULL);

	itdsrc_inited = false;

	(*dev) = malloc(sizeof(struct ma_dev));
	if (*dev == NULL) {
		return ENOMEM;
	}

	retval = 0;

	((*dev)->tip) = false;
	((*dev)->rx_handle) = 0;

	((*dev)->indication_cb) = ma_unitdata_indication_cb;
	((*dev)->status_indication_cb) = ma_unitdata_status_indication_cb;
	((*dev)->xstatus_indication_cb) = ma_unitdatax_status_indication_cb;
	
	retval = itdsrc_init(dev_id, &((*dev)->itdsrc_dev), event_handler, (void *)(*dev));
	if (retval != 0) {
		//fprintf(stderr, "failed to initialize device: %s\n", strerror(retval));
		goto cleanup;
	}
	itdsrc_inited = true;

	//printf("Initialization done.\n");

	return 0;

cleanup:
	if (itdsrc_inited) itdsrc_stop((*dev)->itdsrc_dev);
	if ((*dev) != NULL) free(*dev);
	return retval;
}

void ma_stop(struct ma_dev *dev)
{
	assert(dev != NULL);

	itdsrc_stop(dev->itdsrc_dev);
	free(dev);
	//printf("Stop done.\n");
}

int ma_unitdata_request(
	struct ma_dev *dev,
	const uint8_t source_address[6],
	const uint8_t destination_address[6],
	const uint8_t *data,
	int data_length,
	uint8_t priority,
	uint8_t service_class
)
{
	int retval;
	uint8_t packet[data_length+12]; // add place for the mac addresses

	assert(dev != NULL);
	assert(source_address != NULL);
	assert(destination_address != NULL);
	assert(data != NULL);

	if (dev->tip) {
		fprintf(stderr, "Transmission in progress.\n");
		return -1;
	}

	retval = 0;

	(dev->tip) = true;
	(dev->lastx) = false;

	memcpy(packet, destination_address, 6);
	memcpy(packet+6, source_address, 6);
	memcpy(packet+12, data, data_length);

	retval = itdsrc_request_tx(dev->itdsrc_dev, 0, packet, data_length+12, 0, 3, 0);

	//printf("Request complete.\n");

	return retval;
}

int ma_unitdatax_request(
	struct ma_dev *dev,
	const uint8_t source_address[6],
	const uint8_t destination_address[6],
	const uint8_t *data,
	int data_length,
	uint8_t priority,
	uint8_t service_class,
	uint8_t channel_id,
	uint8_t data_rate,
	uint8_t power_level,
	uint8_t expiry_time
)
{
	int retval;
	uint8_t packet[data_length+12]; // add place for the mac addresses

	assert(dev != NULL);
	assert(source_address != NULL);
	assert(destination_address != NULL);
	assert(data != NULL);

	if (dev->tip) {
		fprintf(stderr, "Transmission in progress.\n");
		return -1;
	}

	retval = 0;

	(dev->tip) = true;
	(dev->lastx) = true;

	memcpy(packet, destination_address, 6);
	memcpy(packet+6, source_address, 6);
	memcpy(packet+12, data, data_length);

	retval = itdsrc_request_tx(dev->itdsrc_dev, 1, packet, data_length+12, power_level, data_rate, 0);

	//printf("Request complete.\n");

	return retval;
}

void event_handler(void *priv, const struct itdsrc_event *event)
{
	int retval;
	uint8_t rx_packet[ITDSRC_MAX_PACKET_SIZE];
	struct ma_unitdata_indication indication;
	struct ma_unitdata_status_indication status_indication;
	struct ma_unitdatax_status_indication xstatus_indication;
	struct ma_dev *dev;

	assert(priv != NULL);
	assert(event != NULL);

	dev = (struct ma_dev *)priv;

	switch (event->event) {
		case ITDSRC_EVENT_DISCONNECTED:
			indication.reception_status = 32;
			(dev->indication_cb)(&indication);
			break;

		case ITDSRC_EVENT_CMD_RECEIVED:
			break;

		case ITDSRC_EVENT_TX_DISCARDED:
			break;

		case ITDSRC_EVENT_TX_REQUESTED:
			break;

		case ITDSRC_EVENT_TX_FINISHED :
			// delete the packet from the device memory
			retval = itdsrc_request_tx_discard(dev->itdsrc_dev, 0, event->packet_id);
			if (retval != 0) {
				fprintf(stderr, "failed to send tx discard request: %s\n", strerror(errno));
			}

			// issue the callback
			if (dev->lastx) {
				xstatus_indication.transmission_status = 0;
				(dev->xstatus_indication_cb)(&xstatus_indication);
			} else {
				status_indication.transmission_status = 0;
				(dev->status_indication_cb)(&status_indication);
			}

			(dev->tip) = false;
			break;

		case ITDSRC_EVENT_TX_FAILED   :
			// issue the callback
			if (dev->lastx) {
				xstatus_indication.transmission_status = 1;
				dev->xstatus_indication_cb(&xstatus_indication);
			} else {
				status_indication.transmission_status = 1;
				dev->status_indication_cb(&status_indication);
			}

			(dev->tip) = false;
			break;

		case ITDSRC_EVENT_RX_DISCARDED:
			break;

		case ITDSRC_EVENT_RX_REQUESTED:
			// read the packet from the queue
			retval = itdsrc_receive_packet(dev->itdsrc_dev, rx_packet, (dev->rx_packet_size)[event->handle]);
			if (retval != 0) {
				fprintf(stderr, "failed to receive packet: %s\n", strerror(retval));
			}

			// discard the packet
			retval = itdsrc_request_rx_discard(dev->itdsrc_dev, event->handle, event->packet_id);
			if (retval != 0) {
				fprintf(stderr, "failed to send rx discard request: %s\n", strerror(retval));
			}

			// issue the callback
			memcpy(indication.destination_address, rx_packet, 6);
			memcpy(indication.source_address, rx_packet+6, 6);
			indication.data = rx_packet+12;
			indication.data_length = (dev->rx_packet_size)[event->handle]-12;
			indication.reception_status = 0;
			(dev->indication_cb)(&indication);
			break;

		case ITDSRC_EVENT_RX_FAILED   :
			// issue the callback signaling the error
			indication.reception_status = 1;
			(dev->indication_cb)(&indication);
			break;

		case ITDSRC_EVENT_RX_DETECTED :
			// save the size for use on reception
			(dev->rx_packet_size)[dev->rx_handle] = event->packet_size;

			// request that the packet is read
			retval = itdsrc_request_rx(dev->itdsrc_dev, dev->rx_handle, event->packet_id, event->packet_size);
			if (retval != 0) {
				fprintf(stderr, "failed to send rx request: %s\n", strerror(retval));
			}

			// increase handle for next packet
			// assumes that there will not be a delay of 256 packets received
			// until being read from memory
			dev->rx_handle = ((dev->rx_handle+1)%256);
			break;

		default:
			fprintf(stderr, "unrecognized event occurred %d\n", event->event);
			break;
	}
}
