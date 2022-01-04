#include "disk.h"
#include "io.h"
#include "status.h"
#include "memory.h"

struct disk disk;

//writing to disk


int32_t read_sectors_from_disk(uint32_t lba, uint32_t num_of_sec, void* buf){
    uint8_t is_ready_for_read;
    
    out_byte(0x1F6, (lba >> 24) | 0xE0); //sending highest bits to lba, selecting master drive
    out_byte(0x1F2, num_of_sec & 0xFF); //sending how many sectors are going to be read
    out_byte(0x1F3, lba & 0xFF); //sending lower bits of lba
    out_byte(0x1F4, (lba >> 8) & 0xFF); //sending bits 8-15 to lba
    out_byte(0x1F5, (lba >> 16) & 0xFF); //sending bits 16-23 to lba
    
    out_byte(0x1F7, 0x20); //prepering to read

    uint16_t* ptr = (uint16_t*) buf;

    for(int i = 0; i < num_of_sec; i++){
        is_ready_for_read = in_byte(0x1F7);
        while(!(is_ready_for_read & 0x08)){
            is_ready_for_read = in_byte(0x1F7);
        }
        for(int j = 0; j < 256; j++){
            *ptr = in_word(0x1F0);
            ptr++;
        }
    }
    return 0;
}

void disk_init(){
    memset(&disk, 0x00, sizeof(struct disk));
    disk.type = MY_OS_DISK_TYPE_REAL;
    disk.sectors_size = MY_OS_DISK_SECTOR_SIZE;
    disk.id = 0;
    disk.filesystem = fs_resolve(&disk);
}

struct disk* get_disk_by_index(uint32_t i){
    if(i != 0){
        return NULL;
    }
    return &disk;
}

uint32_t read_disk_block(struct disk* disk_addr, uint32_t lba, uint32_t num_of_sec, void* buf){
    if(disk_addr != &disk){
        return -EIO;
    }
    return read_sectors_from_disk(lba, num_of_sec, buf);
}