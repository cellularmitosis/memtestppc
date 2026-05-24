OUTPUT_FORMAT("elf32-powerpc")
OUTPUT_ARCH(powerpc)
ENTRY(_start)

SECTIONS
{
    . = 0x01000000;

    .text : { *(.text) *(.text.*) *(.rodata) *(.rodata.*) }
    .data : ALIGN(4) { *(.eh_frame) *(.data) *(.data.*) *(.sdata) *(.sdata.*) }
    .bss : ALIGN(4) {
        __bss_start = .;
        *(.bss) *(.bss.*) *(.sbss) *(.sbss.*) *(COMMON)
        __bss_end = .;
    }
    /DISCARD/ : { *(.comment) *(.note.*) }
}
