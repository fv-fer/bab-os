ENTRY(_start)

SECTIONS
{
    /* The kernel will be loaded at 0x1000 physical, 
       but linked to 3GB + 0x1000 virtual. */
    . = 0xC0001000;

    _kernel_virtual_start = .;
    _kernel_physical_start = . - 0xC0000000;

    .text : AT(ADDR(.text) - 0xC0000000)
    {
        *(.text)
    }

    .rodata : AT(ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata)
    }

    .data : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - 0xC0000000)
    {
        *(.bss)
        *(COMMON)
    }

    _kernel_virtual_end = .;
    _kernel_physical_end = . - 0xC0000000;
}
