.global _start

.section .text.boot

_start:
    // Raspberry Pi 5 starts in AArch64. Only core 0 should run our kernel.
    mrs x0, mpidr_el1
    and x0, x0, #3
    cbz x0, core0_start

secondary_core_hang:
    wfe
    b secondary_core_hang

core0_start:
    // Put stack safely above the loaded kernel area.
    ldr x0, =0x400000
    mov sp, x0

    // Clear .bss so C static globals start at 0.
    ldr x0, =__bss_start
    ldr x1, =__bss_end
clear_bss:
    cmp x0, x1
    b.ge bss_done
    str xzr, [x0], #8
    b clear_bss

bss_done:
    bl kernel_main

hang:
    wfe
    b hang
