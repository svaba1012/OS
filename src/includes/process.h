#ifndef PROCESS_H
#define PROCESS_H

#include "config.h"
#include "task.h"
#include <stdint.h>

enum{BIN, ELF}; //type of exe file

struct process{
    uint16_t id;
    char filename[MY_OS_PATH_MAX_LEN];
    struct task* main_task;
    //saved ptr to all process allocations
    void* allocations[MY_OS_MAX_USER_ALLOCATIONS];

    //physical memory to process code
    void* code_ptr;
    //physical memory to process stack
    void* stack_ptr;
    //physical memory to process data
    void* data_ptr;
    //type of exe file
    uint8_t exe_type;
    //size of process memory
    uint32_t size;

}; 

int32_t process_load(char* filename, struct process** processes);

#endif