#ifndef PATHPARSER_H
#define PATHPARSER_H
#include <stdint.h>


struct path_root{
    uint32_t drive_num;
    struct path_dir* first;
};

struct path_dir{
    char* dir_name;
    struct path_dir* next;
};

struct path_root* path_parse(char* path_name);
void free_path_parser(struct path_root* root);


#endif