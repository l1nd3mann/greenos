/* Linker script для MiniOS ядра */

ENTRY(kernel_entry_64)

SECTIONS
{
    /* Ядро загружается по адресу 1MB */
    . = 0x100000;

    /* Код (текст) */
    .text : ALIGN(4K)
    {
        *(.multiboot)
        *(.text*)
    }

    /* Данные только для чтения */
    .rodata : ALIGN(4K)
    {
        *(.rodata*)
    }

    /* Инициализированные данные */
    .data : ALIGN(4K)
    {
        *(.data*)
    }

    /* Неинициализированные данные (BSS) */
    .bss : ALIGN(4K)
    {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = .;
    }

    /* Конец ядра */
    __kernel_end = .;
}
