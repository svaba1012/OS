#ifndef PATHPARSER_H
#define PATHPARSER_H
#include <stdint.h>

/*functionality to parse file path from string to linked lists of path parts*/

//structure that describes on which disk is file
struct path_root{
    uint32_t drive_num;
    struct path_dir* first;
};

//one node in linked list of partss
struct path_dir{
    char* dir_name;
    struct path_dir* next;
};

struct path_root* path_parse(char* path_name);
void free_path_parser(struct path_root* root);


#endif