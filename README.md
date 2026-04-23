# bab-os

A custom x86 operating system built from the ground up, featuring a multi-stage bootloader and a graphical C-based kernel.

## Current Capabilities
- **Multi-Stage Bootloader**: Loads from MBR (Stage 1) to a larger Stage 2.
- **VESA/VBE Graphics**: Switches hardware into 1024x768x32bpp high-resolution mode.
- **Graphic Console**: A custom font renderer and terminal driver that allows structured text output on a pixel-based framebuffer.
- **32-bit Protected Mode**: Robust kernel entry via a dedicated assembly bridge.

## Project Structure

```text
/
├── Makefile                # Build orchestrator (Assembles, Compiles, Links, Pads)
├── linker.ld               # Memory layout (Kernel at 0x1000)
├── include/                # Header files
│   ├── font.h              # Bitmap font definitions
│   └── terminal.h          # Graphic console interface
├── src/
│   ├── boot/               # Custom Bootloader (Assembly)
│   │   ├── stage1.asm      # MBR boot sector (loads Stage 2)
│   │   ├── stage2.asm      # VESA initialization & Kernel loader
│   │   ├── disk_load.asm   # BIOS disk I/O utility
│   │   ├── gdt.asm         # Global Descriptor Table setup
│   │   ├── switch_to_pm.asm # 16-bit to 32-bit transition
│   │   └── kernel_entry.asm # Bridge to C kmain()
│   ├── kernel/             # Core Kernel Source
│   │   ├── main.c          # kmain() & High-level logic
│   │   ├── font.c          # 8x8 bitmap font data
│   │   └── terminal.c      # Graphic terminal driver (pixels to text)
└── README.md
```

## How It Works

1.  **Stage 1 (MBR)**: BIOS loads `stage1.asm` at `0x7C00`. It initializes segments and loads **Stage 2** from the disk (Sectors 2-3) into `0x7E00`.
2.  **Stage 2**:
    *   Queries the BIOS for VESA/VBE information.
    *   Sets the video mode to **1024x768x32bpp**.
    *   Saves the **Linear Framebuffer (LFB)** address at `0x8000`.
    *   Loads the Kernel from the disk (Sector 4 onwards) into `0x1000`.
    *   Switches to **32-bit Protected Mode**.
3.  **Kernel Entry**: `kernel_entry.asm` executes, setting up the final stack and calling `kmain()`.
4.  **Graphic Terminal**: The kernel reads the VBE info from `0x8000` and uses the `terminal.c` driver to render text by drawing font bitmaps directly onto the LFB pixels.

## Prerequisites

- **NASM**: The Netwide Assembler.
- **i686-elf-gcc**: A cross-compiler (to avoid host headers/PIE/SSP).
- **GNU Binutils**: (`i686-elf-ld`) for linking.
- **QEMU**: For emulation.
- **Truncate**: (Standard on Linux) for disk image padding.

## Getting Started

### 1. Build the OS Image
```bash
make
```
This generates `os-image.bin`, which is padded to 20 sectors to ensure the BIOS read operations succeed.

### 2. Run the OS
```bash
make run
```
You should see a dark blue screen with green and white text rendered via the graphical terminal driver.

### 3. Cleanup
```bash
make clean
```
