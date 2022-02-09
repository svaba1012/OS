#ifndef DISK_STREAMER_H
#define DISK_STREAMER_H

/*basic functionality for reading and writing 
any amount of bytes from any loacation to and from the disk*/

#include "disk.h"

//structure defines which disk is read to or written from
//also it holds current position in the disk from where reading or writing start  
struct disk_streamer{
    struct disk* disk;
    uint64_t pos;
};

int32_t write_bytes_to_disk(struct disk_streamer* stream, void* in, int32_t total);
int32_t read_bytes_from_disk(struct disk_streamer* stream, void* out, int32_t total);
void set_disk_pos(struct disk_streamer* stream, uint32_t pos);
struct disk_streamer* new_disk_stream(uint32_t disk_id);
void delete_streamer(struct disk_streamer* stream);



#endif

