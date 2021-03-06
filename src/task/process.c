#include "process.h"
#include "memory.h"
#include "status.h"
#include "kheap.h"
#include "file.h"
#include "string.h"

/*Process functions. All processes are held in array
*/

struct process* current_process = NULL;
static struct process* processes[MY_OS_MAX_PROCESSES];

struct process* get_current_process(){
    return current_process;
}

static void process_init(struct process* process){
    memset(process, 0, sizeof(process));
}

struct process* process_get(int32_t id){
    if(id < 0 || id >= MY_OS_MAX_PROCESSES){
        return NULL;
    }
    return processes[id];
}

static int32_t process_map_memory_binary(struct process* process){
    //change physical addres of virtual addres MY_OS_PROGRAM_VIRTUAL_ADDRESS to process->code_ptr
    //in that way all process have same virtual addres MY_OS_PROGRAM_VIRTUAL_ADDRESS and different physical
    int32_t res = 0;
    res = pagging_map_to(process->main_task->page_directory->directory_entry, MY_OS_PROGRAM_VIRTUAL_ADDRESS, 
    (uint32_t)process->code_ptr, address_page_allign_end((uint32_t)process->code_ptr + process->size),
    PAGING_PAGE_WRITEABLE | PAGING_PAGE_PRESENT | PAGING_PAGE_ACCESS_FOR_ALL);
    return res;
}


int32_t process_map_memory(struct process* process){
    //change physical addreses of virtual adresses for stack and code and data parts of the program
    int32_t res = 0;
    switch (process->exe_type)  //deppending of type of process file mapping is different
    {
    case BIN:       //currently only supprort binary files
        res = process_map_memory_binary(process);
        break;
    default:
        res = -EINVARG;
        break;
    }
    return res;
}

static int32_t load_process_data_binary(uint32_t fd, struct process* process){
    //load process data if file is binary
    int32_t res = 0;
    //get the stats for fd file
    struct file_stats stat;
    void* program_ptr = NULL;
    res = fstat(fd, &stat);
    if(res < 0){
        goto out;
    }
    //get file size
    process->size = stat.file_size;
    //allocate memory for program data to be loaded
    program_ptr = kzalloc(process->size);
    if(program_ptr == NULL){
        res = -ENOMEM;
        goto out;
    }
    //read process data from the file to program_ptr
    //loading process to memory 
    if(fread(program_ptr, 1, process->size, fd) != process->size){
        res = -EIO; 
        goto out;
    }
    //setting code and data ptr to same values
    process->code_ptr = program_ptr;
    process->data_ptr = program_ptr;
    

    out:
    if(res < 0){
        kfree(program_ptr);
    }
    fclose(fd);//closing the process file
    return res;
}

int32_t get_type_of_exe(uint32_t fd){
    return BIN; //for now only support binary format
}

static int32_t load_process_data(char* filename, struct process* process){
    //load process data, map memory and set some process atributes
    int32_t res;
    uint32_t fd = fopen(filename, "r"); //open process file
    void* stack_ptr = NULL;
    struct task* task = NULL;
    if(fd < 1){
        res = -EBADPATH;
        goto out;
    }
    process->exe_type = get_type_of_exe(fd);
    switch (process->exe_type){ //loading of the process depend of file type
    case BIN:       //currently only supports binary format  
        res = load_process_data_binary(fd, process);
        break;
    default:
        res = -EINVARG;
        break;
    }
    if(res < 0){
        goto out;
    }
    //allocate memory for user stack
    stack_ptr = kzalloc(MY_OS_USER_STACK_SIZE);
    if(stack_ptr == NULL){
        res = -ENOMEM;
        goto out;
    }
    //make new (first) task for this process
    task = task_new(process);
    if(task == NULL){
        res = -ENOMEM;
        goto out;
    }
    process->stack_ptr = stack_ptr;
    process->main_task = task;
    strncpy(process->filename, filename, strlen(filename)); //set process name to file name
    res = process_map_memory(process); //map the code and data parts of the program to constant virtual addresses
    res = pagging_map_to(process->main_task->page_directory->directory_entry, MY_OS_USER_STACK_VIRTUAL_ADDRESS - MY_OS_USER_STACK_SIZE,
    (uint32_t)process->stack_ptr, address_page_allign_end((uint32_t)(process->stack_ptr + MY_OS_USER_STACK_SIZE)),
     PAGING_PAGE_ACCESS_FOR_ALL | PAGING_PAGE_WRITEABLE | PAGING_PAGE_PRESENT); //stack grows downwards 
    // maping stack segment of the program to const virt 
    
    out:
    if(res < 0){
        kfree(stack_ptr);
        task_free(task);
        kfree(process->code_ptr);
        process->code_ptr = NULL;
        kfree(process->data_ptr);
        process->data_ptr = NULL;
    }
    return res;
}

int32_t process_load_for_index(char* filename, struct process** process, int32_t index){
    //set struct process atributes, load process and put it into array
    int32_t res = 0;
    struct process* _process = NULL;
    if(index < 0 || index > MY_OS_MAX_PROCESSES){
        res = -EINVARG;
        goto out;
    }
    //alloacte memory for new struct process
    _process = kzalloc(sizeof(struct process));
    if(_process == NULL){
        res = -ENOMEM;
        goto out;
    }
    process_init(_process); 
    res = load_process_data(filename, _process); //load process
    if(res < 0){
        goto out;
    }
    _process->id = index;
    *process = _process;
    processes[index] = *process;//put new struct process into array
    
    out:
    if(res < 0){
        kfree(_process->code_ptr);
        kfree(_process->data_ptr);
        kfree(_process->stack_ptr);
        task_free(_process->main_task);
        kfree(_process);
    }
    return res;
}

int32_t process_load(char* filename, struct process** process){
    //finds next free space in processes array for new struct process which store info about new process
    for(int32_t i = 0; i < MY_OS_MAX_PROCESSES; i++){
        if(processes[i] == NULL){
            return process_load_for_index(filename, process, i);
        }
    }
    return -ENOMEM;
}
