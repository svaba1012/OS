#include "file.h"
#include "config.h"
#include "memory.h"
#include "terminal.h"
#include "status.h"
#include "kheap.h"
#include "fat16.h"

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
int32_t fopen(char* path_name, char* mode){
    return -EIO;
}
