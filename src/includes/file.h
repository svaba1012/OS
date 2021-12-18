#ifndef FILE_H
#define FILE_H
#include <stdint.h>
#include "disk.h"
#include "pathparser.h"

typedef uint32_t FILE_SEEK_MODE;
enum SEEK_MODE{SEEK_SET, SEEK_CUR, SEEK_END};

typedef uint32_t FILE_MODE;
enum FM{FILE_MODE_READ, FILE_MODE_WRITE, FILE_MODE_APPEND};

struct disk;

typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_dir* path, FILE_MODE mode);
typedef int32_t (*FS_RESOLVE_FUNCTION)(struct disk* disk);

struct filesystem{
    FS_RESOLVE_FUNCTION resolve; //return 0 if disk is usable
    FS_OPEN_FUNCTION open;
    char name[20];
};

struct file_descriptor{
    uint32_t index; //descriptor number
    void* private;
    struct filesystem* fs;
    struct disk* disk;
};

void filesystem_init();
int32_t fopen(char* path_name, char* mode);
void insert_filesystem(struct filesystem* new_fs);
struct filesystem* fs_resolve(struct disk* disk);

#endif