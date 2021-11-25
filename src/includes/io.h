#ifndef IO_H
#define IO_H
#include <inttypes.h>

uint8_t in_byte(uint16_t port);
uint16_t in_word(uint16_t port);
void out_byte(uint16_t port, uint8_t byte);
void out_word(uint16_t port, uint16_t word);


#endif