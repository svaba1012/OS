#include "pathparser.h"
#include "stdbool.h"
#include "string.h"
#include "config.h"
#include "kheap.h"
#include "terminal.h"
#include "memory.h"

#include <stddef.h>

/*provide functionalyty to make link list of file/directory names of some file path
seperates parts of path name and put then in linked list
for eaxample: 0:/dir1/dir2/file.txt forms  0:/--->dir1--->dir2--->file.txt
*/


bool is_valid_path(char* path_name){
    //path is valid if it has format 0:/dir1/dir2/.../file.txt
    uint32_t len = strnlen(path_name, MY_OS_PATH_MAX_LEN);
   //char start_str[3] = ":/";
    return (len >= 3) && is_numeric(path_name[0]) && (memcmp(path_name+1, ":/", 2) == 0) \
     && (strlen(path_name) < MY_OS_PATH_MAX_LEN);
}

struct path_root* path_parse(char* path_name){
    //format of path_name should be "0:/dir1/dir2/.../file.bin"
    char* tmp_path_name = path_name;
    if(!is_valid_path(path_name)){
        return NULL;
    }

    struct path_root* root;
    root = (struct path_root*)kzalloc(sizeof(struct path_root));

    if(root == NULL){
        panic("PathParser:No more memory");
    } 
    root->drive_num = char_to_numeric(path_name[0]); //seting drive_num, should be first char of path_name if it is valid
    tmp_path_name += 3;//skiping first 3 char 0:/
    
    root->first = kzalloc(sizeof(struct path_dir));
    struct path_dir* qur = root->first;
    if(qur == NULL){
        panic("PathParser:No more memory");
    }
    qur->dir_name = kzalloc(MY_OS_PATH_MAX_LEN);
    char* cur_name = qur->dir_name;
    if(cur_name == NULL){
        panic("PathParser:No more memory");
    }
    while(*tmp_path_name){
        if(*tmp_path_name == '/'){ //end of path part name reached
            *cur_name = '\0';       //close cur_name string
            //allocate memory for next part in linked list and link it
            qur->next = kzalloc(sizeof(struct path_dir));
            qur = qur -> next;
            qur->dir_name = kzalloc(MY_OS_PATH_MAX_LEN);
            cur_name = qur->dir_name;
            tmp_path_name++;
            
        }
        *cur_name=*tmp_path_name;   //copy tmp_path_name to cur_name till reaching /
        cur_name++;
        tmp_path_name++;
    }
    *cur_name = '\0'; //close last cur_name str
    qur->next = NULL; //last points to nothing
    return root;
}

void free_path_parser(struct path_root* root){
    //free whole linked list of path parts
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