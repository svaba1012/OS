#include "fat16.h"
#include "status.h"
#include "string.h"
#include "disk.h"
#include "disk_streamer.h"

#define MY_OS_FAT16_SIGNATURE 0x29
#define MY_OS_FAT16_FAT_ENTRY_SIZE 2
#define MY_OS_FAT16_BAD_SECTOR 0xFF7
#define MY_OS_FAT16_UNUSED 0 

typedef uint32_t FAT_ITEM_TYPE;
enum{FAT_ITEM_DIR, FAT_ITEM_FILE};

//bitmasks for FAT16 attribute field in dir items
#define FAT16_FILE_READ_ONLY 0x01
#define FAT16_FILE_HIDDEN 0x02
#define FAT16_FILE_SYSTEM 0x04
#define FAT16_FILE_VOLUME_LABEL 0x08
#define FAT16_FILE_SUBDIRECTORY 0x10
#define FAT16_FILE_ARCHIVED 0x20
#define FAT16_FILE_DEVICE 0x40
#define FAT16_FILE_RESERVED 0x80


struct fat16_header{
    uint8_t short_jmp[3];
    uint8_t OEMIdentifier[8];
    uint16_t bytes_per_cluster;
    uint8_t sectrors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entry;
    uint16_t num_sectors; //if 0 means more than 65535
    uint8_t meadia_type;
    uint16_t sectrors_per_fat;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sector_big;
} __attribute__((packed));

struct fat16_extended_header{
    uint8_t drive_num; 
    uint8_t winNTBit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct fat16_h{
    struct fat16_header primary_header;
    union fat16_h_e{
        struct fat16_extended_header extended_header;
    }shared;
} __attribute__((packed));

struct fat_directory_item{
    uint8_t filenamep[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

struct fat_directory{
    struct fat_directory_item* item;
    uint32_t total;
    uint32_t sector_pos;
    uint32_t ending_sector_pos;
};

struct fat_item{
    union{
        struct fat_directory_item* item;
        struct fat_directory* directory;
    };
    FAT_ITEM_TYPE type;
};

struct fat_item_descriptor{
    struct fat_item* item;
    uint32_t pos;
};

struct fat_private{
    struct fat16_h header;
    struct fat_directory root_directory; 
    struct disk_stream* cluster_read_stream;
    struct disk_stream* fat_read_stream;
    struct disk_stream* directory_stream;
};

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