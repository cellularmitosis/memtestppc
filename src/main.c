/*
 * WAVE 4 CHECKPOINT STUB — throwaway. Replaced by the real ported memtest86+
 * v2.00 main.c in Wave 5.
 *
 * Purpose: prove the test engine + error reporting link and that error.c's
 * PRINTMODE_ADDRESSES error table renders (white-on-red rows) — the M1 headline.
 * It draws the boot screen via init(), then injects a few fake errors via the
 * verbatim error() path (common_err -> the scrolling Tst/Pass/Failing-Address
 * table + print_err_counts' 0x47 red fill -> fb_render_cell).
 *
 * Provides globals/functions owned by later waves so the checkpoint links:
 *   v, tseq[], bail, segs, test, nticks, test_ticks, beepmode - main.c (Wave 5)
 *   find_ticks                                                 - main.c (Wave 5)
 *   adj_mem                                                    - config.c (Wave 6)
 *   page_of / insertaddress                                    - real defs later
 */

#include "test.h"
#include "display.h"
#include "ofw.h"

struct vars variables;
struct vars * const v = &variables;
volatile int bail;
volatile int segs;
volatile int test;
int nticks, test_ticks;
/* (beepmode is defined by init.c) */

/* Test-pattern globals referenced by test.c (defined in main.c upstream). */
volatile ulong *p = 0;
ulong p1 = 0, p2 = 0, p0 = 0;

/* Real v2.00 test table {cache, pat, iter, ticks, errors, msg}. */
struct tseq tseq[] = {
    {0,  5,  3,    0, 0, "[Address test, walking ones, uncached]"},
    {1,  6,  3,    2, 0, "[Address test, own address]           "},
    {1,  0,  3,   14, 0, "[Moving inversions, ones & zeros]     "},
    {1,  1,  2,   80, 0, "[Moving inversions, 8 bit pattern]    "},
    {1, 10, 60,  300, 0, "[Moving inversions, random pattern]   "},
    {1,  7, 64,   66, 0, "[Block move, 64 moves]                "},
    {1,  2,  2,  320, 0, "[Moving inversions, 32 bit pattern]   "},
    {1,  9, 40,  120, 0, "[Random number sequence]              "},
    {1,  3,  4, 1920, 0, "[Modulo 20, random pattern]           "},
    {1,  8,  1,    2, 0, "[Bit fade test, 90 min, 2 patterns]   "},
    {0,  0,  0,    0, 0, 0}
};

void find_ticks(void) { /* Wave-5 stub */ }
void adj_mem(void) { /* Wave-6 stub */ }
unsigned long page_of(void *ptr) { return ((unsigned long)ptr) >> 12; }
int insertaddress(ulong a) { (void)a; return 0; }

int main(void)
{
    init();                 /* fb_init + verbatim TUI + PPC cpu/cache + mem_size() */

    /* set_defaults() does this in Wave 5; init just enough error state here. */
    v->printmode = PRINTMODE_ADDRESSES;
    v->msg_line = LINE_SCROLL - 1;
    v->ecount = 0;
    v->pass = 0;
    v->erri.hdr_flag = 0;
    test = 0;

    /* Inject fake errors to verify the error table + white-on-red rows. */
    error((ulong *)0x00100000, 0x00000000, 0x00000010);
    error((ulong *)0x00100040, 0xffffffff, 0xfffffffd);
    error((ulong *)0x02000000, 0xaaaaaaaa, 0xaabaaaaa);
    error((ulong *)0x07ffff00, 0x12345678, 0x12345679);

    for (;;) ;
    return 0;
}
