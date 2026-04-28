# bab-os

A custom x86 operating system built from the ground up, featuring a multi-stage bootloader, a graphical C-based kernel, and a robust interrupt-driven driver architecture.

## Current Capabilities
- **Multi-Stage Bootloader**: Loads from MBR (Stage 1) to a larger Stage 2.
- **VESA/VBE Graphics**: Switches hardware into 1024x768x32bpp high-resolution mode.
- **Graphic Console**: A custom font renderer and terminal driver for pixel-based text output.
- **Interrupt Handling (IDT/ISR)**: Full support for CPU exceptions and hardware interrupts.
- **Interactive Keyboard Driver**: Support for US layout, Shift, and Caps Lock.
- **System Timer (PIT)**: Steady 100Hz heartbeat with power-saving `sleep(ms)` functionality.
- **Core Library**: Custom implementation of `string.h` (memset, memcpy, strlen) to maintain a zero-dependency kernel.

## Project Structure

```text
/
├── Makefile                # Build orchestrator (Assembles, Compiles, Links, Pads)
├── linker.ld               # Memory layout (Kernel at 0x1000)
├── include/                # Kernel Headers
│   ├── gdt.h / idt.h       # Segmentation and Interrupt tables
│   ├── isr.h               # Interrupt Service Routine definitions
│   ├── io.h                # Low-level I/O port helpers (inb/outb)
│   ├── keyboard.h          # Keyboard driver interface
│   ├── timer.h             # PIT timer and sleep interface
│   ├── string.h            # Core string/memory utilities
│   └── terminal.h          # Graphic console interface
├── src/
│   ├── boot/               # Custom Bootloader (Assembly)
│   │   ├── stage1.asm      # MBR boot sector
│   │   ├── stage2.asm      # VESA init & Kernel loader
│   │   └── kernel_entry.asm # Bridge to C kmain()
│   ├── common/             # Zero-dependency C library
│   │   └── string.c        # Memory and string operations
│   ├── kernel/             # Core Kernel Source
│   │   ├── main.c          # Kernel entry point and main loop
│   │   ├── gdt.c           # GDT initialization
│   │   ├── terminal.c      # Graphic terminal driver
│   │   ├── drivers/        # Hardware Drivers
│   │   │   ├── keyboard.c  # Interrupt-driven keyboard driver
│   │   │   ├── timer.c     # PIT Timer (100Hz)
│   │   │   └── pic.c       # Programmable Interrupt Controller remapping
│   │   └── sys/            # System Architecture
│   │       ├── idt.c       # Interrupt Descriptor Table
│   │       ├── isr.c       # Interrupt dispatch logic
│   │       └── interrupt.asm # Low-level ISR/IRQ assembly stubs
└── README.md
```

## How It Works

1.  **Boot Phase**: BIOS loads Stage 1, which pulls Stage 2 into memory. Stage 2 sets the high-res VESA mode and loads the C kernel.
2.  **Initialization**: The kernel initializes the **GDT** for memory segments and the **IDT** for interrupt handling.
3.  **Hardware Bridge**: The **PIC** is remapped to ensure hardware interrupts (IRQ 0-15) don't conflict with CPU exceptions.
4.  **Interactivity**: The keyboard driver and system timer are registered. Interrupts are enabled via `sti`.
5.  **Main Loop**: The kernel enters an infinite loop, providing a "heartbeat" message via the `sleep()` function while remaining responsive to user keyboard input.

## Prerequisites

- **NASM**: The Netwide Assembler.
- **i686-elf-gcc**: A cross-compiler (to avoid host headers/PIE/SSP).
- **GNU Binutils**: (`i686-elf-ld`) for linking.
- **QEMU**: For emulation.

## Getting Started

### 1. Build the OS Image
```bash
make
```

### 2. Run the OS
```bash
make run
```
You can now type in the terminal, use Shift/Caps Lock, and observe the system heartbeat every 5 seconds.

### 3. Cleanup
```bash
make clean
```
