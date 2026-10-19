/* Tells the linker how to lay out the kernel in memory. */

OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

/* One loadable segment per kind of memory, so each can get the right
   permissions (requests and data: read/write, text: execute, rodata: read). */
PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
    /* Load the kernel in the top 2 GiB of the address space ("higher half").
       This leaves the lower half free for user programs later on, and is
       required by -mcmodel=kernel and Limine base revision 3. */
    . = 0xffffffff80000000;

    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .text : {
        *(.text .text.*)
    } :text

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data : {
        *(.data .data.*)
    } :data

    /* .bss (zero-initialized data) goes last so it takes no space in the file. */
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}
