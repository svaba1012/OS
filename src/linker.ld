ENTRY(_start)
OUTPUT_FORMAT(binary)
SECTIONS
{   
    . = 1M;
    .text : ALIGN(4k)
    {
        *(.text)
    }
    .rodata : ALIGN(4k)
    {
        *(.rodata)
    }
    .data : ALIGN(4k)
    {
        *(.data)
    }
    .bss :  ALIGN(4k)
    {
        *(COMMON)
        *(.bss)
    }
    .asm :  ALIGN(4k)
    {
        *(.asm)
    }

}