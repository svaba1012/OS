#include "task.h"
#include "kheap.h"
#include "paging.h"
#include "status.h"

struct task* task_head = NULL;
struct task* task_tail = NULL;
struct task* current_task = NULL;

struct task* task_new(){
    int32_t res = 0;
    struct task* new_task = kzalloc(sizeof(struct task));
    if(new_task == NULL){
        res = -ENOMEM;
        return NULL;
    }
    new_task ->page_directory = set_4gb_chunk(PAGING_PAGE_PRESENT | PAGING_PAGE_ACCESS_FOR_ALL);
    if(new_task->page_directory == NULL){
        res = -ENOMEM;
        kfree(new_task);
        return NULL;
    }
    if(task_head == NULL){
        task_head = new_task;
        new_task ->prev = NULL;
    }else{
        task_tail ->next = new_task;
        new_task ->prev = task_tail;
    }
    task_tail = new_task;
    new_task ->next = NULL;
    new_task->registers.ip = MY_OS_PROGRAM_VIRTUAL_ADDRESS;
    new_task->registers.esp = MY_OS_USER_STACK_VIRTUAL_ADDRESS; //check where to put the stack
    new_task->registers.cs = USER_CODE_SEG_SELECTOR;
    new_task->registers.ss = USER_DATA_SEG_SELECTOR;
    //see what more need to be set
    //paging left
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
    if(task_current == task){
        current_task = task->next;
        if(current_task == NULL){
            current_task = task_head;
        }
    }
    //free paging 
    pagging_free_4gb_chunk(task->page_directory);
    kfree(task);
    return 0;
}