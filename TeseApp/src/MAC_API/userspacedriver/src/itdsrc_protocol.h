#ifndef ITDSRC_PROTOCOL_H
#define ITDSRC_PROTOCOL_H

#include <stdint.h>  /*uint*_t types */
#include <stdbool.h> /* bool type */
#include <stddef.h>  /* size_t type */
//#include <stdlib.h>

#define ITDSRC_STATUS_SIZE 11

struct itdsrc_cmd {
	uint8_t handle;
	uint8_t op;
	uint8_t packet_id;
	uint16_t packet_size;
	uint8_t power;
	uint8_t bitrate;
	uint8_t backoff;
	uint8_t channel_id;
};

struct itdsrc_status {
	uint8_t fp_handle;
	uint8_t fp_status;

	uint8_t tx_handle;
	uint8_t tx_packet_id;
	uint8_t tx_status;

	uint8_t rx_op_handle;
	uint8_t rx_op_status;
	uint8_t rx_rx_packet_id;
	uint16_t rx_rx_packet_size;
	uint8_t rx_rx_status;
};

void parse_status(uint8_t *buffer, int buffer_size, struct itdsrc_status *status);
void print_status(struct itdsrc_status *status);
int itdsrc_cmd_to_header(struct itdsrc_cmd cmd, uint8_t *header, size_t *header_size);
bool itdsrc_channel_id_is_valid(uint8_t channel_id);

#endif /* ITDSRC_PROTOCOL_H */
