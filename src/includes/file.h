#ifndef FILE_H
#define FILE_H
#include <stdint.h>
#include "disk.h"
#include "pathparser.h"

typedef uint32_t FILE_SEEK_MODE;
enum SEEK_MODE{SEEK_SET, SEEK_CUR, SEEK_END};

typedef uint32_t FILE_MODE;
enum FM{FILE_MODE_READ, FILE_MODE_WRITE, FILE_MODE_APPEND, FILE_MODE_INVALID};

struct disk;
struct file_descriptor;
struct file_stats;

typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_dir* path, FILE_MODE mode);
typedef int32_t (*FS_RESOLVE_FUNCTION)(struct disk* disk);
typedef int32_t (*FS_READ_FUNCTION)(void* buf, size_t block_size, size_t n_memb, struct file_descriptor* f_desc);
typedef int32_t (*FS_STAT_FUNCTION)(struct file_descriptor* f_desc, struct file_stats* f_stats);//?
typedef int32_t (*FS_SEEK_FUNCTION)(struct file_descriptor* f_desc, int32_t offset, int32_t pos);//+
typedef int32_t (*FS_CLOSE_FUNCTION)(struct file_descriptor* f_desc);//?

typedef int32_t (*FS_WRITE_FUNCTION)(void* buf, size_t block_size, size_t n_memb, struct file_descriptor* f_desc);




struct filesystem{
    FS_RESOLVE_FUNCTION resolve; //return 0 if disk is usable
    FS_OPEN_FUNCTION open;
    FS_READ_FUNCTION read;
    FS_SEEK_FUNCTION seek;
    FS_STAT_FUNCTION stat;
    FS_CLOSE_FUNCTION close;
    FS_WRITE_FUNCTION write;
    char name[20];
};

struct file_descriptor{
    uint32_t index; //descriptor number
    void* private;
    struct filesystem* fs;
    struct disk* disk;
};

struct file_stats{
    uint32_t file_size;
    uint8_t creation_day;
    uint8_t creation_mounth;
    uint16_t creation_year;
    uint8_t creation_hour; 
    uint8_t creation_minute;
    uint8_t creation_sec; 
    uint8_t mod_day;
    uint8_t mod_mounth;
    uint16_t mod_year;
    uint8_t mod_hour; 
    uint8_t mod_minute;
    uint8_t mod_sec;
    uint8_t acc_day;
    uint8_t acc_mounth;
    uint16_t acc_year;
    char* user;
};

void filesystem_init();
int32_t fopen(char* path_name, char* mode_str);
void insert_filesystem(struct filesystem* new_fs);
struct filesystem* fs_resolve(struct disk* disk);
int32_t fread(void* buf, size_t block_size, int32_t num_of_blocks, uint32_t fd);
int32_t fseek(uint32_t fd, int32_t offset, int32_t pos);
int32_t fstat(uint32_t fd, struct file_stats* f_stats);
int32_t fclose(uint32_t fd);
int32_t fwrite(void* buf, size_t block_size, int32_t num_of_blocks, uint32_t fd);

//implement more file functionality



#endif