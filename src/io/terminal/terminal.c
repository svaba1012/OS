#include "terminal.h"
#include <stdint.h>
#include <stddef.h>
#include "string.h"

#define MAX_BASE_NUM_SYSTEM 30

uint8_t vga_color; 
uint16_t* video_mem;
uint16_t terminal_col;
uint16_t terminal_row;

uint16_t terminal_make_char(char c, char color){ //combining char ASCII value with char color
    return (color << 8) | c;        
}

void terminal_init(){    //erasing the screen
    video_mem = (uint16_t*) VGA_ADDR_START;
    terminal_col = 0;
    terminal_row = 0;
    for(uint16_t i  = 0; i < TERMINAL_HEIGHT * TERMINAL_WIDTH; i++){
        video_mem[i] = terminal_make_char(' ', BLACK);
    }
}

void terminal_putchar(uint16_t x, uint16_t y, char c){
    video_mem[y * TERMINAL_WIDTH + x] = terminal_make_char(c, vga_color);
    return;
}

void terminal_writechar(char c){
    if(c == '\n'){
        terminal_row++;
        terminal_col = 0;
        return;
    }
    terminal_putchar(terminal_col, terminal_row, c);
    terminal_col++;
    if(terminal_col == TERMINAL_WIDTH){
        terminal_col = 0;
        terminal_row++;
    }
    return;
}

void set_color(uint8_t color){  // setting global color
    vga_color = color;
    return;
}


void print(char* str){
    for(char* p = str; *p; p++){
        terminal_writechar(*p);
    }
    return;
}

void print_num(int32_t num, uint32_t base){
    //printing int numbers in base number system, suported number system base<17
    if(base > MAX_BASE_NUM_SYSTEM){
        print("Unsuported base for number system \n");
        return;
    }
    if(num == 0){
        print("0");
        return;
    }
    char digits[TERMINAL_WIDTH];
    char digit;
    uint32_t num_len = 0;
    if(num < 0){    //if the num is negative print - 
        num *= -1; //negate it to make pos and neg numbers the same
        print("-");
    }
    switch (base){ //print some prefixies for famous base number system
    case 2:
        print("0b");
        break;
    case 16:
        print("0x");
        break;
    default:
        break;
    }
    while (num > 0){
        digit = num % base;
        if(digit < 10){
            digit += '0';   //seting all digits lower than 10 to their char equivalent
        }else{
            digit -= 10;
            digit += 'A';   //seting all digits higher than 10 in alphabetic order
                            //A is 10, B is 11 and so on
        }
        digits[num_len++] = digit; //saving digits of number in reverse order
        num /= base;    
    }
    for(int32_t i = num_len - 1; i >= 0; i--){
        terminal_writechar(digits[i]); //printing string digits in reverse
    }
}