#include "include/vga.h"
#include "include/types.h"

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
static int vga_row = 0;
static int vga_col = 0;

void vga_init() {
    vga_clear();
}

void vga_clear() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = 0x0720; // Пробел, серый текст на черном фоне
    }
    vga_row = 0;
    vga_col = 0;
}

void vga_write_char(char c, uint8_t color) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
        return;
    }

    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
    }

    if (vga_row >= VGA_HEIGHT) {
        // Прокрутка экрана (сдвигаем всё на строку вверх)
        for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
            vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
        }
        for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_WIDTH * VGA_HEIGHT; i++) {
            vga_buffer[i] = 0x0720;
        }
        vga_row = VGA_HEIGHT - 1;
    }

    vga_buffer[vga_row * VGA_WIDTH + vga_col] = (color << 8) | c;
    vga_col++;
}

void vga_write_str(const char* str, uint8_t color) {
    while (*str) {
        vga_write_char(*str++, color);
    }
}
