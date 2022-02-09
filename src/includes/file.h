#ifndef FILE_H
#define FILE_H
#include <stdint.h>
#include "disk.h"
#include "pathparser.h"

/*defines virtul filesystem (VFS) functionality
*/

//mode for setting position in file via fseek function
typedef uint32_t FILE_SEEK_MODE;
enum SEEK_MODE{SEEK_SET, SEEK_CUR, SEEK_END};

//mode for opening the file
typedef uint32_t FILE_MODE;
enum FM{FILE_MODE_READ, FILE_MODE_WRITE, FILE_MODE_APPEND, FILE_MODE_INVALID};

struct disk;
struct file_descriptor;
struct file_stats;

//defines new types for function pointer for basic file functions 
//which will point to functions of any supported filesystem that's on the disk  
typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_dir* path, FILE_MODE mode);
typedef int32_t (*FS_RESOLVE_FUNCTION)(struct disk* disk);
typedef int32_t (*FS_READ_FUNCTION)(void* buf, size_t block_size, size_t n_memb, struct file_descriptor* f_desc);
typedef int32_t (*FS_STAT_FUNCTION)(struct file_descriptor* f_desc, struct file_stats* f_stats);
typedef int32_t (*FS_SEEK_FUNCTION)(struct file_descriptor* f_desc, int32_t offset, int32_t pos);
typedef int32_t (*FS_CLOSE_FUNCTION)(struct file_descriptor* f_desc);
typedef int32_t (*FS_WRITE_FUNCTION)(void* buf, size_t block_size, size_t n_memb, struct file_descriptor* f_desc);


//structure holds pointers to file functions that are process the file from that filesystem 
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

//holds information about opened files
struct file_descriptor{
    uint32_t index; //descriptor number
    void* private;  //private data, type depends of filesystem, holds info that certain filesystem uses
                    //to faster process this file
    struct filesystem* fs;  //filesystem in which this file is stored
    struct disk* disk;      //disk on which this file is stored
};


//holds some stats (info) about file such as creation date, size ...
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