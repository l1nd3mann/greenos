#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

// Базовые типы
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

// VGA буфер для вывода текста
#define VGA_MEMORY 0xb8000
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

// Цвета для VGA
enum vga_color {
    VGA_BLACK = 0,
    VGA_BLUE = 1,
    VGA_GREEN = 2,
    VGA_CYAN = 3,
    VGA_RED = 4,
    VGA_MAGENTA = 5,
    VGA_BROWN = 6,
    VGA_LIGHT_GREY = 7,
    VGA_DARK_GREY = 8,
    VGA_LIGHT_BLUE = 9,
    VGA_LIGHT_GREEN = 10,
    VGA_LIGHT_CYAN = 11,
    VGA_LIGHT_RED = 12,
    VGA_LIGHT_MAGENTA = 13,
    VGA_YELLOW = 14,
    VGA_WHITE = 15,
};

// Структура сетевого пакета
struct network_packet {
    u8* data;
    size_t len;
};

// Структура IP адреса
struct ip_addr {
    u8 addr[4];
};

// Конфигурация сети
struct network_config {
    struct ip_addr ip;
    struct ip_addr gateway;
    struct ip_addr netmask;
    struct ip_addr dns;
};

#endif // KERNEL_H
