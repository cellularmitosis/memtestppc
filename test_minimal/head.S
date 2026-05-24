/*
 * Minimal OF test — save all possible CIF register candidates
 */
.text
.globl _start
.globl _ofw_r3
.globl _ofw_r4
.globl _ofw_r5
.globl _ofw_r6
.globl _ofw_r7

_start:
    /* Save all registers that might contain CIF */
    bl      1f
1:  mflr    r8

    addis   r9, r8, (_ofw_r3 - 1b)@ha
    stw     r3, (_ofw_r3 - 1b)@l(r9)

    addis   r9, r8, (_ofw_r4 - 1b)@ha
    stw     r4, (_ofw_r4 - 1b)@l(r9)

    addis   r9, r8, (_ofw_r5 - 1b)@ha
    stw     r5, (_ofw_r5 - 1b)@l(r9)

    addis   r9, r8, (_ofw_r6 - 1b)@ha
    stw     r6, (_ofw_r6 - 1b)@l(r9)

    addis   r9, r8, (_ofw_r7 - 1b)@ha
    stw     r7, (_ofw_r7 - 1b)@l(r9)

    /* Set up stack */
    lis     r1, 0x00FF
    stwu    r1, -64(r1)

    bl      main

2:  b       2b

.section .data
.align 2
_ofw_r3:
    .long 0
_ofw_r4:
    .long 0
_ofw_r5:
    .long 0
_ofw_r6:
    .long 0
_ofw_r7:
    .long 0
