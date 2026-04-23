#include "kernel.h"

// Объявления функций из других модулей
extern void terminal_init();
extern void terminal_writestring(const char*);
extern void terminal_putchar(char);
extern void terminal_writedecimal(u64);
extern void terminal_writehex(u64);

extern void net_init();
extern int http_get(const struct ip_addr* server, const char* path, const char* host, u8* buffer, size_t max_len);
extern int dns_resolve(const char* hostname, struct ip_addr* result);

extern void pkg_init();
extern int pkg_install(const char* owner, const char* repo);
extern void pkg_list_installed();

// Простая реализация strstr
char* strstr(const char* haystack, const char* needle) {
    if (*needle == 0) return (char*)haystack;
    
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        
        if (*n == 0) return (char*)haystack;
        haystack++;
    }
    
    return 0;
}

// Простая реализация snprintf
int snprintf(char* str, size_t size, const char* format, ...) {
    // Очень упрощенная реализация для базовых форматов
    const char* fmt = format;
    char* ptr = str;
    char* end = str + size - 1;
    
    while (*fmt && ptr < end) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 's') {
                // Строковый аргумент (заглушка)
                *ptr++ = '?';
            } else if (*fmt == 'd') {
                // Числовой аргумент (заглушка)
                *ptr++ = '?';
            } else if (*fmt == '%') {
                *ptr++ = '%';
            }
        } else {
            *ptr++ = *fmt;
        }
        fmt++;
    }
    
    *ptr = 0;
    return ptr - str;
}

// Простая реализация sprintf для форматирования строк
char* itoa_simple(int value, char* str, int base) {
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    unsigned int abs_value;
    
    if (base == 10 && value < 0) {
        abs_value = -value;
        *ptr++ = '-';
    } else {
        abs_value = value;
    }
    
    do {
        *ptr++ = "0123456789abcdef"[abs_value % base];
        abs_value /= base;
    } while (abs_value);
    
    *ptr-- = '\0';
    
    // Реверс строки
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    
    return str;
}

// Точка входа ядра
void kernel_main() {
    // Инициализация терминала
    terminal_init();
    
    // Приветственное сообщение
    terminal_writestring("========================================\n");
    terminal_writestring("       MiniOS Kernel v1.0\n");
    terminal_writestring("   x86_64 OS with Network Support\n");
    terminal_writestring("========================================\n\n");
    
    terminal_writestring("[OK] Kernel loaded successfully!\n");
    terminal_writestring("[OK] VGA text mode initialized\n");
    
    // Инициализация сети
    net_init();
    
    // Инициализация пакетного менеджера
    pkg_init();
    
    terminal_writestring("\n");
    terminal_writestring("========================================\n");
    terminal_writestring("         System Ready!\n");
    terminal_writestring("========================================\n\n");
    
    // Демонстрация работы
    terminal_writestring("Demo: Installing package from GitHub...\n\n");
    
    // Пример установки пакета (закомментировано для MVP)
    // pkg_install("username", "repo-name");
    
    terminal_writestring("\nTip: Use 'pkg install <owner>/<repo>' to install packages\n");
    terminal_writestring("     Use 'pkg list' to show installed packages\n\n");
    
    // Бесконечный цикл
    while (1) {
        __asm__ volatile("hlt");
    }
}
