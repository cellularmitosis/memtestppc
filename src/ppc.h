#ifndef PPC_H
#define PPC_H

/*
 * PowerPC instruction intrinsics for memtestppc+.
 *
 * These are PPC-native helpers with no x86/memtest86+ counterpart — they
 * replace the x86 leaf operations (rdtsc timing, cache control) that the
 * upstream code used inline. Kept in their own header so the ported upstream
 * files (test.h etc.) can include them without carrying x86 asm.
 */

/* Time Base — 64-bit free-running counter; read as two 32-bit halves. */
static inline unsigned long mftb(void)
{
    unsigned long tb;
    __asm__ __volatile__("mftb %0" : "=r"(tb));
    return tb;
}

static inline unsigned long mftbu(void)
{
    unsigned long tbu;
    __asm__ __volatile__("mftbu %0" : "=r"(tbu));
    return tbu;
}

/* Data Cache Block Flush — push a line back to memory (used around tests). */
static inline void dcbf(void *addr)
{
    __asm__ __volatile__("dcbf 0,%0" : : "r"(addr) : "memory");
}

static inline void sync(void)
{
    __asm__ __volatile__("sync" : : : "memory");
}

static inline void isync(void)
{
    __asm__ __volatile__("isync" : : : "memory");
}

#endif /* PPC_H */
