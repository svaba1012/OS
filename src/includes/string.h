#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

bool is_numeric(char c);
uint32_t char_to_numeric(char c);
uint32_t strnlen(char* str, uint32_t max_len);
uint32_t strlen(char* str);

#endif
