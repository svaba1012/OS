#ifndef KHEAP_H
#define HHEAP_H

//functins that operate on kernel heap

void kheap_int();
void* kmalloc(size_t bytes);
void kfree(void* ptr);
void* kzalloc(size_t bytes);

#endif 