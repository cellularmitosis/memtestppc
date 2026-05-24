/*
 * memtestppc+ - Memory tester for PowerPC Macs
 * Core definitions, adapted from memtest86+ test.h
 */

#ifndef MEMTEST_H
#define MEMTEST_H

typedef unsigned long ulong;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

/* Memory segment map */
#define MAX_MEM_SEGMENTS 64
#define SPINSZ    0x400000   /* 16 MB chunks */
#define MOD_SZ    20

#define BAILOUT   if (bail) return 1;
#define BAILR     if (bail) return;

/* Screen layout - matching memtest86+ v5.01 */
#define TITLE_WIDTH    28
#define LINE_TITLE     0
#define LINE_TST       3
#define LINE_RANGE     4
#define LINE_PAT       5
#define LINE_TIME      5
#define LINE_STATUS    8
#define LINE_INFO      9
#define LINE_HEADER    12
#define LINE_SCROLL    14
#define LINE_MSG       22
#define LINE_CPU       7
#define LINE_RAM       8
#define COL_MID        30
#define COL_PAT        41
#define COL_MSG        23
#define COL_TIME       67
#define COL_SPEC       41
#define COL_MODE       15
#define BAR_SIZE       (78 - COL_MID - 9)

/* Memory map structures */
struct mmap {
    ulong pbase_addr;
    ulong *start;
    ulong *end;
};

struct pmap {
    ulong start;
    ulong end;
};

/* Test sequence entry */
struct tseq {
    short sel;
    short cpu_sel;
    short pat;
    short iter;
    short errors;
    char *msg;
};

/* Error tracking */
struct xadr {
    ulong page;
    ulong offset;
};

struct err_info {
    struct xadr low_addr;
    struct xadr high_addr;
    unsigned long ebits;
    long tbits;
    short min_bits;
    short max_bits;
    unsigned long maxl;
    unsigned long eadr;
    unsigned long exor;
    unsigned long cor_err;
    short hdr_flag;
};

/* Global variables structure */
struct vars {
    int pass;
    int msg_line;
    int ecount;
    int msegs;
    int testsel;
    int scroll_start;
    int pass_ticks;
    int total_ticks;
    int pptr;
    int tptr;
    struct err_info erri;
    struct pmap pmap[MAX_MEM_SEGMENTS];
    volatile struct mmap map[MAX_MEM_SEGMENTS];
    ulong plim_lower;
    ulong plim_upper;
    ulong clks_msec;
    ulong starth;
    ulong startl;
    int printmode;
    int numpatn;
    ulong test_pages;
    ulong selected_pages;
};

#define PRINTMODE_ADDRESSES 1

extern struct vars *const v;
extern volatile int bail;
extern volatile int segs;
extern volatile int test;
extern int test_ticks, nticks;
extern struct tseq tseq[];
extern int v_msg_line;

/* Provided by display.c */
void cprint(int y, int x, const char *text);
void cplace(int y, int x, const char c);
void hprint(int y, int x, unsigned long val);
void hprint2(int y, int x, unsigned long val, int digits);
void dprint(int y, int x, unsigned long val, int len, int right);
void aprint(int y, int x, unsigned long page);
void xprint(int y, int x, unsigned long val);
void scroll(void);
void clear_scroll(void);
void footer(void);

/* Provided by test.c */
void rand_seed(unsigned int seed1, unsigned int seed2);
ulong rand_next(void);
void addr_tst1(void);
void addr_tst2(void);
void movinv1(int iter, ulong p1, ulong p2);
void movinvr(void);
void movinv32(int iter, ulong p1, ulong lb, ulong hb, int sval, int off);
void modtst(int off, int iter, ulong p1, ulong p2);
void block_move(int iter);
void bit_fade_fill(ulong p1);
void bit_fade_chk(ulong p1);

/* Provided by main.c */
void do_tick(void);
void error(ulong *adr, ulong good, ulong bad);
void ad_err1(ulong *adr1, ulong *adr2, ulong good, ulong bad);
void ad_err2(ulong *adr, ulong bad);

/* Utility */
int strlen(const char *s);
void *memset(void *s, int c, unsigned long n);
void *memcpy(void *dest, const void *src, unsigned long n);

/* PowerPC-specific */
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

#endif /* MEMTEST_H */
