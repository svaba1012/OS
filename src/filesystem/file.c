#include "file.h"
#include "config.h"
#include "memory.h"
#include "terminal.h"
#include "status.h"
#include "kheap.h"
#include "string.h"
#include "fat16.h"
#include "pathparser.h"

//virtual filesystem (VFS) layer of OS
//VFS holds wrapper function which are firts called and deppending of filesystems of the disk, 
//calls certain function of that filesystem
//In this way it's possible to have support for more fs

struct filesystem* filesystems[MY_OS_MAX_FILESYSTEMS];  //holds filesystems that os support(currently only FAT16)
struct file_descriptor* file_descriptors[MY_OS_MAX_FILE_DESCRIPTORS];   //holds info about all opened files


struct filesystem** get_free_filesystem(){
    //return free slot in filesystem array
    for(int32_t i = 0; i < MY_OS_MAX_FILESYSTEMS; i++){
        if(filesystems[i] == NULL){
            return &filesystems[i]; 
        } 
    }
    return NULL;
}

void insert_filesystem(struct filesystem* new_fs){
    //insert new filesystem
    struct filesystem** fs;
    fs = get_free_filesystem(); 
    if(fs == NULL){
        panic("No more space for filesystems");       
    }
    *fs = new_fs;
}

void filesystem_init(){ //set filesystem functionality that os can recognise
    memset(filesystems, 0, MY_OS_MAX_FILESYSTEMS);
    insert_filesystem(fat16_init());    //currently only FAT16
    memset(file_descriptors, 0, MY_OS_MAX_FILE_DESCRIPTORS);
}

int32_t get_new_file_descriptor(struct file_descriptor** desc_out){
    //search for free file descriptor
    for(int32_t i = 0; i < MY_OS_MAX_FILE_DESCRIPTORS; i++){
        if(file_descriptors[i] == NULL){
            struct file_descriptor* desc = kzalloc(sizeof(struct file_descriptor));
            //descriptor num start with 1
            desc->index = i+1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            return 0; 
        }
    }
    return -EINVARG;
}

struct file_descriptor* get_descriptor_by_fd(int32_t fd){
    //get file desc by fd index num(always + 1 on index) 
    if(fd <= 0 && fd >= MY_OS_MAX_FILE_DESCRIPTORS){
        return NULL;
    }
    return file_descriptors[fd - 1];
}


struct filesystem* fs_resolve(struct disk* disk){
    //look in disk for file signatures and compares it with familiar filesystem
    //if it finds corespondance return filesystem of disk  
    for(int32_t i = 0; i < MY_OS_MAX_FILESYSTEMS; i++){
        if(filesystems[i] != NULL && filesystems[i]->resolve(disk) == 0){
            return filesystems[i];
        }
    }
    return NULL;
}

int32_t char_file_mode_to_int(char* mode){
    //change file open mode from string to int 
    if(strncmp(mode, "r", 1) == 0){
        return FILE_MODE_READ;
    }else if(strncmp(mode, "w", 1) == 0){
        return FILE_MODE_WRITE;
    }else if(strncmp(mode, "a", 1) == 0){
        return FILE_MODE_APPEND;
    }
    return FILE_MODE_INVALID;
}

int32_t fopen(char* path_name, char* mode_str){
    //opening the file and prepering it to be easly accessable
    FILE_MODE mode = char_file_mode_to_int(mode_str);
    if(mode == FILE_MODE_INVALID){
        return 0;
    }
    struct path_root* drive_name = path_parse(path_name);//making linked list of directory names;
    if(drive_name == NULL){
        return 0;
    }
    if(drive_name->drive_num != 0){//only support primary disk
        return 0;
    }
    struct path_dir* first_dir = drive_name->first;
    if(first_dir == NULL){//checking if first directory or file exists
        return 0;
    }
    struct disk* disk = get_disk_by_index(drive_name->drive_num);
    struct filesystem* fs = disk->filesystem;
    struct file_descriptor* free_file_desc;
    if(get_new_file_descriptor(&free_file_desc) < 0){
        return 0;
    }
    void* descriptor_private_data = fs->open(disk, first_dir, mode); //calls open function of fs from the disk
    free_file_desc ->fs = disk->filesystem; 
    free_file_desc ->disk = disk;
    free_file_desc ->private = descriptor_private_data;
    
    return free_file_desc->index; //return index of file desc
}

int32_t fread(void* buf, size_t block_size, int32_t num_of_blocks, uint32_t fd){
    int32_t num_of_bytes;
    int32_t res = 0;
    if(fd < 1){
        res = -EINVARG;
        goto out;
    }
    struct file_descriptor* file_desc = file_descriptors[fd-1];
    //calls read function of fs from the disk
    num_of_bytes = file_desc->disk->filesystem->read(buf, block_size, num_of_blocks, file_desc);
    out:
    if(res < 0){
        return 0;
    }
    return num_of_bytes;
}

int32_t fseek(uint32_t fd, int32_t offset, int32_t pos){
    //set position in the file
    int32_t res = 0;
    if(fd < 1){
        res = -EINVARG;
        goto out;
    }
    struct file_descriptor* f_desc = file_descriptors[fd - 1];    
     //calls seek function of fs from the disk
    res = f_desc->disk->filesystem->seek(f_desc, offset, pos);
    out:
    return res;
}

int32_t fstat(uint32_t fd, struct file_stats* f_stats){
    //get some stats from file
    int32_t res = 0;
    if(fd < 1){
        res = -EINVARG;
        goto out;
    }
    struct file_descriptor* f_desc = file_descriptors[fd - 1];    
     //calls stat function of fs from the disk
    res = f_desc->disk->filesystem->stat(f_desc, f_stats);
    out:
    return res;

}

int32_t fclose(uint32_t fd){
    //close file, free all allocated memory used by that file
    int32_t res = 0;
    if(fd < 1){
        res = -EINVARG;
        goto out;
    }
    struct file_descriptor* f_desc = file_descriptors[fd - 1];    
     //calls close function of fs from the disk
    res = f_desc->disk->filesystem->close(f_desc);
    kfree(f_desc);
    file_descriptors[fd - 1] = NULL;
    out:
    return res;

}

int32_t fwrite(void* buf, size_t block_size, int32_t num_of_blocks, uint32_t fd){
    int32_t num_of_bytes;
    int32_t res = 0;
    if(fd < 1){
        res = -EINVARG;
        goto out;
    }
    struct file_descriptor* file_desc = file_descriptors[fd-1];
     //calls write function of fs from the disk
    num_of_bytes = file_desc->disk->filesystem->write(buf, block_size, num_of_blocks, file_desc);
    out:
    if(res < 0){
        return 0;
    }
    return num_of_bytes;
}