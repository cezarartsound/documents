#ifndef ITDSRC_H
#define ITDSRC_H

#include <stdint.h>        /* uint8_t type */

#define ITDSRC_MAX_PACKET_SIZE 2360
 
#define ITDSRC_EVENT_CMD_RECEIVED 0
#define ITDSRC_EVENT_TX_DISCARDED 1
#define ITDSRC_EVENT_TX_REQUESTED 2
#define ITDSRC_EVENT_TX_FINISHED  3
#define ITDSRC_EVENT_TX_FAILED    4
#define ITDSRC_EVENT_RX_DISCARDED 5
#define ITDSRC_EVENT_RX_REQUESTED 6
#define ITDSRC_EVENT_RX_FAILED    7
#define ITDSRC_EVENT_RX_DETECTED  8
#define ITDSRC_EVENT_DISCONNECTED 9
#define ITDSRC_EVENT_INVALID      10

#define ITDSRC_BITRATE_64QAM_2_3 0
#define ITDSRC_BITRATE_16QAM_1_2 1
#define ITDSRC_BITRATE_QPSK_1_2  2
#define ITDSRC_BITRATE_BPSK_1_2  3
#define ITDSRC_BITRATE_64QAM_3_4 4
#define ITDSRC_BITRATE_16QAM_3_4 5
#define ITDSRC_BITRATE_QPSK_3_4  6
#define ITDSRC_BITRATE_BPSK_3_4  7
#define ITDSRC_BITRATE_INVALID   8

#define ITDSRC_RX_ERROR_CRC          1
#define ITDSRC_RX_ERROR_CARRIER_LOST 2
#define ITDSRC_RX_ERROR_PARITY       3
#define ITDSRC_RX_ERROR_RATE         4

struct itdsrc_dev;

struct itdsrc_event {
	uint8_t  event;
	uint8_t  status;
	uint8_t  handle;
	uint8_t  packet_id;
	uint16_t packet_size;
};

int  itdsrc_init(int dev_id, struct itdsrc_dev **dev, void (*event_handler)(void *priv, const struct itdsrc_event *), void *priv);
void itdsrc_stop(struct itdsrc_dev *dev);
int  itdsrc_request_tx(struct itdsrc_dev *dev, uint8_t handle, const uint8_t *packet, uint16_t packet_size, uint8_t power, uint8_t bitrate, uint8_t backoff);
int  itdsrc_request_tx_discard(struct itdsrc_dev *dev, uint8_t handle, uint8_t packet_id);
int  itdsrc_request_rx(struct itdsrc_dev *dev, uint8_t handle, uint8_t packet_id, uint16_t packet_size);
int  itdsrc_request_rx_discard(struct itdsrc_dev *dev, uint8_t handle, uint8_t packet_id);
int  itdsrc_request_change_channel(struct itdsrc_dev *dev, uint8_t handle, uint8_t channel_id);
int  itdsrc_receive_packet(struct itdsrc_dev *dev, uint8_t *packet, uint16_t packet_size);

#endif /* ITDSRC_H */
