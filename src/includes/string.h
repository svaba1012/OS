#ifndef STRING_H
#define STRING_H

//string functions such as those in standard c library

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

bool is_numeric(char c);
uint32_t char_to_numeric(char c);
uint32_t strnlen(char* str, uint32_t max_len);
uint32_t strlen(char* str);
char* strcpy(char* dest, char* src);
char* strncpy(char* dest, char* src, size_t len);
int32_t strncmp(char* str1, char* str2, int32_t n);
int32_t strcmp(char* str1, char* str2);
int32_t strncmp_terminating_char(char* str1, char* str2, int32_t n, char c);
char* strchr(char* str, char c);
char* strnchr(char* str, char c, int32_t n);


#endif
