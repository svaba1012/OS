#include "file.h"
#include "config.h"
#include "memory.h"
#include "terminal.h"
#include "status.h"
#include "kheap.h"
#include "string.h"
#include "fat16.h"
#include "pathparser.h"

struct filesystem* filesystems[MY_OS_MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[MY_OS_MAX_FILE_DESCRIPTORS];


struct filesystem** get_free_filesystem(){
    for(int32_t i = 0; i < MY_OS_MAX_FILESYSTEMS; i++){
        if(filesystems[i] == NULL){
            return &filesystems[i]; 
        } 
    }
    return NULL;
}

void insert_filesystem(struct filesystem* new_fs){
    struct filesystem** fs;
    fs = get_free_filesystem();
    if(fs == NULL){
        print("No more space for filesystems");
        while(1){
            ;
        }        
    }
    *fs = new_fs;
}

void filesystem_init(){
    memset(filesystems, 0, MY_OS_MAX_FILESYSTEMS);
    insert_filesystem(fat16_init());
    memset(file_descriptors, 0, MY_OS_MAX_FILE_DESCRIPTORS);
}

int32_t get_new_file_descriptor(struct file_descriptor** desc_out){
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
    if(fd <= 0 && fd >= MY_OS_MAX_FILE_DESCRIPTORS){
        return NULL;
    }
    return file_descriptors[fd - 1];
}


struct filesystem* fs_resolve(struct disk* disk){
    for(int32_t i = 0; i < MY_OS_MAX_FILESYSTEMS; i++){
        if(filesystems[i] != NULL && filesystems[i]->resolve(disk) == 0){
            return filesystems[i];
        }
    }
    return NULL;
}

int32_t char_file_mode_to_int(char* mode){
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
    FILE_MODE mode = char_file_mode_to_int(mode_str);
    if(mode == FILE_MODE_INVALID){
        return 0;
    }
    struct path_root* drive_name = path_parse(path_name);//making linked list of directory names;
    if(drive_name == NULL){
        return 0;
    }
    if(drive_name->drive_num != 0){//check this
        return 0;
    }
    struct path_dir* first_dir = drive_name->first;
    if(first_dir == NULL){//checking if first exist
        return 0;
    }
    struct disk* disk = get_disk_by_index(drive_name->drive_num);
    struct filesystem* fs = disk->filesystem;
    struct file_descriptor* free_file_desc;
    if(get_new_file_descriptor(&free_file_desc) < 0){
        return 0;
    }
    void* descriptor_private_data = fs->open(disk, first_dir, mode);
    free_file_desc ->fs = disk->filesystem;
    free_file_desc ->disk = disk;
    free_file_desc ->private = descriptor_private_data;
    
    return free_file_desc->index;
}

int32_t fread(void* buf, size_t block_size, int32_t num_of_blocks, uint32_t fd){
    int32_t num_of_bytes;
    int32_t res = 0;
    if(fd < 1){
        res = -EINVARG;
        goto out;
    }
    struct file_descriptor* file_desc = file_descriptors[fd-1];
    num_of_bytes = file_desc->disk->filesystem->read(buf, block_size, num_of_blocks, file_desc);
    out:
    if(res < 0){
        return 0;
    }
    return num_of_bytes;
}

int32_t fseek(uint32_t fd, int32_t offset, int32_t pos){
    int32_t res = 0;
    if(fd < 1){
        res = -EINVARG;
        goto out;
    }
    struct file_descriptor* f_desc = file_descriptors[fd - 1];    
    res = f_desc->disk->filesystem->seek(f_desc, offset, pos);
    out:
    return res;
}

int32_t fstat(uint32_t fd, struct file_stats* f_stats){
    int32_t res = 0;
    if(fd < 1){
        res = -EINVARG;
        goto out;
    }
    struct file_descriptor* f_desc = file_descriptors[fd - 1];    
    res = f_desc->disk->filesystem->stat(f_desc, f_stats);
    out:
    return res;

}

int32_t fclose(uint32_t fd){
    int32_t res = 0;
    if(fd < 1){
        res = -EINVARG;
        goto out;
    }
    struct file_descriptor* f_desc = file_descriptors[fd - 1];    
    res = f_desc->disk->filesystem->close(f_desc);
    kfree(f_desc);
    file_descriptors[fd - 1] = NULL;
    out:
    return res;

}