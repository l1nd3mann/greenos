#ifndef VGA_H
#define VGA_H

#include "types.h"

void vga_init();
void vga_write_char(char c, uint8_t color);
void vga_write_str(const char* str, uint8_t color);
void vga_clear();

#endif
