#ifndef HOSTLINK_PROTOCOL_H
#define HOSTLINK_PROTOCOL_H

#include <stdint.h>
#include <stdlib.h>

int stuff_packet(uint8_t *head_buffer, size_t head_buffer_size, uint8_t *body_buffer, size_t body_buffer_size, uint8_t *to_buffer, size_t to_buffer_size, size_t *final_size);

#endif /* HOSTLINK_PROTOCOL_H */
