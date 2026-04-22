# Tools
CC = i686-elf-gcc
AS = nasm
LD = i686-elf-ld

# Flags
# CFLAGS:
# -ffreestanding: No standard library, entry point isn't necessarily main
# -m32: Generate 32-bit x86 code for Protected Mode
# -g: Include debug information for GDB
# -fno-stack-protector: Disable stack canaries (requires runtime support we don't have)
# -fno-pie: Disable Position Independent Executable (we need fixed memory addresses)
# -mno-sse: Disable SSE instructions (requires extra CPU setup/init)
CFLAGS = -ffreestanding -m32 -g -fno-stack-protector -fno-pie -mno-sse

# LDFLAGS:
# -T linker.ld: Use our linker script to map memory (e.g., kernel at 0x1000)
# --oformat binary: Output flat machine code instead of an ELF file
# -nostdlib: Do not link standard libraries or default C runtime
LDFLAGS = -T linker.ld --oformat binary -nostdlib

# Files
KERNEL_SRCS = src/kernel/main.c
KERNEL_OBJS = $(KERNEL_SRCS:.c=.o)

.PHONY: all clean run

all: os-image.bin

os-image.bin: boot.bin stage2.bin kernel.bin
	cat $^ > $@
	# Pad image to at least 20 sectors to avoid BIOS read errors
	truncate -s 10240 $@

boot.bin: src/boot/stage1.asm
	$(AS) $< -f bin -o $@

stage2.bin: src/boot/stage2.asm
	$(AS) $< -f bin -o $@
	# Ensure stage2 is at least 512 bytes
	truncate -s %512 $@

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
