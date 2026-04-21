# Tools
CC = i686-elf-gcc
AS = nasm
LD = i686-elf-ld

# Flags
CFLAGS = -ffreestanding -m32 -g
LDFLAGS = -T linker.ld --oformat binary

# Files
BOOTLOADER_SRCS = src/boot/stage1.asm
KERNEL_SRCS = src/kernel/main.c
KERNEL_OBJS = $(KERNEL_SRCS:.c=.o)

.PHONY: all clean run

all: os-image.bin

os-image.bin: boot.bin kernel.bin
	cat $^ > $@

boot.bin: src/boot/stage1.asm
	$(AS) $< -f bin -o $@

kernel.bin: kernel_entry.o $(KERNEL_OBJS)
	$(LD) -o $@ $(LDFLAGS) $^

kernel_entry.o: src/boot/kernel_entry.asm
	$(AS) $< -f elf -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.bin *.o src/kernel/*.o src/boot/*.o

run: os-image.bin
	qemu-system-i386 -drive format=raw,file=os-image.bin
