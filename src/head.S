/*
 * memtestppc+ entry point
 *
 * Open Firmware calls us with:
 *   r5 = pointer to OF client interface function
 *
 * We save it, set up a stack, and call main().
 * The C startup code retrieves the CIF pointer from the known location.
 */

.text
.globl _start
.globl _ofw_cif_store
_start:
    /* Save LR */
    mflr    r0

    /*
     * We need to save r5 (OF client interface pointer).
     * Use bl/mflr trick to find our own address (PC-relative).
     */
    bl      1f
1:  mflr    r8              /* r8 = address of label 1 */

    /* Store r5 to ofw_cif_store (use offset from label 1) */
    addis   r9, r8, (_ofw_cif_store - 1b)@ha
    stw     r5, (_ofw_cif_store - 1b)@l(r9)

    /* Set up stack in our BSS (OF mapped it when loading the ELF) */
    addis   r1, r8, (_stack_top - 1b)@ha
    addi    r1, r1, (_stack_top - 1b)@l
    /* Align to 16 bytes */
    rlwinm  r1, r1, 0, 0, 27
    stwu    r1, -64(r1)

    /* Clear BSS using PC-relative addressing */
    addis   r3, r8, (__bss_start - 1b)@ha
    addi    r3, r3, (__bss_start - 1b)@l
    addis   r4, r8, (__bss_end - 1b)@ha
    addi    r4, r4, (__bss_end - 1b)@l
    li      r0, 0
2:
    cmpw    r3, r4
    bge     3f
    stw     r0, 0(r3)
    addi    r3, r3, 4
    b       2b
3:

    /* Call main() */
    bl      main

    /* Shouldn't return, but just in case */
4:  b       4b


.section .data
.align 2
_ofw_cif_store:
    .long 0

.section .bss
.align 4
_stack_bottom:
    .space 16384
_stack_top:
