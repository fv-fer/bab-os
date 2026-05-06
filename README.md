# bab-os

A custom x86 operating system built from the ground up, featuring a multi-stage bootloader, a graphical C-based kernel, and a robust interrupt-driven driver architecture.

## Current Capabilities
- **Multi-Stage Bootloader**: Loads from MBR (Stage 1) to a larger Stage 2.
- **Preemptive Multitasking**: Round-robin scheduler with automatic context switching via timer interrupts.
- **Memory Management**: 
    - **PMM**: Physical Memory Manager using a Buddy System allocator.
    - **VMM**: Higher-half kernel mapping (0xC0000000) with full paging support.
    - **Heap**: Dynamic kernel memory allocation (`kmalloc`/`kfree`).
- **VESA/VBE Graphics**: Switches hardware into 1024x768x32bpp high-resolution mode.
- **Graphic Console**: A custom font renderer and terminal driver for pixel-based text output.
- **Interrupt Handling (IDT/ISR)**: Full support for CPU exceptions and hardware interrupts.
- **Interactive Keyboard Driver**: Support for US layout, Shift, and Caps Lock.
- **System Timer (PIT)**: Steady 100Hz heartbeat with task preemption.
- **Core Library**: Custom implementation of `string.h` and `stdlib.h` for a zero-dependency kernel.

## Project Structure

```text
/
├── Makefile                # Build orchestrator (Assembles, Compiles, Links, Pads)
├── linker.ld               # Memory layout (Kernel linked at 0xC0001000)
├── include/                # Kernel Headers
│   ├── gdt.h / idt.h       # Segmentation and Interrupt tables
│   ├── isr.h               # Interrupt Service Routine definitions
│   ├── io.h                # Low-level I/O port helpers (inb/outb)
│   ├── keyboard.h          # Keyboard driver interface
│   ├── timer.h             # PIT timer and sleep interface
│   ├── task.h              # Multitasking and TCB definitions
│   ├── pmm.h / vmm.h       # Physical and Virtual memory management
│   ├── kheap.h             # Kernel heap allocator
│   ├── string.h / stdlib.h # Core C library utilities
│   └── stdint.h / stddef.h # Standard type definitions
├── src/
│   ├── boot/               # Custom Bootloader (Assembly)
│   │   ├── stage1.asm      # MBR boot sector
│   │   ├── stage2.asm      # VESA init & Kernel loader
│   │   └── kernel_entry.asm # Higher-half bridge to C kmain()
│   ├── common/             # Zero-dependency C library
│   │   ├── string.c        # Memory and string operations
│   │   └── stdlib.c        # Utility functions (itoa, abs)
│   ├── kernel/             # Core Kernel Source
│   │   ├── main.c          # Kernel entry point and test tasks
│   │   ├── gdt.c           # GDT initialization
│   │   ├── terminal.c      # Graphic terminal driver
│   │   ├── task.c          # Scheduler and task management
│   │   ├── drivers/        # Hardware Drivers
│   │   │   ├── keyboard.c  # Interrupt-driven keyboard driver
│   │   │   ├── timer.c     # PIT Timer (100Hz)
│   │   │   └── pic.c       # PIC remapping
│   │   ├── mm/             # Memory Management
│   │   │   ├── pmm.c       # Physical allocator (Buddy System)
│   │   │   ├── vmm.c       # Virtual memory and paging
│   │   │   └── kheap.c     # Kernel heap
│   │   └── sys/            # System Architecture
│   │       ├── idt.c       # Interrupt Descriptor Table
│   │       ├── isr.c       # Interrupt dispatch logic
│   │       └── interrupt.asm # Low-level context-switching stubs
└── README.md
```

## How It Works

1.  **Boot Phase**: BIOS loads Stage 1, which pulls Stage 2. Stage 2 initializes VESA graphics, enables basic paging, and jumps to the higher-half kernel.
2.  **Memory Init**: The kernel initializes the **PMM (Buddy System)** and **VMM**, then sets up the **Kernel Heap** for dynamic allocations.
3.  **System Init**: The **GDT** and **IDT** are configured. Hardware interrupts are remapped via the **PIC**.
4.  **Multitasking**: The **Scheduler** is initialized, converting the main thread into the first task and creating additional concurrent tasks.
5.  **Preemption**: The **PIT** (Timer) triggers an interrupt every 10ms. Every 100ms, the scheduler automatically saves the current task's state and switches the CPU to the next task in the queue.

## Prerequisites

- **NASM**: The Netwide Assembler.
- **i686-elf-gcc**: A cross-compiler.
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

### 3. Cleanup
```bash
make clean
```
