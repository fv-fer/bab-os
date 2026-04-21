# bab-os

A custom x86 operating system built from the ground up, featuring a manual 16-bit bootloader and a C-based kernel with direct hardware access.

## Project Structure

The project is organized into a modular hierarchy to keep the bootloader, kernel logic, and hardware drivers separate and maintainable.

```text
/
├── Makefile                # The build orchestrator (Assembles, Compiles, Links)
├── linker.ld               # Memory layout definition (Links kernel at 0x1000)
├── src/
│   ├── boot/               # Custom Bootloader (Assembly)
│   │   ├── stage1.asm      # The 512-byte MBR boot sector
│   │   ├── print_string.asm # 16-bit BIOS print utility
│   │   └── kernel_entry.asm # The bridge between ASM and C
│   ├── kernel/             # Core Kernel Source
│   │   ├── main.c          # kmain() entry point & VGA driver logic
│   │   ├── arch/           # Architecture specific (GDT, IDT, Paging)
│   │   ├── drivers/        # Hardware drivers (Keyboard, Disk)
│   │   ├── mm/             # Memory Management (PMM, VMM)
│   │   ├── sys/            # Multitasking, Scheduling & Syscalls
│   │   └── fs/             # File System implementations (FAT)
│   ├── common/             # Minimal C library (libk)
│   └── include/            # Global header files
└── scripts/                # Image creation and deployment scripts
```

## How It Works

1.  **Stage 1 Bootloader**: When the PC starts, the BIOS loads the first 512 bytes (`stage1.asm`) into memory at `0x7C00` and executes it.
2.  **Kernel Handoff**: The bootloader is responsible for loading the rest of the OS from the disk into memory (currently set to `0x1000`) and switching the CPU from 16-bit Real Mode to 32-bit Protected Mode.
3.  **Kernel Entry**: `kernel_entry.asm` calls the `kmain()` function in your C code.
4.  **VGA Driver**: Since there is no standard library, `main.c` communicates with the screen by writing directly to the video memory at `0xB8000`.

## Prerequisites

To build and run this OS, you need the following tools installed on your system:

-   **NASM**: The Netwide Assembler for the bootloader.
-   **i686-elf-gcc**: A cross-compiler (to ensure the OS doesn't use host system headers).
-   **GNU Binutils**: (`i686-elf-ld`) for linking the kernel.
-   **QEMU**: For emulating the x86 hardware.

## Getting Started

### 1. Initialize the Environment
Ensure your cross-compiler is in your system `PATH`. You can verify this by running:
```bash
i686-elf-gcc --version
nasm -v
```

### 2. Build the OS Image
Run the following command to assemble the bootloader, compile the kernel, and create the bootable image:
```bash
make
```
This generates `os-image.bin`, which is a concatenation of the bootloader and the kernel.

### 3. Run the OS
Use QEMU to boot the image:
```bash
make run
```
You should see "Started in 16-bit Real Mode" briefly (via BIOS) and then "Hello" printed on the screen (via your C VGA driver).

### 4. Cleanup
To remove generated binaries and object files:
```bash
make clean
```
