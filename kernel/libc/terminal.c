#include "kernel.h"

static u16* vga_buffer = (u16*)VGA_MEMORY;
static size_t cursor_x = 0;
static size_t cursor_y = 0;

// Создание VGA символа с цветом
static inline u16 vga_entry(unsigned char uc, u8 color) {
    return (u16)uc | ((u16)color << 8);
}

// Инициализация экрана
void terminal_clear() {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', VGA_LIGHT_GREY);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

// Установка цвета текста
void terminal_setcolor(u8 fg, u8 bg) {
    // Можно расширить для поддержки цветов
}

// Перевод строки
static void terminal_newline() {
    cursor_x = 0;
    cursor_y++;
    if (cursor_y >= VGA_HEIGHT) {
        // Скроллинг экрана
        for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
            }
        }
        // Очистка последней строки
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', VGA_LIGHT_GREY);
        }
        cursor_y = VGA_HEIGHT - 1;
    }
}

// Вывод символа
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_newline();
        return;
    }
    
    if (c == '\r') {
        cursor_x = 0;
        return;
    }
    
    if (c == '\t') {
        cursor_x += 4;
        cursor_x &= ~3;  // Выравнивание по 4
        if (cursor_x >= VGA_WIDTH) {
            terminal_newline();
        }
        return;
    }
    
    if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(' ', VGA_LIGHT_GREY);
        }
        return;
    }
    
    if (cursor_x >= VGA_WIDTH) {
        terminal_newline();
    }
    
    vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, VGA_WHITE);
    cursor_x++;
}

// Вывод строки
void terminal_writestring(const char* data) {
    while (*data) {
        terminal_putchar(*data++);
    }
}

// Вывод числа (hex)
void terminal_writehex(u64 value) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[17];
    int i = 16;
    buffer[16] = 0;
    
    if (value == 0) {
        terminal_writestring("0");
        return;
    }
    
    while (value > 0 && i > 0) {
        buffer[--i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    terminal_writestring(&buffer[i]);
}

// Вывод числа (decimal)
void terminal_writedecimal(u64 value) {
    if (value == 0) {
        terminal_putchar('0');
        return;
    }
    
    char buffer[21];
    int i = 20;
    buffer[20] = 0;
    
    while (value > 0 && i > 0) {
        buffer[--i] = '0' + (value % 10);
        value /= 10;
    }
    
    terminal_writestring(&buffer[i]);
}

// Инициализация терминала
void terminal_init() {
    terminal_clear();
}
