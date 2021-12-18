#include "memory.h"
#include <stdint.h>

//basic memory functionality

void* memset(void* ptr, int c, size_t len){
    char *p = (char*) ptr;
    for(int i = 0; i < len; i++){
        p[i] = (char) c;
    }
    return ptr;
}



int32_t memcmp(void* m1, void* m2, uint32_t maxlen){    //for comaparing two arrays
    char* ptr1 = (char*) m1;
    char* ptr2 = (char*) m2;
    for(uint32_t i = 0; i < maxlen; i++){
        if(ptr1[i] != ptr2[i]){
            return ptr1[i] - ptr2[i]; 
        }
    }
    return 0;
}