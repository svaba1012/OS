#include "disk.h"
#include "disk_streamer.h"
#include "kheap.h"
#include "config.h"
#include "terminal.h"
#include "status.h"
#include "memory.h"

//disk_streamer is structure which provides user friendly access to disk
//to read/write any amount of bytes from/to disk, not whole sectors 

struct disk_streamer* new_disk_stream(uint32_t disk_id){ 
    struct disk* selected_disk = get_disk_by_index(disk_id);
    if(disk_id != 0){
        return NULL;
    }
    struct disk_streamer* new_stream = (struct disk_streamer*)kzalloc(sizeof(struct disk_streamer));
    if(new_stream == NULL){
        return NULL;
    }
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
    char buf[MY_OS_DISK_SECTOR_SIZE];   //temporal storage for read data
    char *start = buf + offset; //pointer to starting bytes of data to be read
    int32_t res = read_disk_block(stream->disk, sector, 1, buf); //load whole sector to addres buf
    if(res < 0){
        return res;
    }  
    int32_t total_to_read = total;
    if(total_to_read > MY_OS_DISK_SECTOR_SIZE - offset){  // must stay in boundary of buf array
        total_to_read = MY_OS_DISK_SECTOR_SIZE - offset;     
    }
    for(int32_t i = 0; i < total_to_read; i++){
        *(char*)out++ = start[i]; //copy from buf + offset (start) to out
    }
    total -= total_to_read;     
    stream ->pos += total_to_read;  //change pos on disk
    if(total > 0){  //recursivly call read_bytes_from disk untill there's nothing more to read
        read_bytes_from_disk(stream, out, total);
    }
    return 0;
}

int32_t write_bytes_to_disk(struct disk_streamer* stream, void* in, int32_t total){
    int32_t res = 0;
    uint32_t sector_index = stream->pos / MY_OS_DISK_SECTOR_SIZE;
    uint32_t sector_offset = stream->pos % MY_OS_DISK_SECTOR_SIZE;
    char buf[MY_OS_DISK_SECTOR_SIZE];
    char* start = buf + sector_offset;
    int32_t total_to_write = total;
    //till here all is same as for reading
    if (total_to_write > stream->disk->sectors_size - sector_offset){ //stay in buf array boundary
        total_to_write = stream->disk->sectors_size - sector_offset;
    }
    if (sector_offset == 0  && total_to_write == stream->disk->sectors_size){
     //if there's writing to whole sector just write
        res = write_disk_block(stream->disk, sector_index, 1, in);
        if (res != 0){
            res = -EIO;
            goto out;
        }
    }else{//read whole sector to buf than write from out to buf
    //(change buf content only after offset and total to write bytes only), and than write whole sector back
        res = read_disk_block(stream->disk, sector_index, 1, buf);
        if (res != 0){
            res = -EIO;
            goto out;
        }
        memcpy(start, in, total_to_write);
        res = write_disk_block(stream->disk, sector_index, 1, buf);
        if (res != 0){
            res = -EIO;
            goto out;
        }
    }
    stream->pos += total_to_write; //change disk pos in streamer
    in += total_to_write;
    total -= total_to_write;
    if(total > 0){ //recursivly call write till everything is written
        write_bytes_to_disk(stream, in, total);
    }
    
    out:
    return res;
}

void delete_streamer(struct disk_streamer* stream){
    kfree(stream);  //free allocated mem for streamer
}