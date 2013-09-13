#ifndef ITDSRC_DAEMON_H
#define ITDSRC_DAEMON_H

#include <stdint.h> /* uint*_t types */

#include "itdsrc.h"

#define ITDSRC_WRITER_QUEUE_NAME "/itdsrc_writer"
#define ITDSRC_READER_QUEUE_NAME "/itdsrc_reader"
#define ITDSRC_BOOKIE_QUEUE_NAME "/itdsrc_bookie"

#define ITDSRC_OP_TX             0
#define ITDSRC_OP_RX             1
#define ITDSRC_OP_TX_DISCARD     2
#define ITDSRC_OP_RX_DISCARD     3
#define ITDSRC_OP_CHANGE_CHANNEL 4
#define ITDSRC_OP_INVALID        5

#define ITDSRC_MAX_POWER 63

struct itdsrc_request {
	uint8_t  handle;
	uint8_t  op;
	uint8_t  packet_id;
	uint16_t packet_size;
	uint8_t  power;
	uint8_t  bitrate;
	uint8_t  backoff;
	uint8_t  channel_id;
	uint8_t  packet[ITDSRC_MAX_PACKET_SIZE];
};

struct itdsrc_event {
	uint8_t  handle;
	uint8_t  event;
	uint8_t  packet_id;
	uint16_t packet_size;
};

#endif /* ITDSRC_DAEMON_H */
