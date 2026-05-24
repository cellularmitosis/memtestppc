/*
 * memtestppc+ - Memory tester for PowerPC Macs
 *
 * Inspired by memtest86+ v5.01 by Samuel Demeulemeester and Chris Brady.
 * Adapted for PowerPC / Open Firmware by the memtestppc+ project.
 *
 * Released under GPL v2.
 */

#include "memtest.h"
#include "display.h"
#include "ofw.h"

#define VERSION "0.01"
#define DEFTESTS 9

/* Global state */
static struct vars variables;
struct vars *const v = &variables;
volatile int bail;
volatile int segs;
volatile int test;
int test_ticks, nticks;
int v_msg_line;

static unsigned long tb_freq;     /* timebase ticks per second */
static unsigned long start_tb_h, start_tb_l;  /* test start time */
static int pass_flag;
static int c_iter;
static ulong total_mem_mb;
static int pass_ticks, total_ticks;

/* CPU info */
static unsigned long cpu_pvr;
static unsigned long cpu_clock_mhz;
static const char *cpu_name;
static int l1_cache_kb;
static int l2_cache_kb;

static ofw_ihandle_t stdin_ih = -1;
static void check_key(void);

struct tseq tseq[] = {
    {1, 1,  0,   6, 0, "[Address test, walking ones, no cache] "},
    {1, 1,  1,   6, 0, "[Address test, own address Sequential] "},
    {1, 1,  2,   6, 0, "[Address test, own address Parallel]   "},
    {1, 1,  3,   6, 0, "[Moving inversions, 1s & 0s Parallel]  "},
    {1, 1,  5,   3, 0, "[Moving inversions, 8 bit pattern]     "},
    {1, 1,  6,  30, 0, "[Moving inversions, random pattern]    "},
    {1, 1,  7,  81, 0, "[Block move]                           "},
    {1, 1,  8,   3, 0, "[Moving inversions, 32 bit pattern]    "},
    {1, 1,  9,  48, 0, "[Random number sequence]               "},
    {1, 1, 10,   6, 0, "[Modulo 20, Random pattern]            "},
    {0, 1, 11, 240, 0, "[Bit fade test, 2 patterns]            "},
    {1, 0,  0,   0, 0, (char *)0}
};

/*
 * Utility functions
 */
int strlen(const char *s)
{
    int i = 0;
    while (*s++) i++;
    return i;
}

void *memset(void *s, int c, unsigned long n)
{
    unsigned char *p = (unsigned char *)s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *dest, const void *src, unsigned long n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) *d++ = *s++;
    return dest;
}

/*
 * Read the 64-bit timebase as a pair of 32-bit values
 */
static void read_tb(unsigned long *hi, unsigned long *lo)
{
    unsigned long tbh, tbl, tbh2;
    do {
        tbh = mftbu();
        tbl = mftb();
        tbh2 = mftbu();
    } while (tbh != tbh2);
    *hi = tbh;
    *lo = tbl;
}

/*
 * Get elapsed seconds since test start
 */
static unsigned long elapsed_seconds(void)
{
    unsigned long hi, lo;
    unsigned long long start, now;

    read_tb(&hi, &lo);
    start = ((unsigned long long)start_tb_h << 32) | start_tb_l;
    now = ((unsigned long long)hi << 32) | lo;

    if (tb_freq == 0) return 0;
    return (unsigned long)((now - start) / tb_freq);
}

/*
 * Update the elapsed time display (HH:MM:SS)
 */
static void update_time(void)
{
    unsigned long secs = elapsed_seconds();
    unsigned long h = secs / 3600;
    unsigned long m = (secs / 60) % 60;
    unsigned long s = secs % 60;

    dprint(LINE_TIME, 67, h, 3, 0);
    cprint(LINE_TIME, 70, ":");
    dprint(LINE_TIME, 71, m, 2, 1);
    cprint(LINE_TIME, 73, ":");
    dprint(LINE_TIME, 74, s, 2, 1);
}

/*
 * Progress tick — called periodically by test routines
 */
void do_tick(void)
{
    static const char spin[] = "|/-\\";
    static int spin_idx;
    int pct, bars, i;

    nticks++;
    total_ticks++;

    /* Update test progress bar (line 2) */
    if (test_ticks > 0) {
        pct = (nticks * 100) / test_ticks;
        if (pct > 100) pct = 100;
        bars = (pct * BAR_SIZE) / 100;
        for (i = 0; i < bars && i < BAR_SIZE; i++)
            cprint(2, COL_MID + 9 + i, "#");
        dprint(2, COL_MID + 4, pct, 3, 0);
    }

    /* Update pass progress bar (line 1) */
    if (pass_ticks > 0) {
        pct = (total_ticks * 100) / pass_ticks;
        if (pct > 100) pct = 100;
        bars = (pct * BAR_SIZE) / 100;
        for (i = 0; i < bars && i < BAR_SIZE; i++)
            cprint(1, COL_MID + 9 + i, "#");
        dprint(1, COL_MID + 4, pct, 3, 0);
    }

    /* Spinner */
    spin_idx = (spin_idx + 1) & 3;
    cplace(LINE_CPU, 7, spin[spin_idx]);

    update_time();

    check_key();
}

/*
 * Error reporting
 */
static int err_count = 0;

void error(ulong *adr, ulong good, ulong bad)
{
    ulong xor_val = good ^ bad;
    int i;

    err_count++;
    v->ecount++;
    dprint(LINE_INFO, 72, v->ecount, 6, 0);

    scroll();

    /*
     * Paint the whole error row white-on-red (attr 0x47), matching
     * memtest86+ v5.01 (error.c, cols 1..76). Our vga_buf is a shadow
     * buffer — unlike x86's memory-mapped VGA, the color only appears once
     * we re-render each cell, so do that here before printing the fields.
     */
    for (i = 1; i <= 76; i++) {
        vga_buf[(v_msg_line * SCREEN_WIDTH + i) * 2 + 1] = 0x47;
        display_render_cell(v_msg_line, i);
    }

    hprint(v_msg_line, 1, (ulong)adr);
    hprint(v_msg_line, 15, good);
    hprint(v_msg_line, 27, bad);
    hprint(v_msg_line, 39, xor_val);
    dprint(v_msg_line, 51, test, 2, 1);
    dprint(v_msg_line, 55, v->pass, 4, 0);
}

void ad_err1(ulong *adr1, ulong *adr2, ulong good, ulong bad)
{
    error(adr1, good, bad);
}

void ad_err2(ulong *adr, ulong bad)
{
    error(adr, (ulong)adr, bad);
}

/*
 * Keyboard input via Open Firmware
 */
static void check_key(void)
{
    char c;
    if (stdin_ih == -1) return;

    if (ofw_read(stdin_ih, &c, 1) > 0) {
        if (c == 27) { /* ESC */
            cprint(LINE_RANGE, COL_MID + 23, "Rebooting...");
            ofw_reset();
            ofw_exit(); /* fallback if reset-all not supported */
        }
    }
}

/*
 * Identify the CPU from the PVR
 */
static void identify_cpu(void)
{
    unsigned long pvr;
    ofw_handle_t cpu_node;
    unsigned long clock_freq, l1d, l2sz;

    __asm__ __volatile__("mfpvr %0" : "=r"(pvr));
    cpu_pvr = pvr;

    switch (pvr >> 16) {
    case 0x0001: cpu_name = "PowerPC 601";   break;
    case 0x0003: cpu_name = "PowerPC 603";   break;
    case 0x0004: cpu_name = "PowerPC 604";   break;
    case 0x0006: cpu_name = "PowerPC 603e";  break;
    case 0x0007: cpu_name = "PowerPC 603ev"; break;
    case 0x0008: cpu_name = "PowerPC 750 (G3)"; l1_cache_kb = 32; l2_cache_kb = 256; break;
    case 0x000C: cpu_name = "PowerPC 7400 (G4)"; l1_cache_kb = 32; l2_cache_kb = 256; break;
    case 0x8000: cpu_name = "PowerPC 7450 (G4e)"; l1_cache_kb = 32; l2_cache_kb = 256; break;
    case 0x8001: cpu_name = "PowerPC 7455 (G4e)"; l1_cache_kb = 32; l2_cache_kb = 256; break;
    case 0x8002: cpu_name = "PowerPC 7457 (G4e)"; l1_cache_kb = 32; l2_cache_kb = 512; break;
    case 0x0039: cpu_name = "PowerPC 970 (G5)"; l1_cache_kb = 64; l2_cache_kb = 512; break;
    case 0x003C: cpu_name = "PowerPC 970FX (G5)"; l1_cache_kb = 64; l2_cache_kb = 512; break;
    case 0x0044: cpu_name = "PowerPC 970MP (G5)"; l1_cache_kb = 64; l2_cache_kb = 1024; break;
    default:     cpu_name = "Unknown PowerPC"; break;
    }

    cpu_node = ofw_finddevice("/cpus/PowerPC,G3@0");
    if (cpu_node == -1)
        cpu_node = ofw_finddevice("/cpus/PowerPC,750@0");
    if (cpu_node == -1)
        cpu_node = ofw_finddevice("/cpus/PowerPC,G4@0");
    if (cpu_node == -1)
        cpu_node = ofw_finddevice("/cpus/cpu@0");

    if (cpu_node != -1) {
        clock_freq = 0;
        l1d = 0;
        l2sz = 0;
        if (ofw_getprop(cpu_node, "clock-frequency", &clock_freq, sizeof(clock_freq)) >= 0)
            cpu_clock_mhz = clock_freq / 1000000;
        if (ofw_getprop(cpu_node, "d-cache-size", &l1d, sizeof(l1d)) >= 0)
            l1_cache_kb = l1d / 1024;
        if (ofw_getprop(cpu_node, "l2-cache-size", &l2sz, sizeof(l2sz)) >= 0)
            l2_cache_kb = l2sz / 1024;
    }
}

/*
 * Discover available memory from Open Firmware
 */
static void discover_memory(void)
{
    ofw_handle_t mem_node;
    unsigned long reg[64]; /* pairs of (addr, size) */
    int len, i, seg;

    mem_node = ofw_finddevice("/memory");
    if (mem_node == -1) {
        mem_node = ofw_finddevice("/memory@0");
    }
    if (mem_node == -1) {
        /* Fallback: assume 128MB starting at 0 */
        v->pmap[0].start = 0x800000 >> 12; /* skip first 8MB */
        v->pmap[0].end = 0x8000000 >> 12;
        v->msegs = 1;
        v->test_pages = v->pmap[0].end - v->pmap[0].start;
        v->selected_pages = v->test_pages;
        total_mem_mb = 128;
        return;
    }

    len = ofw_getprop(mem_node, "reg", reg, sizeof(reg));
    if (len < 0) len = 0;

    #define CLAIM_CHUNK 0x100000  /* 1MB chunks */

    seg = 0;
    total_mem_mb = 0;
    for (i = 0; i < len / (int)sizeof(unsigned long); i += 2) {
        unsigned long base = reg[i];
        unsigned long size = reg[i + 1];
        unsigned long addr, end_addr;

        if (size == 0) continue;
        total_mem_mb += size / (1024 * 1024);

        /* Skip the first 8MB to avoid our code, OF, and the framebuffer */
        if (base < 0x800000) {
            if (base + size <= 0x800000) continue;
            size -= (0x800000 - base);
            base = 0x800000;
        }

        /*
         * Claim memory from OF in 1MB chunks. Real Apple OF has the MMU on
         * and claim both allocates and maps. Claiming the whole range at once
         * fails if OF has anything claimed within it, so we go chunk by chunk
         * and coalesce adjacent successes into segments.
         */
        end_addr = base + size;
        for (addr = base; addr < end_addr; addr += CLAIM_CHUNK) {
            unsigned long chunk = end_addr - addr;
            if (chunk > CLAIM_CHUNK) chunk = CLAIM_CHUNK;

            if (ofw_claim((void *)addr, chunk, 0) == (void *)-1) {
                /* This chunk is in use — close out any open segment */
                continue;
            }

            /* Extend current segment or start a new one */
            if (seg > 0 && v->pmap[seg - 1].end == (addr >> 12)) {
                v->pmap[seg - 1].end = (addr + chunk) >> 12;
            } else if (seg < MAX_MEM_SEGMENTS) {
                v->pmap[seg].start = addr >> 12;
                v->pmap[seg].end = (addr + chunk) >> 12;
                seg++;
            }
        }
    }

    v->msegs = seg;
    v->test_pages = 0;
    for (i = 0; i < seg; i++) {
        v->test_pages += v->pmap[i].end - v->pmap[i].start;
    }
    v->selected_pages = v->test_pages;
}

/*
 * Set up the memory map for the current test window.
 * For now, single-window: test all discovered memory directly.
 */
static int setup_segments(void)
{
    int i, sg = 0;

    for (i = 0; i < v->msegs; i++) {
        ulong start_addr = v->pmap[i].start << 12;
        ulong end_addr = v->pmap[i].end << 12;

        v->map[sg].pbase_addr = v->pmap[i].start;
        v->map[sg].start = (ulong *)start_addr;
        v->map[sg].end = (ulong *)(end_addr - 4);
        sg++;
    }
    return sg;
}

/*
 * Draw the initial TUI screen — faithful to memtest86+ v5.01
 */
static void draw_screen(void)
{
    int i;

    /* Make the name background green (attribute 0x20 = green bg, black fg) */
    for (i = 0; i < TITLE_WIDTH; i++) {
        vga_buf[(0 * SCREEN_WIDTH + i) * 2 + 1] = 0x20;
    }
    cprint(0, 0, "   Memtestppc+ v0.01      ");

    /* Blinking "+" — attribute 0xA4 = blink + green bg + red fg */
    vga_buf[(0 * SCREEN_WIDTH + 13) * 2 + 1] = 0xA4;
    cprint(0, 13, "+");

    /* Reverse video bottom line */
    for (i = 0; i < SCREEN_WIDTH; i++) {
        vga_buf[(24 * SCREEN_WIDTH + i) * 2 + 1] = 0x71;
    }

    /* CPU and system info area */
    cprint(1, 0, "CLK:            ");
    cprint(2, 0, "L1 Cache: Unknown ");
    cprint(3, 0, "L2 Cache: Unknown ");
    cprint(4, 0, "Memory  :         ");
    cprint(5, 0, "Testing :         ");

    /* Vertical separator */
    for (i = 0; i < 6; i++) {
        cprint(i, COL_MID - 2, "| ");
    }

    /* Test progress area */
    cprint(1, COL_MID, "Pass   %");
    cprint(2, COL_MID, "Test   %");
    cprint(3, COL_MID, "Test #");
    cprint(4, COL_MID, "Testing: ");
    cprint(5, COL_MID, "Pattern: ");
    cprint(5, 60, "| Time:  0:00:00");

    /* Status line */
    cprint(6, 0,
        "------------------------------------------------------------------------------");
    cprint(7, 0, "State: / Running...");
    cprint(8, 0,
        "                                                                              ");
    cprint(9, 0,
        "Pass:       0                                               Errors:      0    ");
    cprint(10, 0,
        "------------------------------------------------------------------------------");

    /* Error header */
    cprint(11, 0, "                                                                              ");
    cprint(12, 0,
        " Address       Expected    Actual      Xor        Tst  Pass                  ");
    cprint(13, 0,
        "------------------------------------------------------------------------------");

    /* Fill in CPU info */
    if (cpu_clock_mhz > 0) {
        cprint(1, 4, "      MHz");
        dprint(1, 4, cpu_clock_mhz, 5, 0);
    }
    /* CPU name on title line */
    cprint(0, COL_MID, cpu_name ? cpu_name : "Unknown PowerPC");

    /* Cache info */
    if (l1_cache_kb > 0) {
        cprint(2, 0, "L1 Cache:     K  ");
        dprint(2, 11, l1_cache_kb, 3, 0);
    }
    if (l2_cache_kb > 0) {
        cprint(3, 0, "L2 Cache:     K  ");
        dprint(3, 10, l2_cache_kb, 4, 0);
    }

    /* Installed RAM (line 4) vs. amount we claimed to test (line 5) */
    aprint(4, 9, total_mem_mb << 8);   /* MB -> 4K pages */
    aprint(5, 9, v->test_pages);

    /* Footer */
    footer();

    /* Render everything */
    display_refresh();
}

static int find_ticks_for_test(int tst)
{
    int c, ch;

    if (tseq[tst].sel == 0) return 0;

    c = (v->pass == 0) ? tseq[tst].iter / 3 : tseq[tst].iter;
    if (c == 0) c = 1;

    /* Number of SPINSZ chunks per full memory traversal */
    ch = (v->selected_pages + (SPINSZ / 1024) - 1) / (SPINSZ / 1024);
    if (ch < 1) ch = 1;

    switch (tseq[tst].pat) {
    case 0:  return 2;
    case 1:
    case 2:  return 2 * ch;
    case 3:  return (2 + 4 * c) * ch;
    case 5:  return (24 + 24 * c) * ch;
    case 6:  return (c + 4 * c) * ch;
    case 7:  return c * ch;
    case 8:  return (1 + c * 2) * 64 * ch;
    case 9:  return 3 * c * ch;
    case 10: return 4 * 40 * c * ch;
    case 11: return 2 * ch;
    default: return 2;
    }
}

static void calc_pass_ticks(void)
{
    int i;
    pass_ticks = 0;
    for (i = 0; tseq[i].cpu_sel != 0; i++)
        pass_ticks += find_ticks_for_test(i);
}

/*
 * Select next test
 */
static void next_test(void)
{
    test++;
    while (tseq[test].sel == 0 && tseq[test].cpu_sel != 0)
        test++;

    if (tseq[test].cpu_sel == 0) {
        pass_flag++;
        test = 0;
        while (tseq[test].sel == 0 && tseq[test].cpu_sel != 0)
            test++;
    }
}

/*
 * Run one test
 */
static int do_test(void)
{
    int i, j;
    ulong p0, p1, p2;

    switch (tseq[test].pat) {
    case 0: /* Address test, walking ones */
        addr_tst1();
        BAILOUT;
        break;

    case 1:
    case 2: /* Address test, own address */
        addr_tst2();
        BAILOUT;
        break;

    case 3: /* Moving inversions, ones and zeros */
        p1 = 0;
        p2 = ~p1;
        movinv1(c_iter, p1, p2);
        BAILOUT;
        movinv1(c_iter, p2, p1);
        BAILOUT;
        break;

    case 5: /* Moving inversions, 8-bit walking */
        p0 = 0x80;
        for (i = 0; i < 8; i++, p0 >>= 1) {
            p1 = p0 | (p0 << 8) | (p0 << 16) | (p0 << 24);
            p2 = ~p1;
            movinv1(c_iter, p1, p2);
            BAILOUT;
            movinv1(c_iter, p2, p1);
            BAILOUT;
        }
        break;

    case 6: /* Random data */
        rand_seed((unsigned int)mftb(), (unsigned int)mftb() ^ 0x5a5a5a5a);
        for (i = 0; i < c_iter; i++) {
            p1 = rand_next();
            p2 = ~p1;
            movinv1(2, p1, p2);
            BAILOUT;
        }
        break;

    case 7: /* Block move */
        block_move(c_iter);
        BAILOUT;
        break;

    case 8: /* Moving inversions, 32-bit shifting */
        for (i = 0, p1 = 1; p1; p1 <<= 1, i++) {
            movinv32(c_iter, p1, 1, 0x80000000, 0, i);
            BAILOUT;
            movinv32(c_iter, ~p1, 0xfffffffe, 0x7fffffff, 1, i);
            BAILOUT;
        }
        break;

    case 9: /* Random number sequence */
        for (i = 0; i < c_iter; i++) {
            movinvr();
            BAILOUT;
        }
        break;

    case 10: /* Modulo 20 */
        for (j = 0; j < c_iter; j++) {
            p1 = rand_next();
            for (i = 0; i < MOD_SZ; i++) {
                p2 = ~p1;
                modtst(i, 2, p1, p2);
                BAILOUT;
                modtst(i, 2, p2, p1);
                BAILOUT;
            }
        }
        break;

    case 11: /* Bit fade */
        bit_fade_fill(0);
        BAILOUT;
        /* Sleep would go here - skip for now */
        bit_fade_chk(0);
        BAILOUT;
        bit_fade_fill(~0UL);
        BAILOUT;
        bit_fade_chk(~0UL);
        BAILOUT;
        break;
    }
    return 0;
}

/*
 * Main entry point
 */
int main(void)
{
    /* Initialize display */
    if (display_init() < 0) {
        ofw_puts("display_init failed, no framebuffer found\r\n");
        ofw_exit();
        for (;;) ;
    }

    /* Get timebase frequency */
    tb_freq = ofw_get_timebase_freq();

    /* Get keyboard */
    stdin_ih = ofw_get_stdin();

    /* Identify CPU */
    identify_cpu();

    /* Discover memory */
    discover_memory();

    /* Draw the TUI */
    draw_screen();

    /* Record start time */
    read_tb(&start_tb_h, &start_tb_l);

    /* Initialize test state */
    v->pass = 0;
    v->ecount = 0;
    v->testsel = -1;
    v_msg_line = LINE_SCROLL - 1;
    test = 0;
    bail = 0;
    pass_flag = 0;
    total_ticks = 0;
    calc_pass_ticks();

    /* Main test loop */
    while (1) {
        /* Set up the test */
        c_iter = tseq[test].iter / 3; /* reduced for first pass */
        if (v->pass > 0) c_iter = tseq[test].iter;

        test_ticks = find_ticks_for_test(test);
        nticks = 0;

        /* Display current test info */
        dprint(LINE_TST, COL_MID + 6, tseq[test].pat, 2, 1);
        cprint(LINE_TST, COL_MID + 9, tseq[test].msg);
        cprint(LINE_PAT, COL_PAT, "            ");

        /* Clear progress bar */
        cprint(2, COL_MID + 9, "                                         ");

        /* Set up memory segments for testing */
        segs = setup_segments();
        if (segs == 0) {
            cprint(LINE_MSG, COL_MSG, "** No memory segments to test! **");
            for (;;) check_key();
        }

        /* Display memory range being tested */
        {
            ulong p0 = (ulong)v->map[0].start >> 12;
            ulong p1 = (ulong)v->map[segs - 1].end >> 12;
            aprint(LINE_RANGE, COL_MID + 9, p0);
            cprint(LINE_RANGE, COL_MID + 14, " - ");
            aprint(LINE_RANGE, COL_MID + 17, p1);
        }

        /* Run the test */
        do_test();
        bail = 0;

        /* Advance to next test */
        next_test();

        /* If we completed a pass */
        if (pass_flag) {
            pass_flag = 0;
            v->pass++;
            dprint(9, 12, v->pass, 5, 0);

            /* Reset pass progress for next pass */
            total_ticks = 0;
            calc_pass_ticks();
            cprint(1, COL_MID + 9, "                                         ");

            if (v->ecount == 0) {
                cprint(LINE_MSG, COL_MSG - 8,
                    "** Pass complete, no errors, press Esc to exit **");
            }
        }
    }

    return 0;
}
