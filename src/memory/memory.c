#include "memory.h"
#include <stdint.h>

//basic memory functionality

void* memset(void* ptr, int c, size_t len){ //setting all array elemnts to c
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

int32_t switch_endians(int32_t num){
    //transforming 4 byte data from one endian to another 
    int32_t switched_num;
    char* new_ptr, *ptr;
    ptr = (char*) &num;
    new_ptr = (char*) (&switched_num);
    for(int32_t i = 0; i < 4; i++){
        new_ptr[i] = ptr[3-i];
    }
    return switched_num;
}

int32_t memcpy(void* dest, void* src, uint32_t maxlen){//copy data from src array to dest array
    char* dest1 = (char*) dest;
    char* src1 = (char*) src;
    for(uint32_t i = 0; i < maxlen; i++){
        dest1[i] = src1[i];
    }
    return 0;
}