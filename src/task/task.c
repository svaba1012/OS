#include "task.h"
#include "kheap.h"
#include "paging.h"
#include "status.h"
#include "terminal.h"


/*Defines some functions for tasks. 
All existing tasks are held in linked list.
*/

struct task* task_head = NULL;
struct task* task_tail = NULL;
struct task* current_task = NULL;

struct task* task_new(struct process* process){
    //allocate resources for new task and put that task in the list
    int32_t res = 0;
    struct task* new_task = kzalloc(sizeof(struct task));
    if(new_task == NULL){
        res = -ENOMEM;
        return NULL;
    }
    //set new pagging directory for the task
    new_task ->page_directory = set_4gb_chunk(PAGING_PAGE_PRESENT | PAGING_PAGE_ACCESS_FOR_ALL);
    if(new_task->page_directory == NULL){
        res = -ENOMEM;
        kfree(new_task);
        return NULL;
    }
    //putting task into list
    if(task_head == NULL){  //if list is empty
        task_head = new_task;
        current_task = new_task;
        new_task ->prev = NULL;
    }else{
        task_tail ->next = new_task;
        new_task ->prev = task_tail;
    }
    task_tail = new_task;
    new_task ->next = NULL;
    //seting all other task atributes
    new_task ->process = process;
    new_task->registers.ip = MY_OS_PROGRAM_VIRTUAL_ADDRESS;
    new_task->registers.esp = MY_OS_USER_STACK_VIRTUAL_ADDRESS; //check where to put the stack
    new_task->registers.cs = USER_CODE_SEG_SELECTOR;
    new_task->registers.ss = USER_DATA_SEG_SELECTOR;
    //see what more need to be set
    if(res < 0){
        //see what need to be freed
        return NULL;
    }
    return new_task;
}
struct task* task_current(){
    return current_task;
}
struct task* task_get_next(){
    if(current_task -> next == NULL ){
        return task_head;
    }
    return current_task->next;
}



int task_free(struct task* task){
    //delete task from the list
    if(task->prev != NULL){
        task->prev->next = task->next;
    }else{
        task_head = task->next;
    }
    if(task->next == NULL){
        task_tail = task->prev;
    }
    if(task == task_head){
        task_head = task -> next;
    }
    if(current_task == task){
        current_task = task->next;
        if(current_task == NULL){
            current_task = task_head;
        }
    }
    //free paging table
    pagging_free_4gb_chunk(task->page_directory);
    //free task
    kfree(task);
    return 0;
}

int32_t task_switch(struct task* task){
    //set current task and load task pagging directory 
    current_task = task;
    paging_switch(task->page_directory->directory_entry);
    return 0;
}

int32_t task_page(){
    //load task pagging directory and set segment registers to user descriptors
    user_registers();
    task_switch(current_task);
    return 0;
}

void task_run_first_task_ever(){
    if(current_task == NULL){
        panic("task_run_first_task_ever():No current task");
    }
    task_switch(task_head);//set segment regs and pagging directory for task_head to be run 
    task_return(&task_head->registers);//jump to user land
}