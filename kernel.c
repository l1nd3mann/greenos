/* kernel.c - Основное ядро */

#include "include/types.h"
#include "include/string.h"
#include "include/vga.h"
#include "include/serial.h"

// Объявления внешних функций
extern void virtio_init();
extern void pkg_manager_init();
extern int pkg_download_and_install(const char* repo, const char* pkg_name);

// Заглушки для сетевых функций (если не подключены)
__attribute__((weak))
int net_send_packet(const char* data, int len) {
    (void)data;
    (void)len;
    return -1;
}

__attribute__((weak))
int net_recv_packet(char* buffer, int max_len) {
    (void)buffer;
    (void)max_len;
    return 0;
}

// Точка входа
void _start() {
    // Инициализация VGA и Serial
    vga_init();
    serial_init();

    serial_write_str("[KERNEL] Boot started...\n");
    vga_write_str("MyOS Kernel Starting...", 0x0F);

    // Инициализация сети
    serial_write_str("[KERNEL] Initializing network...\n");
    vga_write_str("Initializing network...", 0x0A);
    virtio_init();

    // Инициализация менеджера пакетов
    serial_write_str("[KERNEL] Initializing package manager...\n");
    vga_write_str("Initializing package manager...", 0x0B);
    pkg_manager_init();

    // Пример: попытка скачать пакет (закомментировано для MVP)
    // pkg_download_and_install("https://raw.githubusercontent.com", "test-pkg");

    serial_write_str("[KERNEL] Boot complete. Waiting for commands...\n");
    vga_write_str("Boot complete!", 0x0F);

    // Бесконечный цикл
    while (1) {
        __asm__ volatile ("hlt");
    }
}
