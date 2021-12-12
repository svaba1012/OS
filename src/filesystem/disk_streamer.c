#include "disk.h"
#include "disk_streamer.h"
#include "kheap.h"
#include "config.h"
#include "terminal.h"

struct disk_streamer* new_disk_stream(uint32_t disk_id){
    struct disk* selected_disk = get_disk_by_index(disk_id);
    if(disk_id != 0){
        return NULL;
    }
    struct disk_streamer* new_stream = (struct disk_streamer*)kzalloc(sizeof(struct disk_streamer));
    new_stream -> disk = selected_disk;
    new_stream -> pos = 0;
    return new_stream;
}

void set_disk_pos(struct disk_streamer* stream, uint32_t pos){
    stream -> pos = pos;
}


int32_t read_bytes_from_disk(struct disk_streamer* stream, void* out, int32_t total){
    uint32_t sector = stream -> pos / MY_OS_DISK_SECTOR_SIZE;
    uint32_t offset = stream ->pos % MY_OS_DISK_SECTOR_SIZE;
    char buf[MY_OS_DISK_SECTOR_SIZE];
    char *start = buf + offset;
    int32_t res = read_disk_block(stream->disk, sector, 1, buf);
    if(res < 0){
        return res;
    }  
    int32_t total_to_read = total;
    if(total_to_read > MY_OS_DISK_SECTOR_SIZE){
        total_to_read = MY_OS_DISK_SECTOR_SIZE;
    }
    for(int32_t i = 0; i < total_to_read; i++){
        *(char*)out++ = start[i];
    }
    total -= total_to_read;
    stream ->pos += total_to_read;
    if(total > 0){
        read_bytes_from_disk(stream, out, total);
    }
    return 0;

}

void delete_streamer(struct disk_streamer* stream){
    kfree(stream);
}