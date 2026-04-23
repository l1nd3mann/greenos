; boot.asm - Простой загрузчик (первая стадия)
; Загружается BIOS по адресу 0x7C00, переключает процессор в защищенный режим
; и загружает ядро с диска в память по адресу 0x100000

[bits 16]
[org 0x7c00]

KERNEL_OFFSET equ 0x100000 ; Адрес, куда загрузим ядро

start:
    ; Настройка сегментов
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; Сообщение о загрузке
    mov si, MSG_LOAD
    call print_string

    ; Загрузка ядра с диска
    ; Читаем сектора после загрузчика (начиная со 2-го сектора)
    mov dl, [BOOT_DRIVE] ; Загрузочный диск из BIOS
    mov eax, KERNEL_OFFSET
    mov ebx, 2           ; Начальный сектор (сектор 1 - это мы)
    mov ecx, 64          ; Читаем 64 сектора (достаточно для ядра)
    call disk_load

    ; Переход в защищенный режим
    cli                  ; Отключаем прерывания
    lgdt [gdt_descriptor]; Загружаем таблицу дескрипторов

    ; Включаем защищенный режим
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; Дальний переход в 32-битный режим
    jmp CODE_SEG:start_protected

; === 16-битные подпрограммы ===

print_string:
    pusha
    mov ah, 0x0E
.print_loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .print_loop
.done:
    popa
    ret

disk_load:
    ; DL = номер диска
    ; EAX = адрес назначения
    ; EBX = начальный сектор
    ; ECX = количество секторов
    pusha
    push dx

    ; Ждем готовности диска
    mov ah, 0x02       ; Функция чтения
    mov al, cl         ; Количество секторов
    mov ch, 0          ; Цилиндр 0
    mov cl, bl         ; Сектор (начинается с 1, но у нас смещение)
    inc cl             ; CL = BL + 1 (т.к. сектор 1 - это MBR)
    mov dh, 0          ; Головка 0
    int 0x13           ; Вызов прерывания диска

    jc disk_error      ; Если ошибка флага CF

    pop dx
    popa
    ret

disk_error:
    mov si, MSG_ERR
    call print_string
    jmp $

; === Данные ===
MSG_LOAD db "Booting from disk...", 13, 10, 0
MSG_ERR  db "Disk error!", 13, 10, 0
BOOT_DRIVE db 0

; === Выравнивание и подпись загрузчика ===
times 510 - ($ - $$) db 0
dw 0xAA55

; === Глобальная таблица дескрипторов (GDT) ===
[bits 32]

CODE_SEG equ 0x08
DATA_SEG equ 0x10

gdt_start:
    ; Нулевой дескриптор (обязательно)
    dq 0

    ; Кодовый сегмент: базовый=0, лимит=4GB, исполняемый, читаемый, присутствующий
    ; Гранулярность 4KB, 32-битный
    dw 0xFFFF        ; Лимит (нижние 16 бит)
    dw 0             ; База (нижние 16 бит)
    db 0             ; База (средние 8 бит)
    db 10011010b     ; Флаги: присутствующий, кольцо 0, код, исполняемый, читаемый
    db 11001111b     ; Флаги: гранулярность 4KB, 32-битный + лимит (верхние 4 бита)
    db 0             ; База (верхние 8 бит)

    ; Сегмент данных: базовый=0, лимит=4GB, записываемый, присутствующий
    dw 0xFFFF
    dw 0
    db 0
    db 10010010b     ; Присутствующий, кольцо 0, данные, записываемый
    db 11001111b
    db 0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; Размер GDT
    dd gdt_start               ; Адрес GDT

; === Точка входа в защищенном режиме ===
[bits 32]

start_protected:
    ; Настройка сегментных регистров
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000 ; Устанавливаем стек повыше

    ; Переход к ядру
    jmp KERNEL_OFFSET
