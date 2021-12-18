#include "string.h"
#include <stdint.h>
#include <stdbool.h>

//basic string functionality


uint32_t strlen(char* str){
    char* ptr = str;
    while(*ptr){
        ptr++;
    }
    return (uint32_t)(ptr - str);
} 

uint32_t strnlen(char* str, uint32_t max_len){
    for(uint32_t i = 0; i < max_len; i++){
        if(str[i] == '\0'){
            return i;
        }
    }
    return max_len;
}

bool is_numeric(char c){
    return c >= '0' && c <= '9';
}

uint32_t char_to_numeric(char c){
    return c - '0';
}

