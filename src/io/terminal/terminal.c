#include "terminal.h"
#include <stdint.h>
#include <stddef.h>

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

size_t strlen(char* str){
    size_t len = 0;
    while(str[len++]){
        ;
    }
    return len;
}

void print(char* str){
    for(char* p = str; *p; p++){
        terminal_writechar(*p);
    }
    return;
}