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
# -Iinclude: Add the include directory to the header search path
CFLAGS = -ffreestanding -m32 -g -fno-stack-protector -fno-pie -mno-sse -Iinclude

# LDFLAGS:
# -T linker.ld: Use our linker script to map memory (e.g., kernel at 0x1000)
# --oformat binary: Output flat machine code instead of an ELF file
# -nostdlib: Do not link standard libraries or default C runtime
LDFLAGS = -T linker.ld --oformat binary -nostdlib

# Files
KERNEL_SRCS = src/kernel/main.c src/kernel/font.c src/kernel/terminal.c src/kernel/gdt.c \
              src/kernel/sys/idt.c src/kernel/sys/isr.c src/common/string.c src/common/stdlib.c \
              src/kernel/drivers/keyboard.c src/kernel/drivers/timer.c src/kernel/drivers/pic.c \
              src/kernel/mm/pmm.c src/kernel/mm/vmm.c src/kernel/mm/kheap.c src/kernel/task.c \
              src/kernel/mutex.c src/kernel/cond.c src/kernel/monitor.c \
              src/kernel/vfs.c src/kernel/initrd.c
KERNEL_OBJS = $(KERNEL_SRCS:.c=.o) src/kernel/gdt_flush.o src/kernel/sys/idt_load.o src/kernel/sys/interrupt.o

.PHONY: all clean run

all: os-image.bin

os-image.bin: boot.bin stage2.bin kernel.bin initrd.img
	cat boot.bin stage2.bin kernel.bin > $@
	# Pad to 256KB so initrd starts at a fixed LBA (sector 513)
	truncate -s 262144 $@
	cat initrd.img >> $@
	# Pad image to 512KB total
	truncate -s 524288 $@
	@echo "Kernel size: $$(stat -c%s kernel.bin) bytes"
	@echo "Initrd size: $$(stat -c%s initrd.img) bytes"

boot.bin: src/boot/stage1.asm
	$(AS) $< -f bin -o $@

stage2.bin: src/boot/stage2.asm kernel.bin initrd.img
	$(AS) $< -f bin -o $@ \
		-D KERNEL_SECTORS=$$(expr \( $$(stat -c%s kernel.bin) + 511 \) / 512) \
		-D INITRD_SECTORS=$$(expr \( $$(stat -c%s initrd.img) + 511 \) / 512)
	# Ensure stage2 is exactly 1024 bytes (2 sectors)
	truncate -s 1024 $@

kernel.bin: kernel_entry.o $(KERNEL_OBJS)
	$(LD) -o $@ $(LDFLAGS) $^

initrd.img: tools/mkinitrd.py README.md
	python3 tools/mkinitrd.py $@ README.md

kernel_entry.o: src/boot/kernel_entry.asm
	$(AS) $< -f elf -o $@

src/kernel/gdt_flush.o: src/kernel/gdt_flush.asm
	$(AS) $< -f elf -o $@

src/kernel/sys/idt_load.o: src/kernel/sys/idt_load.asm
	$(AS) $< -f elf -o $@

src/kernel/sys/interrupt.o: src/kernel/sys/interrupt.asm
	$(AS) $< -f elf -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# Stage 1 ELF (Linked at 0x7C00)
boot.elf: src/boot/stage1.asm
	$(AS) $< -f elf32 -g -F dwarf -o boot_sym.o
	$(LD) boot_sym.o -m elf_i386 -Ttext 0x7C00 -o $@

# Stage 2 ELF (Linked at 0x7E00)
stage2.elf: src/boot/stage2.asm
	$(AS) $< -f elf32 -g -F dwarf -o stage2_sym.o \
		-D KERNEL_SECTORS=$$(expr \( $$(stat -c%s kernel.bin) + 511 \) / 512) \
		-D INITRD_SECTORS=$$(expr \( $$(stat -c%s initrd.img) + 511 \) / 512)
	$(LD) stage2_sym.o -m elf_i386 -Ttext 0x7E00 -o $@	

kernel.elf: kernel_entry.o $(KERNEL_OBJS)
	$(LD) -o $@ -T linker.ld $^

clean:
	rm -rf *.bin *.o *.elf *.img src/kernel/*.o src/boot/*.o src/kernel/sys/*.o src/common/*.o src/kernel/drivers/*.o src/kernel/mm/*.o

run: os-image.bin
	qemu-system-i386 -drive format=raw,file=os-image.bin


debug: os-image.bin kernel.elf boot.elf stage2.elf
	# -s: Starts GDB server on port 1234
	# -S: Freeze CPU at startup
	qemu-system-i386 -s -S -drive format=raw,file=os-image.bin -vga std -serial stdio
