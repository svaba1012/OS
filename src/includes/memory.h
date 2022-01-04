#ifndef MEMORY_H
#define MEMORY_H
#include <stddef.h>
#include <stdint.h>

void* memset(void* ptr, int c, size_t len);
int32_t memcmp(void* m1, void* m2, uint32_t maxlen);
int32_t memcpy(void* dest, void* src, uint32_t maxlen);
int32_t switch_endians(int32_t num);


#endif