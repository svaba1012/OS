#include "fat16.h"
#include "status.h"
#include "string.h"
#include "disk.h"
#include "disk_streamer.h"
#include "kheap.h"
#include "status.h"
#include "memory.h"
#include "terminal.h"
#include <stdbool.h>

#define MY_OS_FAT16_SIGNATURE 0x29
#define MY_OS_FAT16_FAT_ENTRY_SIZE 2
#define MY_OS_FAT16_BAD_SECTOR 0xFF7
#define MY_OS_FAT16_UNUSED 0 

//typedef uint32_t FAT_ITEM_TYPE;
typedef uint16_t FAT_ENTRY;
//enum{FAT_ITEM_DIR, FAT_ITEM_FILE};

//bitmasks for FAT16 attribute field in dir items
#define FAT16_FILE_READ_ONLY 0x01
#define FAT16_FILE_HIDDEN 0x02
#define FAT16_FILE_SYSTEM 0x04
#define FAT16_FILE_VOLUME_LABEL 0x08
#define FAT16_FILE_SUBDIRECTORY 0x10
#define FAT16_FILE_ARCHIVED 0x20
#define FAT16_FILE_DEVICE 0x40
#define FAT16_FILE_RESERVED 0x80

#define FAT16_SHORT_FILE_NAME_MAX_LEN 8
#define FAT16_EXTENSION_MAX_LEN 3
#define FAT16_FULL_FILE_NAME_MAX_LEN 11

struct fat16_header{
    uint8_t short_jmp[3];
    uint8_t OEMIdentifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectrors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entry;
    uint16_t num_sectors; //if 0 means more than 65535
    uint8_t meadia_type;
    uint16_t sectrors_per_fat;
    uint16_t sectors_per_track;
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
    uint8_t filename[8];
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

/*
try without this
struct fat_item{
    union{
        struct fat_directory_item* item;
        struct fat_directory* directory;
    };
    FAT_ITEM_TYPE type;
};*/

struct fat_item_descriptor{
    struct fat_directory_item* item;
    uint32_t pos;
};

struct fat_private{
    struct fat16_h header;
    struct fat_directory root_directory; 
    struct disk_streamer* cluster_read_stream;
    struct disk_streamer* fat_read_stream;
    struct disk_streamer* directory_stream;
};

struct filesystem fat16_filesystem;

int32_t fat16_resolve(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_dir* path, FILE_MODE mode);
int32_t fat16_read(void* buf, size_t block_size, size_t n_memb, struct file_descriptor* f_desc);
int32_t fat16_write(void* buf, size_t block_size, size_t n_memb, struct file_descriptor* f_desc);
int32_t fat16_seek(struct file_descriptor* f_desc, int32_t offset, int32_t pos);
int32_t fat16_stat(struct file_descriptor* f_desc, struct file_stats* f_stats);
int32_t fat16_close(struct file_descriptor* f_desc);
//implement more fat16_functionality


struct filesystem* fat16_init(){
    fat16_filesystem.open = fat16_open;
    fat16_filesystem.resolve = fat16_resolve;
    fat16_filesystem.read = fat16_read;
    fat16_filesystem.seek = fat16_seek;
    fat16_filesystem.stat = fat16_stat;
    fat16_filesystem.close = fat16_close;
    fat16_filesystem.write = fat16_write;
    strcpy(fat16_filesystem.name, "FAT16");
    return &fat16_filesystem; 
}

int32_t find_free_cluster(struct fat_private* fat_private, int32_t* cluster_ind){
    int32_t res = 0;
    FAT_ENTRY entry_fat = 1;
    uint32_t fat_entry_disk_pos = fat_private->header.primary_header.reserved_sectors; //start of first fat
    uint32_t size_of_fat = fat_private->header.primary_header.sectrors_per_fat;
    uint32_t num_of_fat_copies = fat_private->header.primary_header.fat_copies;
    int32_t cluster_index = 2;
    for(cluster_index = 2; cluster_index < size_of_fat * MY_OS_DISK_SECTOR_SIZE / sizeof(FAT_ENTRY); cluster_index++){
        fat_entry_disk_pos += cluster_index * 2; 
        set_disk_pos(fat_private->fat_read_stream, fat_entry_disk_pos);
        if(read_bytes_from_disk(fat_private->fat_read_stream, &entry_fat, sizeof(FAT_ENTRY)) != ALL_OK){
            res = -EIO;
            goto out;
        }
        if(entry_fat == 0){ //cluster is free
            entry_fat = 0xFFFF;//look for the value in fat that represent occupied FAT
            for(int32_t i = 0; i < num_of_fat_copies; i++){ //set fat entries in all fat copies
                set_disk_pos(fat_private->fat_read_stream, fat_entry_disk_pos);
                if(write_bytes_to_disk(fat_private->fat_read_stream, &entry_fat, sizeof(FAT_ENTRY)) != ALL_OK){
                    res = -EIO;
                    goto out;
                }
                fat_entry_disk_pos += size_of_fat * MY_OS_DISK_SECTOR_SIZE;
            }
            
            *cluster_ind = cluster_index;
            goto out;
        }
    }

    out: 
    return res;
}

int32_t fat16_create_file(char* filename, struct fat_directory_item* new_entry, uint32_t item_disk_pos, int32_t num_ent, struct fat_private* fat_private){
    int32_t res = 0;
    //set new item entry
    //implement time and date mesuring first to set dates and times of creation, access and last modification
    new_entry->filesize = 0;
    new_entry->attribute = 0;
    char* ext = strnchr(filename, '.', FAT16_FULL_FILE_NAME_MAX_LEN);
    if(ext != NULL){
        *ext = '\0';
        ext++;
        strncpy((char*)new_entry->ext, ext, FAT16_EXTENSION_MAX_LEN);
    }
    strncpy((char*)new_entry->filename, filename, FAT16_SHORT_FILE_NAME_MAX_LEN);
    //find first free cluster in FAT
    int32_t cluster_index;
    res = find_free_cluster(fat_private, &cluster_index);
    if(res < 0){
        goto out;
    }
    new_entry->low_16_bits_first_cluster = cluster_index & 0xffff;
    new_entry->high_16_bits_first_cluster = (cluster_index >> 16) & 0xffff;
    //long filename entry, need to implement
    //...
    //...
    set_disk_pos(fat_private->cluster_read_stream, item_disk_pos + num_ent * sizeof(struct fat_directory_item));
    if(write_bytes_to_disk(fat_private->cluster_read_stream, new_entry, sizeof(struct fat_directory_item)) != ALL_OK){
        res = -EIO;
        goto out;
    }
    out:
    return res;
}

uint32_t get_cluster_position(uint16_t high_bits, uint16_t low_bits){
    return (high_bits << 16) | low_bits;  
}

bool is_valid_path_part(struct path_dir* path, struct fat_directory_item* dir_item){
    int32_t res = 0;
    bool is_sub_dir = (dir_item->attribute & FAT16_FILE_SUBDIRECTORY) == FAT16_FILE_SUBDIRECTORY;
    if(path->next == NULL){
        if(is_sub_dir){
        res = -EBADPATH;
        return false;
        }
    }else{
        if(!is_sub_dir){
            res = -EBADPATH;
            return false;
        }
    }
    if(res < 0){
        return false;
    }
    return true;
}

bool compare_searched_name_with_file_name(struct path_dir* path, struct fat_directory_item* dir_item){
    //checking if searched path exist 
    int32_t res = 0;
    char* path_name = path->dir_name;
    if(strlen(path_name) > FAT16_FULL_FILE_NAME_MAX_LEN){//checking if searched filename is too long
        res = -EINVARG;
        return false;
    }

    if(strncmp_terminating_char((char*)(dir_item->filename),path_name, FAT16_SHORT_FILE_NAME_MAX_LEN, ' ') == 0){
        //comparing existing item names with searched one
        char* file_extension = strnchr(path_name, '.', FAT16_FULL_FILE_NAME_MAX_LEN);
        if(file_extension == NULL && dir_item->ext[0] == ' '){  
            return true;
        }
        file_extension++;
        if((strncmp(file_extension, (char*)(dir_item->ext), FAT16_EXTENSION_MAX_LEN) == 0) && (path->next == NULL)){
            return true;
        }
    }
    if(res <= 0){
        return false;
    }
    return true;
}

int32_t fat16_get_total_items_for_directory(struct disk* disk, int32_t dir_start_sector_pos){
    int32_t total_item = 0;
    struct fat_directory_item item;
    struct fat_private* fat_private = disk->fs_private;
    struct disk_streamer* stream = fat_private->directory_stream;
    set_disk_pos(stream, dir_start_sector_pos * disk->sectors_size);

    while(1){
        if(read_bytes_from_disk(stream, &item, sizeof(item)) != ALL_OK){
            total_item = -EIO;
            goto out;
        }
        if(item.filename[0] == 0x00){
            //empty entry, break
            break;
        }
        //unused directory
        if(item.filename[0] == 0xE5){
            continue;
        }
        
        total_item++;
    }
    out:
    return total_item;
}

void* read_directory_item_entries(struct disk* disk, struct fat_directory_item* dir_item, uint32_t* item_disk_pos){
    int32_t res = 0; 
    void* buf_start = NULL;
    if((dir_item->attribute & FAT16_FILE_SUBDIRECTORY) == 0){// checking if item is subdirectory
        res = -EINVARG;
        goto out;
    }
    struct fat_private* fat_private = disk->fs_private;
    struct fat16_header* prim_header = &fat_private->header.primary_header;
    int32_t cluster_sector_size = prim_header->sectrors_per_cluster;
    //int32_t cluster_size =  cluster_sector_size * disk->sectors_size;
    uint32_t cluster_pos = get_cluster_position(dir_item->high_16_bits_first_cluster, dir_item->low_16_bits_first_cluster);
    uint32_t data_clusters_start_pos = fat_private->root_directory.ending_sector_pos;
    uint32_t f_size = sizeof(struct fat_directory_item) * fat16_get_total_items_for_directory(disk, data_clusters_start_pos + (cluster_pos - 2) * cluster_sector_size);
    dir_item->filesize = f_size;
    buf_start = kzalloc(f_size);
    if(buf_start == NULL){
        res = -ENOMEM;
        goto out;
    }
    cluster_pos = data_clusters_start_pos + (cluster_pos-2) * cluster_sector_size;
    *item_disk_pos = cluster_pos * disk->sectors_size;
    set_disk_pos(fat_private->cluster_read_stream, *item_disk_pos);

    if(read_bytes_from_disk(fat_private->cluster_read_stream, buf_start, f_size) != ALL_OK){
        res = -EIO;
        goto out;
    }
    out:
    if(res < 0){
        if(buf_start != NULL){ //if there is an error and buffer is allocated free it
            kfree(buf_start);
            buf_start = NULL;
        }
    }
    return buf_start;
}

int32_t read_internal(struct disk* disk, struct fat_directory_item* dir_item, void* buf, int32_t offset, uint32_t block_size, uint32_t block_num){
    int32_t res = 0;
    if(dir_item->attribute & FAT16_FILE_SUBDIRECTORY){//is subdir if yes error
        res = -EINVARG;
        goto out;
    }
    struct fat_private* fat_private = disk->fs_private;
    struct fat16_header* prim_header = &fat_private->header.primary_header;
    uint32_t f_size = dir_item->filesize;
    int32_t cluster_sector_size = prim_header->sectrors_per_cluster;
    int32_t cluster_size =  cluster_sector_size * disk->sectors_size;
    uint32_t cluster_pos = get_cluster_position(dir_item->high_16_bits_first_cluster, dir_item->low_16_bits_first_cluster);
    uint32_t data_clusters_start_pos = fat_private->root_directory.ending_sector_pos;

    int32_t num_of_clusters = f_size / cluster_size;
    if(f_size % cluster_size != 0){
        num_of_clusters++;
    }
    uint32_t fat1_start_pos = prim_header->reserved_sectors;
    FAT_ENTRY* cluster_pointer;
    cluster_pointer = kzalloc(sizeof(FAT_ENTRY) * num_of_clusters);
    cluster_pointer[0] = cluster_pos;//change
    for(int32_t i = 1; i < num_of_clusters; i++){//finding all cluster indexes where item data is stored 
        set_disk_pos(fat_private->fat_read_stream, fat1_start_pos * disk->sectors_size + cluster_pointer[i-1]*sizeof(FAT_ENTRY));
        if(read_bytes_from_disk(fat_private->fat_read_stream, &cluster_pointer[i], sizeof(FAT_ENTRY)) != ALL_OK){
            res = -EIO;
            goto out;
        }
        if(cluster_pointer[i] == 0xFF0 || cluster_pointer[i] == 0xFF6){//sector is reserved
            res = -EIO;
            goto out;
        }
        if(cluster_pointer[i] == 0){
            res = -EIO;
            goto out;
        }
        if(cluster_pointer[i] == MY_OS_FAT16_BAD_SECTOR){
            res = -EIO;
            goto out;
        }
    }
    int32_t cluster_index = offset / cluster_size;
    int32_t cluster_offset = offset % cluster_size;
    int32_t left_to_read = block_size * block_num;
    int32_t reading = cluster_size;
    void* buf_c = buf;
    while(left_to_read > 0){
        cluster_pos = data_clusters_start_pos + (cluster_pointer[cluster_index]-2) * cluster_sector_size;
        set_disk_pos(fat_private->cluster_read_stream, cluster_pos * disk->sectors_size + cluster_offset);
        cluster_offset = 0;
        if(left_to_read < cluster_size){
            reading = left_to_read; 
        }
        if(read_bytes_from_disk(fat_private->cluster_read_stream, buf_c,reading) != ALL_OK){
            res = -EIO;
            goto out;
        }
        buf_c += reading;
        left_to_read -= reading; 
        cluster_index++;
    } 
    out:
    if(res < 0){
        return 0;
    }
    return block_size * block_num - left_to_read;
}

/*void* read_clusters(struct disk* disk, struct fat_directory_item* dir_item){
    int32_t res = 0;
    void* buf = NULL;
    void* buf_start = NULL;
    struct fat_private* fat_private = disk->fs_private;
    struct fat16_header* prim_header = &fat_private->header.primary_header;
    uint32_t f_size = dir_item->filesize;
    int32_t cluster_sector_size = prim_header->sectrors_per_cluster;
    int32_t cluster_size =  cluster_sector_size * disk->sectors_size;
    uint32_t cluster_pos = get_cluster_position(dir_item->high_16_bits_first_cluster, dir_item->low_16_bits_first_cluster);
    uint32_t data_clusters_start_pos = fat_private->root_directory.ending_sector_pos;
    if(dir_item->attribute & FAT16_FILE_SUBDIRECTORY){//seting file_size if the item is subdirectory
        f_size = sizeof(struct fat_directory_item) * fat16_get_total_items_for_directory(disk, data_clusters_start_pos + (cluster_pos - 2) * cluster_sector_size);
        dir_item->filesize = f_size;
    }    
    int32_t num_of_clusters = f_size / cluster_size;
    if(f_size % cluster_size != 0){
        num_of_clusters++;
    }
    uint32_t fat1_start_pos = prim_header->reserved_sectors;
    FAT_ENTRY* cluster_pointer;
    cluster_pointer = kzalloc(sizeof(FAT_ENTRY) * num_of_clusters);
    cluster_pointer[0] = cluster_pos;//change
    for(int32_t i = 1; i < num_of_clusters; i++){//finding all cluster indexes where are item data is stored 
        set_disk_pos(fat_private->fat_read_stream, fat1_start_pos * disk->sectors_size + cluster_pointer[i-1]*sizeof(FAT_ENTRY));
        if(read_bytes_from_disk(fat_private->fat_read_stream, &cluster_pointer[i], sizeof(FAT_ENTRY)) != ALL_OK){
            res = -EIO;
            goto out;
        }
        if(cluster_pointer[i] == 0xFF0 || cluster_pointer[i] == 0xFF6){//sector is reserved
            res = -EIO;
            goto out;
        }
        if(cluster_pointer[i] == 0){
            res = -EIO;
            goto out;
        }
        if(cluster_pointer[i] == MY_OS_FAT16_BAD_SECTOR){
            res = -EIO;
            goto out;
        }
    }
    buf = kzalloc(f_size);
    if(buf == NULL){
        res = -ENOMEM;
        goto out;
    }
    buf_start = buf;
    cluster_pos = data_clusters_start_pos + (cluster_pos-2) * cluster_sector_size;
    for(int32_t i = 1; i < num_of_clusters; i++){
        set_disk_pos(fat_private->cluster_read_stream, cluster_pos * disk->sectors_size);
        if(read_bytes_from_disk(fat_private->cluster_read_stream, buf,cluster_size) != ALL_OK){
            res = -EIO;
            goto out;
        }
        buf += cluster_size;
        cluster_pos = data_clusters_start_pos + (cluster_pointer[i]-2) * cluster_sector_size;
    }
    set_disk_pos(fat_private->cluster_read_stream, cluster_pos * disk->sectors_size);
    if(read_bytes_from_disk(fat_private->cluster_read_stream, buf, f_size % cluster_size) != ALL_OK){
        res = -EIO;
        goto out;
    }
    
    out:
    kfree(cluster_pointer); 
    if(res < 0){
        if(buf_start != NULL){ //if there is an error and buffer is allocated free it
            kfree(buf_start);
            buf_start = NULL;
        }
    }
    return buf_start;
}*/

int32_t fat16_read(void* buf, size_t block_size, size_t n_memb, struct file_descriptor* f_desc){
    int32_t bytes_read = 0;
    int32_t res = 0;
    if(block_size < 0 || n_memb < 0){
        res = -EINVARG;
        goto out;
    }
    if(f_desc == NULL || buf == NULL){
        res = -EINVARG;
        goto out;
    }
    struct fat_item_descriptor* fat_desc = (struct fat_item_descriptor*)(f_desc->private);
    bytes_read = read_internal(f_desc->disk, fat_desc->item, buf, fat_desc->pos, block_size, n_memb);
    fat_desc->pos += bytes_read;
    out: 
    if(res < 0){
        return 0;
    }
    return bytes_read;
}

void* fat16_open(struct disk* disk, struct path_dir* path, FILE_MODE mode){
    int32_t res = 0;
    struct fat_item_descriptor* f_desc = NULL;
    struct fat_directory_item* items_in_dir = NULL;
    void* file_data = NULL;
    if(mode != FILE_MODE_READ){ //first implement writing to disk than this
        res = -ENOTIMPLEMENTEDYET;
        return NULL;
    }
    struct fat_private* fat_private = disk -> fs_private;
    struct fat_directory* root_dir = &(fat_private->root_directory);
    int32_t entries_num = root_dir->total; 
    int32_t all_entries_size = entries_num * sizeof(struct fat_directory_item);
    items_in_dir = kzalloc(all_entries_size); //holds all item in root
    
    if(items_in_dir == NULL){
        res = -ENOMEM;
        return NULL;
    } 
    //all items in root dir will be loaded to items_in_dir
    uint32_t item_disk_pos = root_dir->sector_pos * disk->sectors_size;
    set_disk_pos(fat_private->directory_stream, item_disk_pos);
    if(read_bytes_from_disk(fat_private->directory_stream, items_in_dir,all_entries_size) != ALL_OK){
        res = -EIO;
        return NULL;    
    }//read root dir 
    struct path_dir* cur_dir = path;
    int32_t i = 0;
    struct fat_directory_item* cur_item = NULL;
    while(cur_dir != NULL){
        i = 0;
        for(;i < entries_num; i++){//find searched subdir name in cur_dir
            if(compare_searched_name_with_file_name(cur_dir, &items_in_dir[i])){
                //seached subdir found
                break;
            }
        }
        if(i >= entries_num){
            //searched subdir doesnt exist
            if(mode == FILE_MODE_READ || cur_dir->next != NULL){
                //exit if mode is reading or if the searched item is subdir
                res = -EBADPATH; 
                goto out;
            }else{
                //else create new file for writing
                cur_item = kzalloc(sizeof(struct fat_directory_item));
                if(cur_item == NULL){
                    res = -ENOMEM;
                    goto out;
                }
                if(fat16_create_file(cur_dir->dir_name, cur_item, item_disk_pos, entries_num, fat_private) < 0){
                    res = -EIO;
                    goto out;
                }
            }
        }else{
            cur_item = &items_in_dir[i];
            if(!is_valid_path_part(cur_dir, &items_in_dir[i])){//check path
                res = -EBADPATH;
                goto out;
            }
        }
        if(cur_item->attribute & FAT16_FILE_SUBDIRECTORY){ //if part is subdirectory
            kfree(file_data);
            file_data = read_directory_item_entries(disk, &items_in_dir[i], &item_disk_pos); //read data from found entry
            if(file_data == NULL){
                res = -EIO;
                goto out;
            }
            //code for determing type of file 
            entries_num = items_in_dir[i].filesize / sizeof(struct fat_directory_item);
            //check if something misses
            //check this
            kfree(items_in_dir); //free old entries
            items_in_dir = (struct fat_directory_item*) file_data; //than data represent new entries
            //check more   
        }else{
            f_desc = kzalloc(sizeof(struct fat_item_descriptor));
            if(f_desc == NULL){
                res = -ENOMEM;
                goto out;
            }
            f_desc->item = kzalloc(sizeof(struct fat_directory_item));
            memcpy(f_desc->item, cur_item, sizeof(struct fat_directory_item));
            f_desc->pos = 0;
            if(mode == FILE_MODE_APPEND){
                f_desc->pos = cur_item->filesize;
            }
        }
        cur_dir = cur_dir->next; //move to next part in list
    }
    out:
    kfree(items_in_dir);
    kfree(file_data);
    kfree(cur_item); //check this for error
    if(res < 0){
        return NULL;
    }
    return f_desc;
}



int32_t fat16_get_root_directory(struct disk* disk, struct fat_private* fat_private, struct fat_directory* root_dir){
    int32_t res = 0;
    struct fat16_header* primary_header = &fat_private->header.primary_header; //selecting only primary header
    int32_t num_of_root_dir_entries = primary_header->root_dir_entry;   
    //calculating starting sectror for root dir
    int32_t start_sector_of_root_dir = primary_header->reserved_sectors + primary_header->fat_copies * primary_header->sectrors_per_fat;
    int32_t root_dir_size = num_of_root_dir_entries * sizeof(struct fat_directory_item);
    int32_t root_dir_size_in_sectors = root_dir_size / disk->sectors_size;
    if(root_dir_size % disk->sectors_size){
        root_dir_size_in_sectors++;
    }
    //getting count of all files and subdirectory in root
    int32_t total_items = fat16_get_total_items_for_directory(disk, start_sector_of_root_dir);
    

    struct fat_directory_item* dir = kzalloc(root_dir_size);
    if(dir == NULL){
        res = -ENOMEM;
        goto end;
    }
    struct disk_streamer* stream = fat_private->directory_stream;
    set_disk_pos(stream, start_sector_of_root_dir * disk->sectors_size);
    if(read_bytes_from_disk(stream, dir, root_dir_size) != ALL_OK){
        res = -EIO;
        goto end;
    }

    root_dir->item = dir;
    root_dir->sector_pos = start_sector_of_root_dir;
    root_dir->total = total_items;
    root_dir->ending_sector_pos = start_sector_of_root_dir + root_dir_size / disk->sectors_size;

    end:
    if(res < 0){
        kfree(dir);
    }
    return res;
}

int32_t fat16_resolve(struct disk* disk){
    int32_t res = 0;
    struct fat_private* fat_private = kzalloc(sizeof(fat_private));
    if(fat_private == NULL){
        res = -ENOMEM;
        goto out;
    }
    disk->fs_private = fat_private;
    disk->filesystem = &fat16_filesystem;
    fat_private->fat_read_stream = new_disk_stream(disk->id);
    fat_private->directory_stream = new_disk_stream(disk->id);
    fat_private->cluster_read_stream = new_disk_stream(disk->id);

    struct disk_streamer* stream = new_disk_stream(disk->id);
    if(stream == NULL){
        res = -ENOMEM;
        goto out;
    }
    if(read_bytes_from_disk(stream, &fat_private->header, sizeof(fat_private->header)) != ALL_OK){
        //reading FAT16 header from boot sector into header in c
        res = -EIO;
        goto out;
    }

    if(fat_private->header.shared.extended_header.signature != 0x29){
        //Checking signature
        res = -EFSNOTUS;
        goto out;
    }

    if(fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != ALL_OK){
        res = -EIO;
        goto out;
    }

    out:
    if(stream){
        delete_streamer(stream);
    }
    if(res < 0){
        kfree(fat_private);
        disk->fs_private = 0;
        disk->filesystem = 0;
    }
    return res;
}

int32_t fat16_seek(struct file_descriptor* f_desc, int32_t offset, int32_t pos){
    int32_t res = 0;
    struct fat_item_descriptor* fat_desc = (struct fat_item_descriptor*)f_desc->private;
    uint32_t f_size = fat_desc->item->filesize;
    switch (pos){
    case SEEK_SET:
        if(offset < 0){
            res = -EINVARG;
            goto out;
        }
        fat_desc->pos = offset;
        break;

    case SEEK_CUR:
        if(offset + (int32_t)fat_desc->pos < 0 || offset + (int32_t)fat_desc->pos > f_size){
            res = -EINVARG;
            goto out;
        }
        fat_desc ->pos += offset;
        break;

    case SEEK_END:
        if(offset > 0){
            res = -EINVARG;
            goto out;
        }
        fat_desc ->pos = f_size + offset;
        break;

    default:
        res = -EINVARG;
        break;
    }
    out:
    return res;
}

int32_t fat16_stat(struct file_descriptor* f_desc, struct file_stats* f_stats){
    struct fat_item_descriptor* fat_desc = (struct fat_item_descriptor*)f_desc->private;
    struct fat_directory_item* file_entry = fat_desc->item;
    f_stats->user = NULL; //not implemented yet
    f_stats->file_size = (int32_t)file_entry->filesize;
    f_stats->creation_year = ((file_entry->creation_date >> 9) & 0x007F) + 1980;
    f_stats->creation_mounth = (file_entry->creation_date >> 5) & 0x000F;
    f_stats->creation_day = file_entry->creation_date & 0x001F;
    f_stats->mod_year = ((file_entry->last_mod_date >> 9) & 0x007F) + 1980;
    f_stats->mod_mounth = (file_entry->last_mod_date >> 5) & 0x000F;
    f_stats->mod_day = file_entry->last_mod_date & 0x001F;
    f_stats->acc_year = ((file_entry->last_access >> 9) & 0x007F) + 1980;
    f_stats->acc_mounth = (file_entry->last_access >> 5) & 0x000F;
    f_stats->acc_day = file_entry->last_access & 0x001F;
    f_stats->creation_hour = (file_entry->creation_time >> 11) & 0x001F;
    f_stats->creation_minute = (file_entry->creation_time >> 5) & 0x003F;
    f_stats->creation_sec = (file_entry->creation_time & 0x001F) * 2;
    f_stats->mod_hour = (file_entry->last_mod_time >> 11) & 0x001F;
    f_stats->mod_minute = (file_entry->last_mod_time >> 5) & 0x003F;
    f_stats->mod_sec = (file_entry->last_mod_time & 0x001F) * 2;
    return 0;
}

int32_t fat16_close(struct file_descriptor* f_desc){
    struct fat_item_descriptor* fat_desc = (struct fat_item_descriptor*)f_desc->private;
    kfree(fat_desc->item);
    kfree(fat_desc);
    kfree(f_desc);
    return 0;
}


int32_t write_internal(struct disk* disk, struct fat_directory_item* file_item, void* buf, uint32_t file_pos, size_t n_memb, size_t block_size){
    int32_t bytes_written = 0;
    int32_t res = 0;

    if(res){
        
    }

    out:
    return bytes_written;
}

int32_t fat16_write(void* buf, size_t block_size, size_t n_memb, struct file_descriptor* f_desc){
    int32_t bytes_written = 0;
    int32_t res = 0;
    if(block_size < 0 || n_memb < 0){
        res = -EINVARG;
        goto out;
    }
    if(f_desc == NULL || buf == NULL){
        res = -EINVARG;
        goto out;
    }
    struct fat_item_descriptor* fat_desc = (struct fat_item_descriptor*)(f_desc->private);
    bytes_written = write_internal(f_desc->disk, fat_desc->item, buf, fat_desc->pos, block_size, n_memb);
    fat_desc->pos += bytes_written;
    out: 
    if(res < 0){
        return 0;
    }
    return bytes_written;
}








