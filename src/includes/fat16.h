#ifndef FAT16_H
#define FAT16_H
#include "file.h"

struct filesystem* fat16_init();
void* fat16_open(struct disk* disk, struct path_dir* path, FILE_MODE mode);


#endif