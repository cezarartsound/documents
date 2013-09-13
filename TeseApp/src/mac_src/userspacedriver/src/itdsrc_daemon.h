#ifndef ITDSRC_DAEMON_H
#define ITDSRC_DAEMON_H

#include <stdint.h> /* uint*_t types */

#include "itdsrc.h"

//#define ITDSRC_MAX_PACKET_SIZE 2000

#define ITDSRC_WRITER_QUEUE_NAME "/itdsrc_writer"
#define ITDSRC_READER_QUEUE_NAME "/itdsrc_reader"
#define ITDSRC_BOOKIE_QUEUE_NAME "/itdsrc_bookie"

//#define ITDSRC_EVENT_CMD_RECEIVED 0
//#define ITDSRC_EVENT_TX_DISCARDED 1
//#define ITDSRC_EVENT_TX_REQUESTED 2
//#define ITDSRC_EVENT_TX_FINISHED  3
//#define ITDSRC_EVENT_TX_FAILED    4
//#define ITDSRC_EVENT_RX_DISCARDED 5
//#define ITDSRC_EVENT_RX_REQUESTED 6
//#define ITDSRC_EVENT_RX_FAILED    7
//#define ITDSRC_EVENT_RX_DETECTED  8
//#define ITDSRC_EVENT_INVALID      9

#define ITDSRC_OP_TX             0
#define ITDSRC_OP_RX             1
#define ITDSRC_OP_TX_DISCARD     2
#define ITDSRC_OP_RX_DISCARD     3
#define ITDSRC_OP_CHANGE_CHANNEL 4
#define ITDSRC_OP_INVALID        5

//#define ITDSRC_BITRATE_64QAM_2_3 0
//#define ITDSRC_BITRATE_16QAM_1_2 1
//#define ITDSRC_BITRATE_QPSK_1_2  2
//#define ITDSRC_BITRATE_BPSK_1_2  3
//#define ITDSRC_BITRATE_64QAM_3_4 4
//#define ITDSRC_BITRATE_16QAM_3_4 5
//#define ITDSRC_BITRATE_QPSK_3_4  6
//#define ITDSRC_BITRATE_BPSK_3_4  7
//#define ITDSRC_BITRATE_INVALID   8

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

#endif /* ITDSRC_DAEMON_H */
