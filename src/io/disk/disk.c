#include "disk.h"
#include "io.h"
#include "status.h"
#include "memory.h"


struct disk disk;


//assembly function for writing from buf addres to lba starting index num_of_sec sectors 
extern void ata_lba_write(uint32_t lba, uint32_t num_of_sec, void* buf);
//assembly function for reading to buf addres from lba starting index num_of_sec sectors  
extern int32_t ata_lba_read(uint32_t lba, uint32_t num_of_sec, void* buf);


void disk_init(){
    memset(&disk, 0x00, sizeof(struct disk));
    disk.type = MY_OS_DISK_TYPE_REAL;
    disk.sectors_size = MY_OS_DISK_SECTOR_SIZE;
    disk.id = 0;
    disk.filesystem = fs_resolve(&disk);    //check and recognise if there is any known filesystem on the disk
}

struct disk* get_disk_by_index(uint32_t i){
    //in current implementation os only recognises primary disk with index 0
    if(i != 0){
        return NULL;
    }
    return &disk;
}

uint32_t read_disk_block(struct disk* disk_addr, uint32_t lba, uint32_t num_of_sec, void* buf){
    //read num_of_sec sectors starting with index lba to adress buf
    if(disk_addr != &disk){
        return -EIO;
    }
    if(ata_lba_read(lba, num_of_sec, buf) != 0){
        read_disk_block(disk_addr,lba, num_of_sec, buf);
    }
    return 0; 
}

uint32_t write_disk_block(struct disk* disk_addr, uint32_t lba, uint32_t num_of_sec, void* buf){
    //write to num_of_sec sectors starting with index lba from adress buf
    if(disk_addr != &disk){
        return -EIO;
    }
    ata_lba_write(lba, num_of_sec, buf);
    return 0;
}