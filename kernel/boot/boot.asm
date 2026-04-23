[bits 16]
[org 0x7c00]

; Загрузчик ядра - загружается BIOS по адресу 0x7c00
; Переход в 32-битный режим, затем загрузка ядра с диска

KERNEL_OFFSET equ 0x1000    ; Адрес загрузки ядра
KERNEL_SECTORS equ 32       ; Максимальный размер ядра (в секторах)

start:
    ; Инициализация сегментов
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    
    ; Сообщение о загрузке
    mov si, msg_loading
    call print_string
    
    ; Загрузка ядра с диска
    mov ah, 0x02              ; Функция чтения диска
    mov al, KERNEL_SECTORS    ; Количество секторов
    mov ch, 0                 ; Цилиндр 0
    mov cl, 2                 ; Сектор 2 (сектор 1 - это мы)
    mov dh, 0                 ; Головка 0
    mov dl, 0x80              ; Диск 0 (первый жесткий диск)
    mov bx, KERNEL_OFFSET     ; Буфер для чтения
    
    int 0x13                  ; Вызов BIOS
    
    jc disk_error             ; Если ошибка - переходим к обработке
    
    ; Проверка успешной загрузки
    cmp ah, 0
    jne disk_error
    
    ; Сообщение об успехе
    mov si, msg_success
    call print_string
    
    ; Переход в защищенный режим
    cli                       ; Отключаем прерывания
    lgdt [gdt_descriptor]     ; Загружаем таблицу дескрипторов
    
    ; Включаем защищенный режим
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Дальний переход в 32-битный режим
    jmp 0x08:kernel_entry_32
    
disk_error:
    mov si, msg_disk_error
    call print_string
    jmp $                     ; Бесконечный цикл

; Подпрограмма печати строки (реальный режим)
print_string:
    pusha
    mov ah, 0x0e              ; Функция BIOS "Teletype"
.print_loop:
    lodsb                     ; Загрузить байт из SI
    test al, al
    jz .done
    int 0x10                  ; Вызов видео-BIOS
    jmp .print_loop
.done:
    popa
    ret

; Данные
msg_loading:      db "Loading kernel...", 13, 10, 0
msg_success:      db "OK! Entering protected mode...", 13, 10, 0
msg_disk_error:   db "Disk error!", 13, 10, 0

; Таблица дескрипторов (GDT)
gdt_start:
    ; Нулевой дескриптор (обязательно должен быть)
    dq 0x0000000000000000
    
    ; Кодовый дескриптор (селектор 0x08)
    ; Base=0, Limit=4GB, Executable=1, Present=1, DPL=0, D/B=1
    dq 0x00cf9a000000ffff
    
    ; Дата-дескриптор (селектор 0x10)
    ; Base=0, Limit=4GB, Executable=0, Present=1, DPL=0, D/B=1
    dq 0x00cf92000000ffff
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1    ; Размер GDT
    dd gdt_start                   ; Адрес GDT

; Выравнивание и подпись загрузчика
times 510 - ($ - $$) db 0
dw 0xaa55

; 32-битная входная точка (в том же файле для простоты)
[bits 32]
kernel_entry_32:
    ; Инициализация сегментных регистров
    mov ax, 0x10        ; Селектор дата-сегмента
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000    ; Стек
    
    ; Сообщение о переходе в 32-битный режим
    mov esi, msg_32bit
    call print_string_32
    
    ; Переход в 64-битный режим
    ; Сначала включаем PAE
    mov eax, cr4
    or eax, 1 << 5      ; PAE bit
    mov cr4, eax
    
    ; Загружаем адрес таблицы страниц (будет заполнен ядром)
    ; Для простоты используем идентичное отображение
    mov edi, 0x10000    ; Адрес страницы для PML4
    mov cr3, edi
    
    ; Включаем long mode
    mov ecx, 0xc0000080
    rdmsr
    or eax, 1 << 8      ; LME bit
    wrmsr
    
    ; Включаем страничную трансляцию
    mov eax, cr0
    or eax, 1 << 31     ; PG bit
    mov cr0, eax
    
    ; Дальний переход в 64-битный режим
    jmp 0x08:kernel_entry_64

print_string_32:
    ; Простая печать через VGA память
    pushad
    mov edi, 0xb8000    ; Адрес видеопамяти
    xor ecx, ecx        ; Позиция курсора
.print_loop_32:
    lodsb
    test al, al
    jz .done_32
    mov [edi + ecx * 2], al
    mov [edi + ecx * 2 + 1], byte 0x0f  ; Белый цвет на черном
    inc ecx
    jmp .print_loop_32
.done_32:
    popad
    ret

msg_32bit: db "Entered 32-bit mode...", 0

[bits 64]
kernel_entry_64:
    ; Инициализация сегментов для 64-битного режима
    xor eax, eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rsp, 0x90000    ; Стек
    
    ; Сообщение о переходе в 64-битный режим
    lea rsi, [rel msg_64bit]
    call print_string_64
    
    ; Вызов ядра - используем относительный адрес
    ; Ядро будет по адресу 0x100000 (1MB)
    mov rax, 0x100000   ; Адрес kernel_main будет здесь после линковки
    add rax, kernel_main_offset
    call rax
    
    ; Если ядро вернется - зацикливаемся
.halt:
    hlt
    jmp .halt

print_string_64:
    push rax
    push rdi
    push rcx
    mov rdi, 0xb8000
    xor rcx, rcx
.print_loop_64:
    lodsb
    test al, al
    jz .done_64
    mov [rdi + rcx * 2], al
    mov [rdi + rcx * 2 + 1], byte 0x0f
    inc rcx
    jmp .print_loop_64
.done_64:
    pop rcx
    pop rdi
    pop rax
    ret

msg_64bit: db "Entered 64-bit LONG MODE! Calling kernel...", 0

; Смещение к kernel_main (будет заполнено при линковке)
kernel_main_offset: dq 0
