#ifndef HEAP_H
#define HEAP_H
#include <stddef.h>
#include <stdint.h>

typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

struct heap_table{
    HEAP_BLOCK_TABLE_ENTRY* entries; //pointer to array of heap table entries
    size_t total; //size of table in bytes
};

struct  heap{
    struct heap_table* table;
    void* start_adress; //start adress
};

int heap_create(struct heap* heap, void* start, void* end, struct heap_table* table);
void* heap_malloc(struct heap* heap, size_t bytes);
void heap_free(struct heap* heap, void* ptr);

#endif