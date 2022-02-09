#ifndef TASK_H
#define TASK_H

//task functionality

#include "config.h"
#include "paging.h"
#include <stdint.h>
#include "process.h"


//used to save states of all registers
struct registers
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
}__attribute__((packed));


//structure that holds all necesserry info about task
struct task
{
    /**
     * The page directory of the task
     */
    struct paging_4gb_chunk* page_directory;

    // The registers of the task when the task is not running
    struct registers registers;

    //Process of the task
    struct process* process;

    // The next task in the linked list
    struct task* next;

    // Previous task in the linked list
    struct task* prev;
};

struct task* task_new(struct process* process);
struct task* task_current();
struct task* task_get_next();
int task_free(struct task* task);

void restore_general_purpose_registers(struct registers* regs);
void task_return(struct registers* regs);
void user_registers();

int32_t task_page();
int32_t task_switch(struct task* task);
void task_run_first_task_ever();



#endif