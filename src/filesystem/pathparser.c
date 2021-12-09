#include "pathparser.h"
#include "stdbool.h"
#include "string.h"
#include "config.h"
#include "kheap.h"
#include "terminal.h"
#include "memory.h"

#include <stddef.h>


bool is_valid_path(char* path_name){
    uint32_t len = strnlen(path_name, MY_OS_PATH_MAX_LEN);
   //char start_str[3] = ":/";
    return (len >= 3) && is_numeric(path_name[0]) && (memcmp(path_name+1, ":/", 2) == 0) \
     && (strlen(path_name) < MY_OS_PATH_MAX_LEN);
}

struct path_root* path_parse(char* path_name){
    //format of path_name should be "0:/dir1/dir2/.../file.bin"
    char* tmp_path_name = path_name;
    if(!is_valid_path(path_name)){
        //print("Nope");
        return NULL;
    }

    struct path_root* root;
    root = (struct path_root*)kzalloc(20);
    
    //print(tmp_path_name);
    //print("\n");

    if(root == NULL){
        print("No more memory");
        while (1){
            ;
        }
    } 
    root->drive_num = char_to_numeric(path_name[0]); //seting drive_num, should be first char of path_name if it is valid
    tmp_path_name += 3;//skiping first 3 char 0:/
    
    root->first = kzalloc(sizeof(struct path_dir));
    struct path_dir* qur = root->first;
    if(qur == NULL){
        print("No more memory");
        while (1){
            ;
        }
    }
    qur->dir_name = kzalloc(MY_OS_PATH_MAX_LEN);
    char* cur_name = qur->dir_name;
    if(cur_name == NULL){
        print("No more memory");
        while (1){
            ;
        }
    }
    while(*tmp_path_name){
        if(*tmp_path_name == '/'){
            *cur_name = '\0';
            //print(qur->dir_name);
            //print("\n");
            qur->next = kzalloc(sizeof(struct path_dir));
            qur = qur -> next;
            qur->dir_name = kzalloc(MY_OS_PATH_MAX_LEN);
            cur_name = qur->dir_name;
            tmp_path_name++;
            
        }
        *cur_name=*tmp_path_name;
        cur_name++;
        tmp_path_name++;
    }
    *cur_name = '\0';
    //print(qur->dir_name);
    qur->next = NULL;
    return root;
}

void free_path_parser(struct path_root* root){
    struct path_dir* qur = root -> first;
    struct path_dir* next;
    kfree(root);
    while(qur != NULL){
        kfree(qur->dir_name);
        next = qur -> next;
        kfree(qur);
        qur = next;
    }
}