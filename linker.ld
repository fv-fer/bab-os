ENTRY(_start)

SECTIONS
{
    /* The kernel will be loaded at 0x10000 physical, 
       but linked to 3GB + 0x10000 virtual. */
    . = 0xC0010000;

    _kernel_virtual_start = .;
    _kernel_physical_start = . - 0xC0000000;

    /* Boot section: contains paging structures that must not be zeroed */
    .boot : AT(ADDR(.boot) - 0xC0000000)
    {
        *(.boot)
    }

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
        _bss_start = .;
        *(.bss)
        *(COMMON)
        _bss_end = .;
    }

    _kernel_virtual_end = .;
    _kernel_physical_end = . - 0xC0000000;
}
