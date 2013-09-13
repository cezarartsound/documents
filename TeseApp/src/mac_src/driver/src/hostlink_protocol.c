#define SPECIAL_MARK 0xFF
#define SOF_MARK 0x00
#define EOF_MARK 0xF0
#define STUFF_BYTE 0xFF

#include <stdint.h>
#include <stdlib.h>

int stuff_packet(uint8_t *head_buffer, size_t head_buffer_size, uint8_t *body_buffer, size_t body_buffer_size, uint8_t *to_buffer, size_t to_buffer_size, size_t *final_size)
{
	size_t from_index;
	size_t to_index;
	
	if (to_buffer_size < 4) return -1;

	to_index = 0;

	/* insert sof */
	to_buffer[to_index] = SPECIAL_MARK;
	to_index++;
	to_buffer[to_index] = SOF_MARK;
	to_index++;

	/* parse header */
	for (from_index = 0; from_index < head_buffer_size; ++from_index) {
		to_buffer[to_index] = head_buffer[from_index];
		to_index++;

		/* insert stuff byte when needed */
		if (head_buffer[from_index] == SPECIAL_MARK) {
			to_buffer[to_index] = STUFF_BYTE;
			to_index++;
		}

		if (to_index > to_buffer_size-2) return -1;
	}

	/* parse body */
	for (from_index = 0; from_index < body_buffer_size; ++from_index) {
		to_buffer[to_index] = body_buffer[from_index];
		to_index++;

		/* insert stuff byte when needed */
		if (body_buffer[from_index] == SPECIAL_MARK) {
			to_buffer[to_index] = STUFF_BYTE;
			to_index++;
		}
		if (to_index > to_buffer_size-2) return -1;
	}

	/* insert eof */
	to_buffer[to_index] = SPECIAL_MARK;
	to_index++;
	to_buffer[to_index] = EOF_MARK;
	to_index++;
	
	*final_size = to_index;

	return 0;
}
