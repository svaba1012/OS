#ifndef DISK_H
#define DISK_H

#include <stdint.h>
#include <stddef.h>

#define MY_OS_DISK_SECTOR_SIZE 512

typedef uint32_t MY_OS_DISK_TYPE;
enum DISK_TYPES{MY_OS_DISK_TYPE_REAL};

struct disk{
    MY_OS_DISK_TYPE type;
    uint32_t sectors_size;
};

uint32_t read_disk_block(struct disk* disk_addr, uint32_t lba, uint32_t num_of_sec, void* buf);
struct disk* get_disk_by_index(uint32_t i);
void disk_init();


#endif