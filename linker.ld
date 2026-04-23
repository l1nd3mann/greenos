/* linker.ld - Скрипт компоновщика */

ENTRY(_start)

SECTIONS
{
    . = 0x100000; /* Загружаем ядро по адресу 1MB */

    .text : ALIGN(4K)
    {
        *(.text*)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata*)
    }

    .data : ALIGN(4K)
    {
        *(.data*)
    }

    .bss : ALIGN(4K)
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    __kernel_end = .;
}
