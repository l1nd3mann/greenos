# Makefile для сборки ядра MiniOS

# Компиляторы и инструменты
CC = gcc
LD = ld
NASM = nasm
OBJCOPY = objcopy

# Флаги компиляции
CFLAGS = -ffreestanding -fno-stack-protector -nostdlib -fno-builtin \
         -Wall -Wextra -I./include -m64 -mcmodel=large -mred-zone \
         -c -O2

ASFLAGS = -f elf64

# Флаги линковки
LDFLAGS = -nostdlib -ffreestanding -T linker.ld

# Директории
KERNEL_DIR = kernel
BOOT_DIR = $(KERNEL_DIR)/boot
LIBC_DIR = $(KERNEL_DIR)/libc
NET_DIR = $(KERNEL_DIR)/drivers/net
FS_DIR = $(KERNEL_DIR)/fs

# Выходные файлы
ISO_NAME = minios.iso
KERNEL_BIN = kernel.bin
KERNEL_ELF = kernel.elf
BOOT_BIN = boot.bin

# Исходные файлы
ASM_SOURCES = $(BOOT_DIR)/boot.asm
C_SOURCES = $(KERNEL_DIR)/kernel.c \
            $(LIBC_DIR)/string.c \
            $(LIBC_DIR)/terminal.c \
            $(NET_DIR)/network.c \
            $(FS_DIR)/pkg_manager.c

# Объектные файлы
ASM_OBJECTS = $(BOOT_DIR)/boot.o
C_OBJECTS = $(C_SOURCES:.c=.o)

# Цель по умолчанию
all: $(ISO_NAME)

# Сборка загрузчика (16-bit bin формат)
$(BOOT_BIN): $(BOOT_DIR)/boot.asm
	@echo "[NASM] Creating boot sector..."
	$(NASM) -f bin -o $@ $<

# Сборка C файлов
$(KERNEL_DIR)/%.o: $(KERNEL_DIR)/%.c
	@echo "[GCC] Compiling $<..."
	$(CC) $(CFLAGS) -o $@ $<

$(LIBC_DIR)/%.o: $(LIBC_DIR)/%.c
	@echo "[GCC] Compiling $<..."
	$(CC) $(CFLAGS) -o $@ $<

$(NET_DIR)/%.o: $(NET_DIR)/%.c
	@echo "[GCC] Compiling $<..."
	$(CC) $(CFLAGS) -o $@ $<

$(FS_DIR)/%.o: $(FS_DIR)/%.c
	@echo "[GCC] Compiling $<..."
	$(CC) $(CFLAGS) -o $@ $<

# Линковка ядра (используем gcc вместо ld)
$(KERNEL_ELF): $(C_OBJECTS) linker.ld
	@echo "[LD] Linking kernel..."
	$(CC) -nostdlib -ffreestanding -T linker.ld -o $@ $(C_OBJECTS)

# Создание бинарного образа ядра
$(KERNEL_BIN): $(KERNEL_ELF)
	@echo "[OBJCOPY] Creating binary kernel..."
	$(OBJCOPY) -O binary $< $@

# Создание полного образа диска
$(ISO_NAME): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "[ISO] Creating bootable disk image..."
	@cat $(BOOT_BIN) $(KERNEL_BIN) > $@
	@echo "[OK] Build complete!"

# Запуск в QEMU
run: $(ISO_NAME)
	@echo "[QEMU] Starting MiniOS..."
	qemu-system-x86_64 $(ISO_NAME) -boot c \
		-m 512M \
		-netdev user,id=net0 \
		-device e1000,netdev=net0 \
		-serial stdio \
		-display curses

# Запуск с отладкой
debug: $(ISO_NAME)
	@echo "[QEMU] Starting MiniOS with GDB stub..."
	qemu-system-x86_64 $(ISO_NAME) -boot c \
		-m 512M \
		-netdev user,id=net0 \
		-device e1000,netdev=net0 \
		-serial stdio \
		-display curses \
		-s -S

# Очистка
clean:
	@echo "[CLEAN] Removing build files..."
	rm -rf $(BOOT_DIR)/*.o
	rm -rf $(KERNEL_DIR)/*.o
	rm -rf $(LIBC_DIR)/*.o
	rm -rf $(NET_DIR)/*.o
	rm -rf $(FS_DIR)/*.o
	rm -rf $(KERNEL_ELF) $(KERNEL_BIN) $(BOOT_BIN)
	rm -rf isodir
	rm -rf $(ISO_NAME)
	rm -rf disk_image.img

# Установка инструментов (для Debian/Ubuntu)
install-deps:
	@echo "Installing dependencies..."
	sudo apt-get update
	sudo apt-get install -y nasm gcc qemu-system-x86 grub-pc-bin xorriso

# Помощь
help:
	@echo "MiniOS Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build the kernel and ISO (default)"
	@echo "  run          - Build and run in QEMU"
	@echo "  debug        - Build and run with GDB stub"
	@echo "  clean        - Remove all build files"
	@echo "  install-deps - Install required dependencies"
	@echo "  help         - Show this help message"

.PHONY: all run debug clean install-deps help
