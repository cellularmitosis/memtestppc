/*
 * memtestppc+ test algorithms
 *
 * Ported from memtest86+ v5.01 test.c (GPL v2)
 * All x86 inline assembly replaced with portable C.
 * Original by Chris Brady and Samuel Demeulemeester.
 */

#include "memtest.h"

static inline ulong roundup(ulong value, ulong mask)
{
    return (value + mask) & ~mask;
}

/*
 * Memory address test, walking ones
 */
void addr_tst1(void)
{
    int i, j;
    volatile ulong *p, *pt, *end;
    ulong bad, mask, bank, p1;

    for (p1 = 0, j = 0; j < 2; j++) {
        hprint(LINE_PAT, COL_PAT, p1);

        p = (ulong *)roundup((ulong)v->map[0].start, 0x1ffff);
        *p = p1;

        p1 = ~p1;
        end = v->map[segs - 1].end;
        for (i = 0; i < 100; i++) {
            mask = 4;
            do {
                pt = (ulong *)((ulong)p | mask);
                if (pt == p) { mask <<= 1; continue; }
                if (pt >= end) break;
                *pt = p1;
                if ((bad = *p) != ~p1) {
                    ad_err1((ulong *)p, (ulong *)mask, bad, ~p1);
                    i = 1000;
                }
                mask <<= 1;
            } while (mask);
        }
        do_tick();
        BAILR
    }

    if (v->pmap[v->msegs - 1].end > (0x800000 >> 12))
        bank = 0x100000;
    else
        bank = 0x40000;

    for (p1 = 0, j = 0; j < 2; j++) {
        hprint(LINE_PAT, COL_PAT, p1);
        for (i = 0; i < segs; i++) {
            p = v->map[i].start;
            p = (ulong *)roundup((ulong)p, bank - 1);
            end = v->map[i].end;
            while (p < end && p > v->map[i].start && p != 0) {
                *p = p1;
                p1 = ~p1;
                for (mask = 4; mask; mask <<= 1) {
                    pt = (ulong *)((ulong)p | mask);
                    if (pt == p) continue;
                    if (pt >= end) break;
                    *pt = p1;
                    if ((bad = *p) != ~p1) {
                        ad_err1((ulong *)p, (ulong *)mask, bad, ~p1);
                    }
                }
                if (p + bank > p)
                    p += bank;
                else
                    p = end;
                p1 = ~p1;
            }
        }
        do_tick();
        BAILR
        p1 = ~p1;
    }
}

/*
 * Memory address test, own address
 */
void addr_tst2(void)
{
    int j, done;
    volatile ulong *p, *pe, *end, *start;

    cprint(LINE_PAT, COL_PAT, "address ");

    /* Write each address with its own address */
    for (j = 0; j < segs; j++) {
        start = v->map[j].start;
        end = v->map[j].end;
        pe = start;
        p = start;
        done = 0;
        do {
            do_tick();
            BAILR
            if (pe + SPINSZ > pe && pe != 0)
                pe += SPINSZ;
            else
                pe = end;
            if (pe >= end) { pe = end; done++; }
            if (p == pe) break;

            for (; p <= pe; p++)
                *p = (ulong)p;

            p = pe + 1;
        } while (!done);
    }

    /* Verify each address contains its own address */
    for (j = 0; j < segs; j++) {
        start = v->map[j].start;
        end = v->map[j].end;
        pe = start;
        p = start;
        done = 0;
        do {
            do_tick();
            BAILR
            if (pe + SPINSZ > pe && pe != 0)
                pe += SPINSZ;
            else
                pe = end;
            if (pe >= end) { pe = end; done++; }
            if (p == pe) break;

            for (; p <= pe; p++) {
                ulong bad;
                if ((bad = *p) != (ulong)p)
                    ad_err2((ulong *)p, bad);
            }

            p = pe + 1;
        } while (!done);
    }
}

/*
 * Moving inversions test with patterns p1/p2
 */
void movinv1(int iter, ulong p1, ulong p2)
{
    int i, j, done;
    volatile ulong *p, *pe;
    ulong *start, *end;
    ulong bad;

    hprint(LINE_PAT, COL_PAT, p1);

    /* Initialize memory with p1 */
    for (j = 0; j < segs; j++) {
        start = v->map[j].start;
        end = v->map[j].end;
        pe = start;
        p = start;
        done = 0;
        do {
            do_tick();
            BAILR
            if (pe + SPINSZ > pe && pe != 0)
                pe += SPINSZ;
            else
                pe = end;
            if (pe >= end) { pe = end; done++; }
            if (p == pe) break;

            for (; p <= pe; p++)
                *p = p1;

            p = pe + 1;
        } while (!done);
    }

    /* Check p1, write p2 (forward), then check p2, write p1 (reverse) */
    for (i = 0; i < iter; i++) {
        /* Forward pass */
        for (j = 0; j < segs; j++) {
            start = v->map[j].start;
            end = v->map[j].end;
            pe = start;
            p = start;
            done = 0;
            do {
                do_tick();
                BAILR
                if (pe + SPINSZ > pe && pe != 0)
                    pe += SPINSZ;
                else
                    pe = end;
                if (pe >= end) { pe = end; done++; }
                if (p == pe) break;

                for (; p <= pe; p++) {
                    if ((bad = *p) != p1)
                        error((ulong *)p, p1, bad);
                    *p = p2;
                }

                p = pe + 1;
            } while (!done);
        }

        /* Reverse pass */
        for (j = segs - 1; j >= 0; j--) {
            start = v->map[j].start;
            end = v->map[j].end;
            pe = end;
            p = end;
            done = 0;
            do {
                do_tick();
                BAILR
                if (pe - SPINSZ < pe && pe != 0)
                    pe -= SPINSZ;
                else {
                    pe = start;
                    done++;
                }
                if (pe < start || pe > end) {
                    pe = start;
                    done++;
                }
                if (p == pe) break;

                do {
                    if ((bad = *p) != p2)
                        error((ulong *)p, p2, bad);
                    *p = p1;
                } while (--p >= pe);

                p = pe - 1;
            } while (!done);
        }
    }
}

/*
 * Moving inversions, 32-bit shifting pattern
 */
void movinv32(int iter, ulong p1, ulong lb, ulong hb, int sval, int off)
{
    int i, j, k, done;
    volatile ulong *p, *pe;
    ulong *start, *end;
    ulong pat, bad;

    hprint(LINE_PAT, COL_PAT, p1);

    /* Initialize memory */
    for (j = 0; j < segs; j++) {
        start = v->map[j].start;
        end = v->map[j].end;
        pe = start;
        p = start;
        done = 0;
        k = off;
        pat = p1;
        do {
            do_tick();
            BAILR
            if (pe + SPINSZ > pe && pe != 0)
                pe += SPINSZ;
            else
                pe = end;
            if (pe >= end) { pe = end; done++; }
            if (p == pe) break;

            while (p <= pe) {
                *p = pat;
                if (++k >= 32) { pat = lb; k = 0; }
                else { pat = (pat << 1) | sval; }
                p++;
            }

            p = pe + 1;
        } while (!done);
    }

    /* Forward check and invert */
    for (i = 0; i < iter; i++) {
        for (j = 0; j < segs; j++) {
            start = v->map[j].start;
            end = v->map[j].end;
            pe = start;
            p = start;
            done = 0;
            k = off;
            pat = p1;
            do {
                do_tick();
                BAILR
                if (pe + SPINSZ > pe && pe != 0)
                    pe += SPINSZ;
                else
                    pe = end;
                if (pe >= end) { pe = end; done++; }
                if (p == pe) break;

                while (p <= pe) {
                    if ((bad = *p) != pat)
                        error((ulong *)p, pat, bad);
                    *p = ~pat;
                    if (++k >= 32) { pat = lb; k = 0; }
                    else { pat = (pat << 1) | sval; }
                    p++;
                }

                p = pe + 1;
            } while (!done);
        }

        /* Reverse pass */
        if (--k < 0) k = 31;
        for (pat = lb, i = 0; i < k; i++) {
            pat = (pat << 1) | sval;
        }
        k++;

        for (j = segs - 1; j >= 0; j--) {
            start = v->map[j].start;
            end = v->map[j].end;
            p = end;
            pe = end;
            done = 0;
            do {
                do_tick();
                BAILR
                if (pe - SPINSZ < pe && pe != 0)
                    pe -= SPINSZ;
                else { pe = start; done++; }
                if (pe < start || pe > end) { pe = start; done++; }
                if (p == pe) break;

                while (p >= pe) {
                    if ((bad = *p) != ~pat)
                        error((ulong *)p, ~pat, bad);
                    *p = pat;
                    if (--k <= 0) { pat = hb; k = 32; }
                    else { pat = (pat >> 1) | (sval << 31); }
                    if (p == pe) break;
                    p--;
                }

                p = pe - 1;
            } while (!done);
        }
    }
}

/*
 * Random number generator (Marsaglia's MWC)
 */
static ulong rand_x = 521288629;
static ulong rand_y = 362436069;

void rand_seed(unsigned int seed1, unsigned int seed2)
{
    rand_x = seed1;
    rand_y = seed2;
}

ulong rand_next(void)
{
    ulong t;
    rand_x = 69069 * rand_x + 1;
    rand_y ^= (rand_y << 13);
    rand_y ^= (rand_y >> 17);
    rand_y ^= (rand_y << 5);
    t = rand_x + rand_y;
    return t;
}

/*
 * Moving inversions with random data
 */
void movinvr(void)
{
    int i, j, done;
    volatile ulong *p, *pe;
    ulong *start, *end;
    unsigned int seed1, seed2;
    ulong bad, num;

    seed1 = (unsigned int)mftb();
    seed2 = seed1 ^ 0xa5a5a5a5;

    hprint(LINE_PAT, COL_PAT, seed1);
    rand_seed(seed1, seed2);

    /* Fill memory with random data */
    for (j = 0; j < segs; j++) {
        start = v->map[j].start;
        end = v->map[j].end;
        pe = start;
        p = start;
        done = 0;
        do {
            do_tick();
            BAILR
            if (pe + SPINSZ > pe && pe != 0)
                pe += SPINSZ;
            else
                pe = end;
            if (pe >= end) { pe = end; done++; }
            if (p == pe) break;

            for (; p <= pe; p++)
                *p = rand_next();

            p = pe + 1;
        } while (!done);
    }

    /* Check and invert */
    for (i = 0; i < 2; i++) {
        rand_seed(seed1, seed2);
        for (j = 0; j < segs; j++) {
            start = v->map[j].start;
            end = v->map[j].end;
            pe = start;
            p = start;
            done = 0;
            do {
                do_tick();
                BAILR
                if (pe + SPINSZ > pe && pe != 0)
                    pe += SPINSZ;
                else
                    pe = end;
                if (pe >= end) { pe = end; done++; }
                if (p == pe) break;

                for (; p <= pe; p++) {
                    num = rand_next();
                    if (i) num = ~num;
                    if ((bad = *p) != num)
                        error((ulong *)p, num, bad);
                    *p = ~num;
                }

                p = pe + 1;
            } while (!done);
        }
    }
}

/*
 * Modulo-20 test
 */
void modtst(int offset, int iter, ulong p1, ulong p2)
{
    int j, k, l, done;
    volatile ulong *p, *pe;
    ulong *start, *end;
    ulong bad;

    hprint(LINE_PAT, COL_PAT - 2, p1);
    cprint(LINE_PAT, COL_PAT + 6, "-");
    dprint(LINE_PAT, COL_PAT + 7, offset, 2, 1);

    /* Write every nth location with pattern */
    for (j = 0; j < segs; j++) {
        start = v->map[j].start;
        end = v->map[j].end - MOD_SZ;
        pe = start;
        p = start + offset;
        done = 0;
        do {
            do_tick();
            BAILR
            if (pe + SPINSZ > pe && pe != 0)
                pe += SPINSZ;
            else
                pe = end;
            if (pe >= end) { pe = end; done++; }
            if (p == pe) break;

            for (; p <= pe; p += MOD_SZ)
                *p = p1;
        } while (!done);
    }

    /* Fill rest with complement */
    for (l = 0; l < iter; l++) {
        for (j = 0; j < segs; j++) {
            start = v->map[j].start;
            end = v->map[j].end;
            pe = start;
            p = start;
            done = 0;
            k = 0;
            do {
                do_tick();
                BAILR
                if (pe + SPINSZ > pe && pe != 0)
                    pe += SPINSZ;
                else
                    pe = end;
                if (pe >= end) { pe = end; done++; }
                if (p == pe) break;

                for (; p <= pe; p++) {
                    if (k != offset) *p = p2;
                    if (++k > MOD_SZ - 1) k = 0;
                }

                p = pe + 1;
            } while (!done);
        }
    }

    /* Check every nth location */
    for (j = 0; j < segs; j++) {
        start = v->map[j].start;
        end = v->map[j].end - MOD_SZ;
        pe = start;
        p = start + offset;
        done = 0;
        do {
            do_tick();
            BAILR
            if (pe + SPINSZ > pe && pe != 0)
                pe += SPINSZ;
            else
                pe = end;
            if (pe >= end) { pe = end; done++; }
            if (p == pe) break;

            for (; p <= pe; p += MOD_SZ) {
                if ((bad = *p) != p1)
                    error((ulong *)p, p1, bad);
            }
        } while (!done);
    }
}

/*
 * Block move test
 */
void block_move(int iter)
{
    int i, j, done;
    ulong len;
    volatile ulong *p, *pe;
    ulong pp;
    ulong *start, *end;

    cprint(LINE_PAT, COL_PAT - 2, "          ");

    /* Initialize: write a rotating 64-byte block pattern */
    for (j = 0; j < segs; j++) {
        start = v->map[j].start;
        end = v->map[j].end + 1;
        pe = start;
        p = start;
        done = 0;
        do {
            do_tick();
            BAILR
            if (pe + SPINSZ > pe && pe != 0)
                pe += SPINSZ;
            else
                pe = end;
            if (pe >= end) { pe = end; done++; }
            if (p == pe) break;

            len = ((ulong)pe - (ulong)p) / 64;
            {
                ulong pat = 1;
                ulong *pp2 = (ulong *)p;
                ulong cnt;
                for (cnt = 0; cnt < len; cnt++) {
                    ulong inv = ~pat;
                    pp2[0]  = pat; pp2[1]  = pat;
                    pp2[2]  = pat; pp2[3]  = pat;
                    pp2[4]  = inv; pp2[5]  = inv;
                    pp2[6]  = pat; pp2[7]  = pat;
                    pp2[8]  = pat; pp2[9]  = pat;
                    pp2[10] = inv; pp2[11] = inv;
                    pp2[12] = pat; pp2[13] = pat;
                    pp2[14] = inv; pp2[15] = inv;
                    /* Rotate left with carry (32-bit) */
                    pat = (pat << 1) | (pat >> 31);
                    pp2 += 16;
                }
                p = (ulong *)pp2;
            }
        } while (!done);
    }

    /* Move data around */
    for (j = 0; j < segs; j++) {
        start = v->map[j].start;
        end = v->map[j].end + 1;
        pe = start;
        p = start;
        done = 0;
        do {
            if (pe + SPINSZ > pe && pe != 0)
                pe += SPINSZ;
            else
                pe = end;
            if (pe >= end) { pe = end; done++; }
            if (p == pe) break;

            pp = (ulong)p + (((ulong)pe - (ulong)p) / 2);
            len = ((ulong)pe - (ulong)p) / 8;

            for (i = 0; i < iter; i++) {
                do_tick();
                BAILR
                /* Move first half to second half */
                memcpy((void *)pp, (void *)p, len * 4);
                /* Move second half (minus 32 bytes) to first half + 32 */
                memcpy((void *)((ulong)p + 32), (void *)pp, (len - 8) * 4);
                /* Move last 32 bytes to start */
                memcpy((void *)p, (void *)((ulong)pp + (len - 8) * 4), 32);
            }

            p = pe;
        } while (!done);
    }

    /* Verify: adjacent words should match */
    for (j = 0; j < segs; j++) {
        start = v->map[j].start;
        end = v->map[j].end + 1;
        pe = start;
        p = start;
        done = 0;
        do {
            do_tick();
            BAILR
            if (pe + SPINSZ > pe && pe != 0)
                pe += SPINSZ;
            else
                pe = end;
            if (pe >= end) { pe = end; done++; }
            if (p == pe) break;
            pe -= 2;

            while (p <= pe) {
                if (p[0] != p[1])
                    error((ulong *)p, p[0], p[1]);
                p += 2;
            }
        } while (!done);
    }
}

/*
 * Bit fade test: fill memory, wait, then check
 */
void bit_fade_fill(ulong p1)
{
    int j, done;
    volatile ulong *p, *pe;
    ulong *start, *end;

    hprint(LINE_PAT, COL_PAT, p1);

    for (j = 0; j < segs; j++) {
        start = v->map[j].start;
        end = v->map[j].end;
        pe = start;
        p = start;
        done = 0;
        do {
            do_tick();
            BAILR
            if (pe + SPINSZ > pe && pe != 0)
                pe += SPINSZ;
            else
                pe = end;
            if (pe >= end) { pe = end; done++; }
            if (p == pe) break;

            for (; p < pe; p++)
                *p = p1;

            p = pe + 1;
        } while (!done);
    }
}

void bit_fade_chk(ulong p1)
{
    int j, done;
    volatile ulong *p, *pe;
    ulong *start, *end;
    ulong bad;

    for (j = 0; j < segs; j++) {
        start = v->map[j].start;
        end = v->map[j].end;
        pe = start;
        p = start;
        done = 0;
        do {
            do_tick();
            BAILR
            if (pe + SPINSZ > pe && pe != 0)
                pe += SPINSZ;
            else
                pe = end;
            if (pe >= end) { pe = end; done++; }
            if (p == pe) break;

            for (; p < pe; p++) {
                if ((bad = *p) != p1)
                    error((ulong *)p, p1, bad);
            }

            p = pe + 1;
        } while (!done);
    }
}
