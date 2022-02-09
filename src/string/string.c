#include "string.h"
#include <stdint.h>
#include <stdbool.h>

//basic string functionality, commonly defined in string.h 


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

char* strcpy(char* dest, char* src){
    char* tmp = dest;
    while(*src){
        *dest++ = *src++;
    }
    *dest = 0;
    return tmp;
}

int32_t strcmp(char* str1, char* str2){
    while(1){
        if(*str1 != *str2){
            return *str1 - *str2;
        }
        if (*str1 == '\0'){
            break;
        }
    }
    return 0;
}

int32_t strncmp(char* str1, char* str2, int32_t n){
    for(int32_t i = 0; i < n; i++){
        if(str1[i] != str2[i]){
            return str1[i] - str2[i];
        }
        if (str1[i] == '\0'){
            break;
        }
    }
    return 0;
}

int32_t strncmp_terminating_char(char* str1, char* str2, int32_t n, char c){
    //compare to strings similar as strncmp, but it can also see char c as terminating
    for(int32_t i = 0; i < n;){
        if(str1[i] != str2[i]){
            return str1[i] - str2[i];
        }
        i++;
        if (str1[i] == '\0' || str1[i] == c){
            break;
        }
    }
    return 0;
}

char* strchr(char* str, char c){
    while(*str){
        if(*str == c){
            return str;
        }
        str++;
    }
    return NULL;
}

char* strnchr(char* str, char c, int32_t n){
    for(int32_t i = 0; i < n; i++){
        if(str[i] == c){
            return str + i;
        }
        if(str[i] == '\0'){
            break;
        }
    }
    return NULL;
}

char* strncpy(char* dest, char* src, size_t len){
    int32_t i = 0;
    for(i = 0; i < len; i++){
        dest[i] = src[i];
        if(dest[i] == '\0'){
            break;
        }
    }
    dest[i] = '\0';
    return dest;
}
