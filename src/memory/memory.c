#include "memory.h"

void* _memset(void* ptr, int c, size_t len){
    char *p = (char*) ptr;
    for(int i = 0; i < len; i++){
        p[i] = (char) c;
    }
    return ptr;
}
