/*
 * WAVE 3 CHECKPOINT STUB — throwaway. Replaced by the real ported memtest86+
 * v2.00 main.c in Wave 5.
 *
 * Purpose: prove init.c (verbatim TUI header + PPC CPU/cache/clock detection)
 * and memsize.c (OF /memory discovery + 1MB-chunk claim) render a real boot
 * screen. init() does it all — it calls mem_size() itself (init.c:223), zeroes
 * tseq[].errors, and calls find_ticks() — so main() just calls init().
 *
 * Provides globals/functions owned by later waves so the checkpoint links:
 *   v          - struct vars (real def comes with main.c, Wave 5)
 *   tseq[]     - the test table (owned by main.c, Wave 5) — real v2.00 entries
 *   find_ticks - owned by main.c (Wave 5); stubbed (tick bars stay 0)
 *   adj_mem    - owned by config.c (Wave 6); stubbed (mem_size calls it)
 */

#include "test.h"
#include "display.h"
#include "ofw.h"

struct vars variables;
struct vars * const v = &variables;

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

void find_ticks(void) { /* Wave-5 stub: leave tick counters at 0 */ }
void adj_mem(void) { /* Wave-6 stub: mem_size() already set selected_pages */ }

int main(void)
{
    init();          /* fb_init + verbatim TUI + PPC cpu/cache + mem_size() */
    for (;;) ;
    return 0;
}
