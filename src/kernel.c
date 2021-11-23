#include "./kernel.h"
#include <stdint.h>


void kernel_main(){
    uint16_t* video_mem = (uint16_t*) 0xb8000;
    *video_mem = 'A' | (2 << 8); 
    return;
}

