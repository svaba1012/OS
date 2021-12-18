#include "fat16.h"
#include "status.h"
#include "string.h"

struct filesystem fat16_filesystem;

int32_t fat16_resolve(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_dir* path, FILE_MODE mode);

struct filesystem* fat16_init(){
    fat16_filesystem.open = fat16_open;
    fat16_filesystem.resolve = fat16_resolve;
    strcpy(fat16_filesystem.name, "FAT16");
    return &fat16_filesystem; 
}

void* fat16_open(struct disk* disk, struct path_dir* path, FILE_MODE mode){
    return NULL;
}

int32_t fat16_resolve(struct disk* disk){
    return -EIO;
}