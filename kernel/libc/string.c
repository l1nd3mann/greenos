#include "kernel.h"

// Простая реализация memcpy для ядра
void* memcpy(void* dest, const void* src, size_t n) {
    u8* d = (u8*)dest;
    const u8* s = (const u8*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

// Простая реализация memset для ядра
void* memset(void* s, int c, size_t n) {
    u8* p = (u8*)s;
    while (n--) {
        *p++ = (u8)c;
    }
    return s;
}

// Простая реализация strcmp
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// Простая реализация strlen
size_t strlen(const char* s) {
    size_t len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

// Простая реализация strcpy
char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}
