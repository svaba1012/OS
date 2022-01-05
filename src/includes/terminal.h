#ifndef TERMINAL_H
#define TERMINAL_H
#include <stdint.h>

#define TERMINAL_WIDTH 80
#define TERMINAL_HEIGHT 20
#define VGA_ADDR_START 0xb8000

enum terminal_colors{BLACK, BLUE, GREEN, CYAN, RED, PURPLE, BROWN, GREY, DARK_GRAY, L_BLUE, L_GREEN, L_CYAN, L_RED, L_PURPLE, YELLOW, WHITE};
//value of colors

void print(char* str);
void print_num(int32_t num, uint32_t base);
void terminal_init();
void set_color(uint8_t color);
void panic(char* msg);

#endif