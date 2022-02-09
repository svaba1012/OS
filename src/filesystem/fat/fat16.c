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

/*File allocation table is one of the most simpliest filesystem, yet it's the best for learning
You can look how it works here link https://wiki.osdev.org/FAT
*/

//signature number that should be located on the disk is disk filesystem is FAT16
#define MY_OS_FAT16_SIGNATURE 0x29
//size of one FAT entry block
#define MY_OS_FAT16_FAT_ENTRY_SIZE 2
//number in FAT that resebles bad sector in cluster
#define MY_OS_FAT16_BAD_SECTOR 0xFF7
//number in FAT that resebles unused cluster
#define MY_OS_FAT16_UNUSED 0 

//new type for elemetnt in FAT
typedef uint16_t FAT_ENTRY;

//bitmasks for FAT16 attribute field in dir items
#define FAT16_FILE_READ_ONLY 0x01
#define FAT16_FILE_HIDDEN 0x02
#define FAT16_FILE_SYSTEM 0x04
#define FAT16_FILE_VOLUME_LABEL 0x08
#define FAT16_FILE_SUBDIRECTORY 0x10
#define FAT16_FILE_ARCHIVED 0x20
#define FAT16_FILE_DEVICE 0x40
#define FAT16_FILE_RESERVED 0x80

//lenght of string for filename in fat_directory_item
#define FAT16_SHORT_FILE_NAME_MAX_LEN 8
//lenght of string for extension(ext) in fat_directory_item
#define FAT16_EXTENSION_MAX_LEN 3
//sum of those two above
#define FAT16_FULL_FILE_NAME_MAX_LEN 11

//structure that resembles FAT header data stored on boot sector 
struct fat16_header{
    uint8_t short_jmp[3];   //first 3 bytes used for instructions to jump over FAT header
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

//extended FAT header also located in boot sector imediatly after standard FAT header
struct fat16_extended_header{
    uint8_t drive_num; 
    uint8_t winNTBit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

//structure that holds both headers 
struct fat16_h{
    struct fat16_header primary_header;
    union fat16_h_e{
        struct fat16_extended_header extended_header;
    }shared;
} __attribute__((packed));

//structure for loading directory/file entries (short names) from directory 
struct fat_directory_item{
    uint8_t filename[8];                    //name of file or dir
    uint8_t ext[3];                         //name of extension
    uint8_t attribute;                      //flags that describes type of file
    uint8_t reserved;                       //unused
    uint8_t creation_time_tenths_of_a_sec;  //creation, last modification, last access time and date
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;    //high 16 bits of position on disk of first cluster 
                                            //where file data is stored
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;     //low 16 bits of position of cluster
    uint32_t filesize;                      //size of file in bytes
} __attribute__((packed));      


//structure used to hold info about root directory
struct fat_directory{
    struct fat_directory_item* item;    //array of file/directory entries in root
    uint32_t total;                     //number of file/dir entries
    uint32_t sector_pos;                //starting sector of dir
    uint32_t ending_sector_pos;         //end sec of dir
};

//returned as private data to file descriptor
//holds all necesery data for opened file
struct fat_item_descriptor{
    struct fat_directory_item* item;            //pointer to file entry of opened file
    struct fat_directory_item* upper_subdir;    //pointer to dir entry of subdir of opened file
    int32_t index_in_subdir;                    //index number of file entry of opened file in subdir
    uint32_t pos;                               //position in file
};

//helpful structure for holding all necesery info about FAT 
struct fat_private{
    struct fat16_h header;
    struct fat_directory root_directory; 
    struct disk_streamer* cluster_read_stream;
    struct disk_streamer* fat_read_stream;
    struct disk_streamer* directory_stream;
};

struct filesystem fat16_filesystem;

//looks into disk if its filesystem is FAT16, and saves all necessery info
int32_t fat16_resolve(struct disk* disk);   
void* fat16_open(struct disk* disk, struct path_dir* path, FILE_MODE mode);
int32_t fat16_read(void* buf, size_t block_size, size_t n_memb, struct file_descriptor* f_desc);
int32_t fat16_write(void* buf, size_t block_size, size_t n_memb, struct file_descriptor* f_desc);
int32_t fat16_seek(struct file_descriptor* f_desc, int32_t offset, int32_t pos);    //change file position
int32_t fat16_stat(struct file_descriptor* f_desc, struct file_stats* f_stats);     //get info about file
int32_t fat16_close(struct file_descriptor* f_desc);              //close file, free all allocations
//implement more fat16_functionality


struct filesystem* fat16_init(){
    //set all atributes of struct filesystem of FAT16
    //set all function pointer
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
    //this func iterate over the FAT and looks for entry which number represent free cluster
    //when it finds that entry, index of that entry minus two represnt index of the frist free cluster
    int32_t res = 0;
    FAT_ENTRY entry_fat;
    uint32_t fat_start = fat_private->header.primary_header.reserved_sectors * MY_OS_DISK_SECTOR_SIZE; //start of first fat
    uint32_t size_of_fat = fat_private->header.primary_header.sectrors_per_fat; //size of one FAT
    uint32_t num_of_fat_copies = fat_private->header.primary_header.fat_copies; //number of FAT
    int32_t cluster_index = 3;      //starting index in FAT. 0,1,2 are unused 
    uint32_t fat_entry_disk_pos;    
    for(cluster_index = 3; cluster_index < size_of_fat * MY_OS_DISK_SECTOR_SIZE / sizeof(FAT_ENTRY); cluster_index++){
        //reading FAT entry one by one until finding one that's free
        fat_entry_disk_pos = fat_start + cluster_index * sizeof(FAT_ENTRY); //position on disk of entry
        set_disk_pos(fat_private->fat_read_stream, fat_entry_disk_pos); //set that position on disk
        if(read_bytes_from_disk(fat_private->fat_read_stream, &entry_fat, sizeof(FAT_ENTRY)) != ALL_OK){
            //read one entry
            res = -EIO;
            goto out;
        }
        if(entry_fat == 0){ //cluster is free
            entry_fat = 0xFFFF;//look for the value in fat that represent occupied FAT
            for(int32_t i = 0; i < num_of_fat_copies; i++){ //set fat entries in all fat copies
                set_disk_pos(fat_private->fat_read_stream, fat_entry_disk_pos);
                if(write_bytes_to_disk(fat_private->fat_read_stream, &entry_fat, sizeof(FAT_ENTRY)) != ALL_OK){
                    //now marks that entry to number of ocupied cluster 
                    res = -EIO;
                    goto out;
                }
                fat_entry_disk_pos += size_of_fat * MY_OS_DISK_SECTOR_SIZE; //jump to next FAT
            }
            
            *cluster_ind = cluster_index;   //saves index of free cluster that's found to address cluster_ind
            goto out;
        }
    }
    out: 
    return res;
}

int32_t fat16_create_file(char* filename, struct fat_directory_item* new_entry, uint32_t item_disk_pos, int32_t num_ent, struct fat_private* fat_private){
    //creates new file with name filename, set its file entry to *new_entry structure
    //, which entry's subdir is loacated on item_disk_pos position on disk
    //which has num_ent entries 
    int32_t res = 0;
    //set new item entry
    //implement time and date mesuring first to set dates and times of creation, access and last modification
    new_entry->filesize = 0;    //new file is empty so size is 0
    
    //in further development could be set to new function argument
    new_entry->attribute = 0;   //plain file
    //ectracts extension from filename
    char* ext = strnchr(filename, '.', FAT16_FULL_FILE_NAME_MAX_LEN);
    if(ext != NULL){
        *ext = '\0';
        ext++;
        //copy ext to file entry structure
        strncpy((char*)new_entry->ext, ext, FAT16_EXTENSION_MAX_LEN);
    }
    //copy filename to file entry structure
    strncpy((char*)new_entry->filename, filename, FAT16_SHORT_FILE_NAME_MAX_LEN);
    //find first free cluster in FAT
    int32_t cluster_index;
    res = find_free_cluster(fat_private, &cluster_index);
    //cluster index now holds index of free cluster
    if(res < 0){
        goto out;
    }
    //save cluster index to file entry structure
    new_entry->low_16_bits_first_cluster = cluster_index & 0xffff;
    new_entry->high_16_bits_first_cluster = (cluster_index >> 16) & 0xffff;
    //long filename entry, need to be implemented
    //...
    //...
    //writes newly created file_entry to the corresponding disk position
    //so file is created
    set_disk_pos(fat_private->cluster_read_stream, item_disk_pos + num_ent * sizeof(struct fat_directory_item));
    if(write_bytes_to_disk(fat_private->cluster_read_stream, new_entry, sizeof(struct fat_directory_item)) != ALL_OK){
        res = -EIO;
        goto out;
    }
    out:
    return res;
}

uint32_t get_cluster_position(uint16_t high_bits, uint16_t low_bits){
    //combines high and low bits of cluster index
    return (high_bits << 16) | low_bits;  
}

bool is_valid_path_part(struct path_dir* path, struct fat_directory_item* dir_item){
    //check if file path name is valid
    int32_t res = 0;
    bool is_sub_dir = (dir_item->attribute & FAT16_FILE_SUBDIRECTORY) == FAT16_FILE_SUBDIRECTORY;
    if(path->next == NULL){
        if(is_sub_dir){     //if path part is last and it's subdir there's no file
            res = -EBADPATH;
            return false;
        }
    }else{
        if(!is_sub_dir){    //path part in the midlle is supposed to be dir
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
        //comparing extensions to
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
    //returns number of file/dir entry in subdir
    int32_t total_item = 0;
    struct fat_directory_item item;
    struct fat_private* fat_private = disk->fs_private;
    struct disk_streamer* stream = fat_private->directory_stream;
    //set disk position to starting sector of directory
    set_disk_pos(stream, dir_start_sector_pos * disk->sectors_size);

    while(1){
        //reading file/dir entry one by one and counting till free is found
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
    //read data from directory. reads all file/dir entries from the directory
    int32_t res = 0; 
    void* buf_start = NULL;     //start addres where data from dir will be written 
    if((dir_item->attribute & FAT16_FILE_SUBDIRECTORY) == 0){// checking if item is subdirectory
        //if not error
        res = -EINVARG;
        goto out;
    }
    struct fat_private* fat_private = disk->fs_private;
    struct fat16_header* prim_header = &fat_private->header.primary_header;
    int32_t cluster_sector_size = prim_header->sectrors_per_cluster;
    uint32_t cluster_pos = get_cluster_position(dir_item->high_16_bits_first_cluster, dir_item->low_16_bits_first_cluster);
    uint32_t data_clusters_start_pos = fat_private->root_directory.ending_sector_pos;
    //size of directory is size of items * number of items in it
    uint32_t f_size = sizeof(struct fat_directory_item) * fat16_get_total_items_for_directory(disk, data_clusters_start_pos + (cluster_pos - 2) * cluster_sector_size);
    dir_item->filesize = f_size;
    buf_start = kzalloc(f_size);    
    if(buf_start == NULL){
        res = -ENOMEM;
        goto out;
    }
    //get starting sector of cluster
    cluster_pos = data_clusters_start_pos + (cluster_pos-2) * cluster_sector_size;
    //get position on disk of cluster start
    *item_disk_pos = cluster_pos * disk->sectors_size;
    set_disk_pos(fat_private->cluster_read_stream, *item_disk_pos);
    //read directory from cluster to buf_start
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
    return buf_start;   //returns dir data
}

FAT_ENTRY* get_cluster_indexes(struct fat_private* fat_private, struct fat_directory_item* dir_item){
    //returns array of cluster indexes of clusters where dir_item file data is located
    int32_t res = 0;
    struct fat16_header* prim_header = &fat_private->header.primary_header;
    uint32_t f_size = dir_item->filesize;
    int32_t cluster_sector_size = prim_header->sectrors_per_cluster;
    int32_t cluster_size =  cluster_sector_size * MY_OS_DISK_SECTOR_SIZE;
    //first cluster index is saved in dir_item structure (file/dir entry)
    uint32_t cluster_pos = get_cluster_position(dir_item->high_16_bits_first_cluster, dir_item->low_16_bits_first_cluster);
    int32_t num_of_clusters = f_size / cluster_size;
    if(f_size % cluster_size != 0 || f_size == 0){ //check this
        //if f_size == 0 file is empty but still has one cluster
        num_of_clusters++;
    }
    //calculate number of clusters

    //start sector of FAT
    uint32_t fat1_start_pos = prim_header->reserved_sectors;
    FAT_ENTRY* cluster_pointer;
    //alocate memory for array that will hold indexes of file clusters
    cluster_pointer = kzalloc(sizeof(FAT_ENTRY) * num_of_clusters);
    if(cluster_pointer == NULL){
        res = -ENOMEM;
        goto out;
    }
    cluster_pointer[0] = cluster_pos;
    for(int32_t i = 1; i < num_of_clusters; i++){//finding all cluster indexes where item data is stored 
        //value of current file entry holds index of next. 0xFFFF represent last cluster
        set_disk_pos(fat_private->fat_read_stream, fat1_start_pos * MY_OS_DISK_SECTOR_SIZE + cluster_pointer[i-1]*sizeof(FAT_ENTRY));
        //read that value to next element in array
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
    out:
    if(res < 0){
        kfree(cluster_pointer);
        return NULL;
    }
    return cluster_pointer;     //return array of cluster indexes of the file
}


int32_t read_internal(struct disk* disk, struct fat_directory_item* dir_item, void* buf, int32_t offset, uint32_t block_size, uint32_t block_num){
    //functions to read block_num of blocks which are block_size bytes long, from disk, from file 
    //which entry is dir_item from file position offset to buf 
    
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
    
    int32_t reading = cluster_size;                 //how much it will be read in one instance
    int32_t cluster_index = offset / cluster_size;  //get index of cluster
    int32_t cluster_offset = offset % cluster_size; //number of bytes offset from start of the cluster    
    int32_t left_to_read = block_size * block_num;  //what is left to be read
    void* buf_c = buf;                              //help variable 
    FAT_ENTRY* cluster_pointer = get_cluster_indexes(fat_private, dir_item);    //get array of all cluster indexes
    if(cluster_pointer == NULL){
        res = -EIO;
        goto out;
    }
    if(f_size < left_to_read + offset){
        //if reading out of file bound
        res = -EINVARG; 
        goto out;
    }
    
    while(left_to_read > 0){//iterating while there is still something to read
        //calculate current cluster sector position
        cluster_pos = data_clusters_start_pos + (cluster_pointer[cluster_index]-2) * cluster_sector_size;
        //set position to cluster_offset bytes from start of current cluster
        set_disk_pos(fat_private->cluster_read_stream, cluster_pos * disk->sectors_size + cluster_offset);
        if(left_to_read < cluster_size){    //reading one cluster at time
            reading = left_to_read;         //if last cluster
        }
        if((cluster_offset != 0) && (reading + cluster_offset > cluster_size)){
            reading = cluster_size - cluster_offset;        //read in bound of a cluster
        }
        //read data from cluster
        if(read_bytes_from_disk(fat_private->cluster_read_stream, buf_c,reading) != ALL_OK){
            res = -EIO;
            goto out;
        }
        cluster_offset = 0;
        buf_c += reading;           //move to the end of read data in buffer
        left_to_read -= reading;    
        cluster_index++;            //move to next cluster pointer in array
    } 
    out:
    if(res < 0){
        return 0;
    }
    //return number of bytes that's been read
    return block_size * block_num - left_to_read;
}

int32_t fat16_read(void* buf, size_t block_size, size_t n_memb, struct file_descriptor* f_desc){
    //wrapper function for read_internal function
    //called by higer level function from VFS fread 
    int32_t bytes_read = 0;
    int32_t res = 0;
    //check if arguments are valid
    if(block_size < 0 || n_memb < 0){
        res = -EINVARG;
        goto out;
    }
    if(f_desc == NULL || buf == NULL){
        res = -EINVARG;
        goto out;
    }
    struct fat_item_descriptor* fat_desc = (struct fat_item_descriptor*)(f_desc->private);
    //call read_internal which will do most of the job
    bytes_read = read_internal(f_desc->disk, fat_desc->item, buf, fat_desc->pos, block_size, n_memb);
    fat_desc->pos += bytes_read;        //move file position bytes_read bytes forward
    out: 
    if(res < 0){
        return 0;
    }
    //return number of bytes that's been read
    return bytes_read;
}

void* fat16_open(struct disk* disk, struct path_dir* path, FILE_MODE mode){
    //called from upper level function from VFS fopen
    int32_t res = 0;
    struct fat_item_descriptor* f_desc = NULL;
    struct fat_directory_item* items_in_dir = NULL;
    void* file_data = NULL;
    if(mode == FILE_MODE_INVALID){ //invalid mode 
        res = -EINVARG;
        return NULL;
    }
    struct fat_private* fat_private = disk -> fs_private;
    //get attributes of root directory 
    struct fat_directory* root_dir = &(fat_private->root_directory);
    int32_t entries_num = root_dir->total; 
    int32_t all_entries_size = entries_num * sizeof(struct fat_directory_item);
    items_in_dir = kzalloc(all_entries_size); //holds all item in root
    if(items_in_dir == NULL){
        res = -ENOMEM;
        return NULL;
    } 
    //all items in root dir will be loaded to items_in_dir
    //get disk position of root dir and read its items
    uint32_t item_disk_pos = root_dir->sector_pos * disk->sectors_size;
    set_disk_pos(fat_private->directory_stream, item_disk_pos);
    if(read_bytes_from_disk(fat_private->directory_stream, items_in_dir,all_entries_size) != ALL_OK){
        res = -EIO;
        return NULL;    
    }//read root dir 
    struct path_dir* cur_dir = path;    //holds part of file path
    int32_t i = 0;                      //used as counter in further loop
    struct fat_directory_item* cur_item = NULL;     
    f_desc = kzalloc(sizeof(struct fat_item_descriptor));   //descriptor for file to be opened,will be used after
    if(f_desc == NULL){
        res = -ENOMEM;
        goto out;
    }
    f_desc->upper_subdir = kzalloc(sizeof(struct fat_item_descriptor)); //descriptor of subdir of file to be opened,will be used after
    if(f_desc->upper_subdir == NULL){
        res = -ENOMEM;
        goto out;
    }
    while(cur_dir != NULL){
        //iterate over path parts in file path and look for items in subdirs
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
                //else create new file for writing mode
                cur_item = kzalloc(sizeof(struct fat_directory_item));  //allocate mem for new file entry
                if(cur_item == NULL){
                    res = -ENOMEM;
                    goto out;
                }
                //create new file via writing filling file entry attributes and writing it to apropriate place
                //on the disk
                if(fat16_create_file(cur_dir->dir_name, cur_item, item_disk_pos, entries_num, fat_private) < 0){
                    res = -EIO;
                    goto out;
                }
            }
        }else{
            //item is found, already exists on the disk
            cur_item = &items_in_dir[i];    //point to found item
            if(!is_valid_path_part(cur_dir, &items_in_dir[i])){//check path
                res = -EBADPATH;
                goto out;
            }
        }
        if(cur_item->attribute & FAT16_FILE_SUBDIRECTORY){ //if part is subdirectory
            kfree(file_data);           //free old file data and read new one
            file_data = read_directory_item_entries(disk, &items_in_dir[i], &item_disk_pos); //read data from found entry
            if(file_data == NULL){
                res = -EIO;
                goto out;
            }
            //save file entry of curret subdir, last one will be the upper dir of opened file 
            memcpy(f_desc->upper_subdir, &items_in_dir[i], sizeof(struct fat_directory_item));
            //index of file in subdir
            f_desc->index_in_subdir = i - 1;
            //get num of file entries in current subdir
            entries_num = items_in_dir[i].filesize / sizeof(struct fat_directory_item);
            //check if something misses
            //check this
            kfree(items_in_dir); //free old entries
            items_in_dir = (struct fat_directory_item*) file_data; //than data represent new entries
            //check more   
        }else{          //file is reached
            //save entry of file to fat_desc
            f_desc->item = kzalloc(sizeof(struct fat_directory_item));
            memcpy(f_desc->item, cur_item, sizeof(struct fat_directory_item));
            //set file pos to 0
            f_desc->pos = 0;
            if(mode == FILE_MODE_APPEND){
                //if mode is append set file pos to end of the file
                f_desc->pos = cur_item->filesize;
            }
        }
        cur_dir = cur_dir->next; //move to next part in list
    }
    out:
    //free 
    kfree(items_in_dir);
    kfree(file_data);
    kfree(cur_item); //check this for error
    if(res < 0){
        //free descriptor if error
        kfree(f_desc->item);
        kfree(f_desc->upper_subdir);
        kfree(f_desc);
        return NULL;
    }
    return f_desc;  //return descriptor
}



int32_t fat16_get_root_directory(struct disk* disk, struct fat_private* fat_private, struct fat_directory* root_dir){
    //set 
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
    //read root directory
    struct disk_streamer* stream = fat_private->directory_stream;
    set_disk_pos(stream, start_sector_of_root_dir * disk->sectors_size);
    if(read_bytes_from_disk(stream, dir, root_dir_size) != ALL_OK){
        res = -EIO;
        goto end;
    }
    //set all atributes
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
    //called by VFS function fs_resolve to check is the disk filesystem FAT16
    int32_t res = 0;
    //setting fat_private structure
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
        //Checking FAT16 boot signature
        res = -EFSNOTUS;
        goto out;
    }

    if(fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != ALL_OK){
        //check root directory
        res = -EIO;
        goto out;
    }

    out:
    if(stream){
        delete_streamer(stream);
    }
    if(res < 0){
        kfree(fat_private);
        //if error disk's filesystem isn't FAT16
        disk->fs_private = 0;
        disk->filesystem = 0;
    }
    return res;
}

int32_t fat16_seek(struct file_descriptor* f_desc, int32_t offset, int32_t pos){
    //called by VFS function fseek if FAT16 is filesystem
    //used to set position in file
    int32_t res = 0;
    struct fat_item_descriptor* fat_desc = (struct fat_item_descriptor*)f_desc->private;
    uint32_t f_size = fat_desc->item->filesize;
    switch (pos){
        //pos has 3 modes 
        //1)SEEK_SET for start of the file, 
        //2)SEEK_CUR for current pos
        //3)SEEK_END for end of the file
    case SEEK_SET:
        if(offset < 0){
            //if it's start of the file offset can not be negative
            res = -EINVARG;
            goto out;
        }
        fat_desc->pos = offset;
        break;

    case SEEK_CUR:
        if(offset + (int32_t)fat_desc->pos < 0 || offset + (int32_t)fat_desc->pos > f_size){
            //offset must be in bound betwen 0 and file size
            res = -EINVARG;
            goto out;
        }
        fat_desc ->pos += offset;   //add offset to old value
        break;

    case SEEK_END:
        if(offset > 0){
            //if it's end of the file offset can not be positive
            res = -EINVARG;
            goto out;
        }
        fat_desc ->pos = f_size + offset;   //set pos relative to f_size
        break;

    default:
        res = -EINVARG;
        break;
    }
    out:
    return res;
}

int32_t fat16_stat(struct file_descriptor* f_desc, struct file_stats* f_stats){
    //called by VFS function if file system is FAT16
    //fill f_stats structure with info about file 
    struct fat_item_descriptor* fat_desc = (struct fat_item_descriptor*)f_desc->private;
    struct fat_directory_item* file_entry = fat_desc->item;
    f_stats->user = NULL; //not implemented yet
    //set file size
    f_stats->file_size = (int32_t)file_entry->filesize;
    //set date and time of creation, last access and last modification
    //format of saving those dates and times can be found here https://wiki.osdev.org/FAT
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
    //called by VFS function fclose if filesystem is FAT16, free all allocations of the file
    int32_t res;
    struct fat_item_descriptor* fat_desc = (struct fat_item_descriptor*)f_desc->private;
    //update access date and time
    //first implement time and date menagement in system
    //... 
    //write file_item_entry to disk

    //get cluster of upper_subdir
    uint32_t cluster_index = get_cluster_position(fat_desc->upper_subdir->high_16_bits_first_cluster, fat_desc->upper_subdir->low_16_bits_first_cluster);
    struct fat_private* fat_private = (struct fat_private*)f_desc->disk->fs_private;
    uint32_t data_start_pos = fat_private->root_directory.ending_sector_pos;
    uint32_t file_entry_disk_pos = (data_start_pos + (cluster_index - 2) 
    * fat_private->header.primary_header.sectrors_per_cluster) 
    * f_desc->disk->sectors_size;
    //calculate where to save updated file entry
    file_entry_disk_pos += fat_desc->index_in_subdir * sizeof(struct fat_directory_item);
    set_disk_pos(fat_private->directory_stream, file_entry_disk_pos);
    //write updated item_entry to disk
    if(write_bytes_to_disk(fat_private->directory_stream, fat_desc->item, sizeof(struct fat_directory_item)) != ALL_OK){
        res = -EIO;
        goto out;    
    }
    //free all descriptor allocations as well as descriptor
    kfree(fat_desc->upper_subdir);
    kfree(fat_desc->item);
    kfree(fat_desc);
    kfree(f_desc);
    out:
    return res;
}


int32_t write_internal(struct disk* disk, struct fat_directory_item* file_item, void* buf, uint32_t file_pos, size_t n_memb, size_t block_size){
    //called by wrapper function fat16_write
    //writes to file which entry is file_item on the disk disk from buf to file_pos file position
    int32_t bytes_written = 0;
    int32_t res = 0;
    //some basic error check
    if(n_memb < 0 || block_size < 0){
        res = -EINVARG;
        goto out;
    }
    if(disk == NULL || file_item == NULL || buf == NULL){
        res = -EINVARG;
        goto out;
    }
    if(file_item->attribute & FAT16_FILE_SUBDIRECTORY){//if file is subdirectory
        res = -EINVARG;
        goto out;
    }
    int32_t bytes_to_write = n_memb * block_size;
    struct fat_private* fat_private = disk->fs_private;
    struct fat16_header* prim_header = &fat_private->header.primary_header;
    uint32_t f_size = file_item->filesize;
    int32_t cluster_sector_size = prim_header->sectrors_per_cluster;
    int32_t cluster_size =  cluster_sector_size * disk->sectors_size;
    uint32_t cluster_pos = get_cluster_position(file_item->high_16_bits_first_cluster, file_item->low_16_bits_first_cluster);
    uint32_t data_clusters_start_pos = fat_private->root_directory.ending_sector_pos;
    uint32_t num_of_clusters = f_size / cluster_size;
    if(f_size % cluster_size || f_size == 0){
        //if file is empty it still has 1 cluster
        num_of_clusters++;
    }
    //get array of all file cluster indexes
    FAT_ENTRY* cluster_pointer = get_cluster_indexes(fat_private, file_item);
    if(cluster_pointer == NULL){
        res = -EIO;
        goto out;
    }
    //same as reading
    uint32_t left_to_write = bytes_to_write;
    int32_t writing = cluster_size;                     //writing one cluster at iteration  
    int32_t cluster_index = file_pos / cluster_size;    
    int32_t cluster_offset = file_pos % cluster_size;
    void* buf_c = buf;
    FAT_ENTRY cur_cluster_pointer;
    while(left_to_write > 0){
        //loop while there is no more bytes to write
        if(cluster_index >= num_of_clusters){//fs need more clusters to write
            //allocate new cluster for writing
            FAT_ENTRY new_cluster_ind;              //index of new allocated cluster
            res = find_free_cluster(fat_private, (int32_t*)&new_cluster_ind);//gets free cluster
            if(res < 0){
                goto out;
            }
            uint32_t fat_entry_disk_pos = prim_header->reserved_sectors * MY_OS_DISK_SECTOR_SIZE + (cur_cluster_pointer + 2) * sizeof(FAT_ENTRY); 
            for(int32_t i = 0; i < prim_header->fat_copies; i++){
                //seting last fat entry to point to next one in all FAT copies
                set_disk_pos(fat_private->fat_read_stream, fat_entry_disk_pos);
                if(write_bytes_to_disk(fat_private->fat_read_stream, &new_cluster_ind, sizeof(FAT_ENTRY)) != ALL_OK){
                    res = -EIO;
                    goto out;
                }
                fat_entry_disk_pos += prim_header->sectrors_per_fat * MY_OS_DISK_SECTOR_SIZE;
            }
            cur_cluster_pointer = new_cluster_ind; //set cur cluster pointer to new one
        }else{
            cur_cluster_pointer = cluster_pointer[cluster_index]-2;
            cluster_index++;                //move to next cluster index in array
        }
        cluster_pos = data_clusters_start_pos + cur_cluster_pointer * cluster_sector_size;
        set_disk_pos(fat_private->cluster_read_stream, cluster_pos * disk->sectors_size + cluster_offset);
        if(left_to_write < cluster_size){ //reaching last cluster for writing
            writing = left_to_write; 
        }
        if((cluster_offset != 0) && (cluster_offset + writing > cluster_size)){
            writing = cluster_size - cluster_offset;    //staying in bound of a cluster when writing
        }
        //write data from buf_c
        if(write_bytes_to_disk(fat_private->cluster_read_stream, buf_c, writing) != ALL_OK){
            //writing one cluster at the time
            res = -EIO;
            goto out;
        }
        cluster_offset = 0;//every other cluster to write will be written from the start
        buf_c += writing;   //move to the end of already written data in the buffer
        left_to_write -= writing;   //update left to write
    }
    if(cluster_index < num_of_clusters){//there are unused clusters that could be freed
        uint32_t fat_entry_disk_pos = prim_header->reserved_sectors * MY_OS_DISK_SECTOR_SIZE + cluster_pointer[cluster_index - 1] * sizeof(FAT_ENTRY); 
        FAT_ENTRY last_used_cluster_fat_entry = 0xFFFF;
        for(int32_t i = 0; i < prim_header->fat_copies; i++){
            //seting last used cluster entry as last
            set_disk_pos(fat_private->fat_read_stream, fat_entry_disk_pos);
            if(write_bytes_to_disk(fat_private->fat_read_stream, &last_used_cluster_fat_entry, sizeof(FAT_ENTRY)) != ALL_OK){
                res = -EIO;
                goto out;
            }
            fat_entry_disk_pos += prim_header->sectrors_per_fat * MY_OS_DISK_SECTOR_SIZE;
        }
        while(cluster_index < num_of_clusters){
            //setting all other clusters as unused
            fat_entry_disk_pos = prim_header->reserved_sectors * MY_OS_DISK_SECTOR_SIZE + cluster_pointer[cluster_index] * sizeof(FAT_ENTRY); 
            FAT_ENTRY unused_cluster_fat_entry = 0;
            for(int32_t i = 0; i < prim_header->fat_copies; i++){
                //seting all left fat_entry as unused in all fat_copies
                set_disk_pos(fat_private->fat_read_stream, fat_entry_disk_pos);
                if(write_bytes_to_disk(fat_private->fat_read_stream, &unused_cluster_fat_entry, sizeof(FAT_ENTRY)) != ALL_OK){
                    res = -EIO;
                    goto out;
                }
                fat_entry_disk_pos += prim_header->sectrors_per_fat * MY_OS_DISK_SECTOR_SIZE;
            }
            cluster_index++;    //move to next cluster pointer
        }
        
    } 



    out:
    if(res < 0){
        //!!!
        //free ...
        //look what need to be freed
        //!!!
        return 0;
    }
    bytes_written = bytes_to_write - left_to_write;
    if(file_item->filesize < bytes_written + file_pos){     //if it is written over the size of the file
        file_item->filesize = bytes_written + file_pos;     //update the size
    }
    return bytes_written; //return number of written bytes
}

int32_t fat16_write(void* buf, size_t block_size, size_t n_memb, struct file_descriptor* f_desc){
    //called by VFS function fwrite if FAT16 is filesystem
    //rely on write_internal function
    int32_t bytes_written = 0;
    int32_t res = 0;
    //check arguments 
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
    fat_desc->pos += bytes_written; //move file position by bytes_written bytes forward
    out: 
    if(res < 0){
        return 0;
    }
    return bytes_written;   //return number of written bytes

}








