#include "itdsrc_daemon.h"
#include "itdsrc_protocol.h"

#include <stdio.h>

void parse_status(uint8_t *buffer, int buffer_size, struct itdsrc_status *status)
{
	if (buffer_size != ITDSRC_STATUS_SIZE) {
		fprintf(stderr, "status message must be %d bytes, but %d where announced", ITDSRC_STATUS_SIZE, buffer_size);
	}

	status->rx_rx_packet_id = buffer[0];
	status->rx_rx_packet_size = (uint16_t)(buffer[1]);
	status->rx_rx_packet_size |= (uint16_t)(buffer[2]) << 8;
	status->rx_rx_status = buffer[3];
	status->rx_op_handle = buffer[4];
	status->rx_op_status = buffer[5];
	status->tx_handle = buffer[6];
	status->tx_packet_id = buffer[7];
	status->tx_status = buffer[8];
	status->fp_handle = buffer[9];
	status->fp_status = buffer[10];
}

void print_status(struct itdsrc_status *status)
{
	printf("STATUS MESSAGE\n");
	printf("\tRX_RX_PACKET_ID   : %04d\n", status->rx_rx_packet_id);
	printf("\tRX_RX_PACKET_SIZE : %04d\n", status->rx_rx_packet_size);
	printf("\tRX_RX_STATUS      : %04d\n", status->rx_rx_status);
	printf("\tRX_OP_HANDLE      : %04d\n", status->rx_op_handle);
	printf("\tRX_OP_STATUS      : %04d\n", status->rx_op_status);
	printf("\tTX_HANDLE         : %04d\n", status->tx_handle);
	printf("\tTX_PACKET_ID      : %04d\n", status->tx_packet_id);
	printf("\tTX_STATUS         : %04d\n", status->tx_status);
	printf("\tFP_HANDLE         : %04d\n", status->fp_handle);
	printf("\tFP_STATUS         : %04d\n", status->fp_status);
	printf("END OF STATUS MESSAGE\n");
}

/* Converts the command to a linear buffer that is ready to be sent */
int itdsrc_cmd_to_header(struct itdsrc_cmd cmd, uint8_t *header, size_t *header_size)
{
	int status;
	
	status = 0;
	
	header[0] = cmd.handle;
	header[1] = cmd.op;
	switch (cmd.op) {
		case ITDSRC_OP_TX:
			header[2] = (uint8_t)(cmd.packet_size & 0x00FF);
			header[3] = (uint8_t)((cmd.packet_size >> 8) & 0x00FF);
			header[4] = cmd.power;
			header[5] = cmd.bitrate;
			header[6] = cmd.backoff; // backoff
			*header_size = 7;
			break;
		
		case ITDSRC_OP_RX:
			header[2] = cmd.packet_id;
			header[3] = (uint8_t)(cmd.packet_size & 0x00FF);
			header[4] = (uint8_t)((cmd.packet_size >> 8) & 0x00FF);
			*header_size = 5;
			break;
		
		case ITDSRC_OP_TX_DISCARD:
			header[2] = cmd.packet_id;
			*header_size = 3;
			break;

		case ITDSRC_OP_RX_DISCARD:
			header[2] = cmd.packet_id;
			*header_size = 3;
			break;
		
		default:
			status = -1;
	}
	
	return status;
}

bool itdsrc_channel_id_is_valid(uint8_t channel_id)
{
	bool retval;

	switch (channel_id) {
		case 172:
		case 174:
		case 176:
		case 178:
		case 180:
		case 182:
		case 184:
			retval = true;
			break;

		default:
			retval = false;
			break;
	}

	return retval;
}
