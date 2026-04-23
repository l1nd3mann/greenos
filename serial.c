#include "include/serial.h"
#include "include/types.h"

#define SERIAL_COM1 0x3F8

// Инициализация последовательного порта COM1
void serial_init() {
    // Отключаем прерывания
    __asm__ volatile ("outb %%al, %%dx" :: "a"(0x00), "d"(SERIAL_COM1 + 1));
    // Включаем DLAB (доступ к делителю)
    __asm__ volatile ("outb %%al, %%dx" :: "a"(0x80), "d"(SERIAL_COM1 + 3));
    // Устанавливаем делитель на 9600 бод (115200 / 9600 = 12)
    __asm__ volatile ("outb %%al, %%dx" :: "a"(0x01), "d"(SERIAL_COM1));     // младший байт
    __asm__ volatile ("outb %%al, %%dx" :: "a"(0x00), "d"(SERIAL_COM1 + 1)); // старший байт
    // 8 бит, 1 стоп-бит, без четности
    __asm__ volatile ("outb %%al, %%dx" :: "a"(0x03), "d"(SERIAL_COM1 + 3));
    // Включаем FIFO
    __asm__ volatile ("outb %%al, %%dx" :: "a"(0xC7), "d"(SERIAL_COM1 + 2));
    // Включаем прерывания (опционально)
    __asm__ volatile ("outb %%al, %%dx" :: "a"(0x0B), "d"(SERIAL_COM1 + 4));
}

// Проверка, готов ли порт к передаче
static int serial_is_transmit_empty() {
    uint8_t status;
    __asm__ volatile ("inb %%dx, %%al" : "=a"(status) : "d"(SERIAL_COM1 + 5));
    return status & 0x20;
}

void serial_write_char(char c) {
    while (!serial_is_transmit_empty());
    __asm__ volatile ("outb %%al, %%dx" :: "a"(c), "d"(SERIAL_COM1));
}

void serial_write_str(const char* str) {
    while (*str) {
        serial_write_char(*str++);
    }
}

int serial_received() {
    uint8_t status;
    __asm__ volatile ("inb %%dx, %%al" : "=a"(status) : "d"(SERIAL_COM1 + 5));
    return status & 0x01;
}

char serial_read_char() {
    while (!serial_received());
    uint8_t ret;
    __asm__ volatile ("inb %%dx, %%al" : "=a"(ret) : "d"(SERIAL_COM1));
    return (char)ret;
}
