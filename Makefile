# Makefile для сборки ядра и образа диска

CC = gcc
LD = ld
AS = nasm
CFLAGS = -m64 -ffreestanding -nostdlib -O2 -fno-pie -no-pie -Wall -fno-builtin -fno-stack-protector -I./include -mcmodel=large
LDFLAGS = -m elf_x86_64 -nostdlib -static -T linker.ld -no-pie

OBJS = kernel.o virtio.o string.o pkg_manager.o vga.o serial.o

all: os.img

boot.bin: boot.asm
	$(AS) -f bin $< -o $@

kernel.elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: fs/%.c
	$(CC) $(CFLAGS) -c $< -o $@

os.img: boot.bin kernel.bin
	# Создаем образ дискеты/жесткого диска (1.44MB + место под ядро)
	cat boot.bin kernel.bin > os.img
	# Дополним до размера, если нужно (опционально)
	# truncate -s 4M os.img

kernel.bin: kernel.elf
	objcopy -O binary $< $@

clean:
	rm -f *.o *.elf *.bin *.img output.log fs/*.o

run: os.img
	qemu-system-x86_64 \
		-drive file=os.img,format=raw \
		-m 512M \
		-netdev user,id=net0 \
		-device e1000,netdev=net0 \
		-serial stdio \
		-display curses

run-log: os.img
	qemu-system-x86_64 \
		-drive file=os.img,format=raw \
		-m 512M \
		-netdev user,id=net0 \
		-device e1000,netdev=net0 \
		-serial file:output.log \
		-display none

.PHONY: all clean run run-log
