#ifndef FAT16_H
#define FAT16_H
#include "file.h"

/*File allocation table 16 (FAT16) filesystem defines basic file functionality for this fs*/

struct filesystem* fat16_init();
void* fat16_open(struct disk* disk, struct path_dir* path, FILE_MODE mode);


#endif