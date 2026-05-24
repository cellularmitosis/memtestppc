/* main.c - MemTest-86  Version 3.2
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady
 * ----------------------------------------------------
 * MemTest86+ V2.00 Specific code (GPL V2.0)
 * By Samuel DEMEULEMEESTER, sdemeule@memtest.org
 * http://www.canardplus.com - http://www.memtest.org
 * ----------------------------------------------------
 * memtestppc+ port: line-faithful port of memtest86+ v2.00 main.c to
 * PowerPC / Open Firmware (single-CPU). Discipline (see
 * docs/sessions/006-port-fidelity-restructure/PORTING-GUIDE.md): start from
 * the verbatim v2.00 file; comment out x86 leaf code with a one-line reason;
 * add PPC/OF replacements inline, marked "PPC:". Nothing from the old
 * attic/src-v0.01 hack is inherited.
 *
 * THE ONE STRUCTURAL CHANGE (Key invariant #2/#4): on x86, head.S jumps
 * straight into do_test() and the *test loop is the relocation itself* — at the
 * end of each window/test do_test() calls run_at(), which memmove()s the image
 * to a fresh address and re-enters via startup. That scheme exists to (a) loop
 * and (b) vacate the RAM about to be tested. On PPC/OF we test IN PLACE over
 * OF-claimed memory (no self-relocation), so:
 *   - the loop lives in main() as a plain for(;;) do_test();
 *   - do_test() returns where upstream would have run_at()'d;
 *   - the x86 PAE "windows[]" relocation/aliasing table collapses to a single
 *     flat window spanning all physical memory (compute_segments clamps it to
 *     [plim_lower, plim_upper] and intersects v->pmap[]);
 *   - map_page/mapping/emapping/page_of/paging_off become flat in-place ops
 *     (init.c #if 0'd its x86 PAE versions; we provide the flat ones here).
 */

#include "test.h"
#include "defs.h"   /* x86: body is #if 0'd in our port (real-mode boot/GDT/reloc
                     * segment defs); included for fidelity, contributes nothing. */
#undef TEST_TIMES
#define DEFTESTS 9

/* x86: bzero() came from the x86 string/lib glue; unused in main.c and not part
 * of our lib.c. Commented (no symbol referenced). */
/* extern void bzero(); */

const struct tseq tseq[] = {
	{0,  5,  3,   0, 0, "[Address test, walking ones, uncached]"},
	{1,  6,  3,   2, 0, "[Address test, own address]           "},
	{1,  0,  3,  14, 0, "[Moving inversions, ones & zeros]     "},
	{1,  1,  2,  80, 0, "[Moving inversions, 8 bit pattern]    "},
	{1, 10, 60, 300, 0, "[Moving inversions, random pattern]   "},
	{1,  7, 64,  66, 0, "[Block move, 64 moves]                "},
	{1,  2,  2, 320, 0, "[Moving inversions, 32 bit pattern]   "},
	{1,  9, 40,	120, 0, "[Random number sequence]              "},
	{1,  3,  4, 1920, 0,"[Modulo 20, random pattern]           "},
	{1,  8,  1,		2, 0, "[Bit fade test, 90 min, 2 patterns]   "},
	{0,  0,  0,   0, 0, NULL}
};

char firsttime = 0;
char cmdline_parsed = 0;

struct vars variables = {};
struct vars * const v = &variables;

volatile ulong *p = 0;
ulong p1 = 0, p2 = 0, p0 = 0;
int segs = 0, bail = 0;
int test_ticks;
int nticks;
/* x86: high_test_adr is the secondary relocation target (16 MB if enough RAM).
 * Only the (commented-out) self-relocation paths read it — kept for fidelity. */
ulong high_test_adr = 0x200000;

static int window = 0;
/* PPC: a single flat window spanning all physical memory. compute_segments()
 * clamps [window.start, window.end] to [v->plim_lower, v->plim_upper] and
 * intersects it with v->pmap[], so this one entry covers every claimed segment;
 * the test routines chunk it internally by SPINSZ. window++ wraps to 0 after
 * each test (sizeof(windows)/sizeof(windows[0]) == 1), so do_test() runs each
 * test exactly once over all memory, then advances v->test. */
static struct pmap windows[] =
{
	{ 0, 0xffffffff },
};

/* x86: the original 32-entry PAE window table — 2 MB-granular virtual windows
 * spanning 0..64 GB that the test relocated itself between (map_page programmed
 * %cr3 to alias each into the low 2 MB). N/A on PPC/OF flat in-place testing;
 * preserved (never deleted) for upstream fidelity. */
#if 0
static struct pmap windows[] =
{
	{ 0, 0x080000 },
	{ 0, 0 },

	{ 0x080000, 0x100000 },
	{ 0x100000, 0x180000 },
	{ 0x180000, 0x200000 },

	{ 0x200000, 0x280000 },
	{ 0x280000, 0x300000 },

	{ 0x300000, 0x380000 },
	{ 0x380000, 0x400000 },

	{ 0x400000, 0x480000 },
	{ 0x480000, 0x500000 },

	{ 0x500000, 0x580000 },
	{ 0x580000, 0x600000 },

	{ 0x600000, 0x680000 },
	{ 0x680000, 0x700000 },

	{ 0x700000, 0x780000 },
	{ 0x780000, 0x800000 },

	{ 0x800000, 0x880000 },
	{ 0x880000, 0x900000 },

	{ 0x900000, 0x980000 },
	{ 0x980000, 0xA00000 },

	{ 0xA00000, 0xA80000 },
	{ 0xA80000, 0xB00000 },

	{ 0xB00000, 0xB80000 },
	{ 0xB80000, 0xC00000 },

	{ 0xC00000, 0xC80000 },
	{ 0xC80000, 0xD00000 },

	{ 0xD00000, 0xD80000 },
	{ 0xD80000, 0xE00000 },

	{ 0xE00000, 0xE80000 },
	{ 0xE80000, 0xF00000 },

	{ 0xF00000, 0xF80000 },
	{ 0xF80000, 0x1000000 },
};
#endif


/* x86: LOW_TEST_ADR is the relocation target (must be in the low 640K real-mode
 * area). N/A on PPC (linker.ld owns the load address, OF maps it). Commented. */
/*
#if (LOW_TEST_ADR > (640*1024))
#error LOW_TEST_ADR must be below 640K
#endif
*/

static int find_ticks_for_test(unsigned long chunks, int test);
static void compute_segments(int win);

/* PPC: flat in-place address mapping. init.c carries the upstream x86 PAE
 * versions of these (map_page/mapping/emapping/page_of/paging_off) but wraps
 * them in #if 0 — under OF the MMU is already on and our claimed RAM is mapped
 * 1:1, so there is no paging window to program. These flat versions are the
 * FLAT==1 branch of the upstream code: a page number maps to its byte address
 * and back, and map_page/paging_off are no-ops. They satisfy the prototypes in
 * test.h and the call sites in compute_segments()/do_test()/error.c. */
int map_page(unsigned long page)
{
	(void)page;
	return 0;            /* flat: every claimed page is already mapped */
}

void *mapping(unsigned long page_addr)
{
	/* FLAT branch of upstream mapping(): page number -> byte address. */
	return (void *)(page_addr << 12);
}

void *emapping(unsigned long page_addr)
{
	/* Verbatim upstream emapping() over the flat mapping(): the end pointer
	 * is 0xf00 into the last page of the segment (page_addr is exclusive). */
	void *result;
	result = mapping(page_addr - 1);
	result = ((unsigned char *)result) + 0xf00;
	return result;
}

unsigned long page_of(void *addr)
{
	/* FLAT branch of upstream page_of(): byte address -> page number. */
	return ((unsigned long)addr) >> 12;
}

void paging_off(void)
{
	/* flat: nothing to disable (no PAE window active) */
}

/* x86: adj_mem() (config.c, Wave 6) re-derives v->selected_pages / plim_* from
 * the address-range selection. mem_size() (memsize.c) calls it, and it is also
 * reachable from the config menu. Until config.c lands it is a no-op: memsize.c
 * already set selected_pages = test_pages (the whole map), which is exactly what
 * adj_mem() computes when no range restriction is in effect. */
void adj_mem(void)
{
	/* Wave-6 stub — config.c will provide the real address-range logic. */
}

/* x86: insertaddress() records a failing address into the BadRAM pattern table
 * (patn.c) for the PRINTMODE_PATTERNS "badram=" dump; error.c's common_err()
 * calls it. patn.c is a Wave-6 file, and the default print mode is
 * PRINTMODE_ADDRESSES (set in init.c), so this path is not taken in a normal
 * run. Stub returning 0 (no pattern recorded) keeps the link resolved until
 * patn.c lands. */
int insertaddress(ulong addr)
{
	(void)addr;
	return 0;
}

/* x86: self-relocation + boot-protocol command line.
 *  - __run_at()/run_at() memmove() the whole image to a new address and jump to
 *    the relocated startup_32 (a GCC computed goto). This both forms the test
 *    loop AND vacates the RAM under test. PPC tests in place (Key invariant #4),
 *    so there is no relocation; the loop is main()'s for(;;).
 *  - parse_command_line() reads "console=..." from the old Linux boot protocol
 *    via real-mode INITSEG segment pointers (defs.h) and hands it to
 *    serial_console_setup() — N/A on PPC/OF (OF loads the ELF; no real-mode boot
 *    block, and the serial console leaf is commented in lib.c).
 * Preserved verbatim (never deleted) inside #if 0 for upstream fidelity. */
#if 0
static void __run_at(unsigned long addr)
{
	/* Copy memtest86+ code */
	memmove((void *)addr, &_start, _end - _start);
	/* Jump to the start address */
	p = (ulong *)(addr + startup_32 - _start);
	goto *p;
}

static unsigned long run_at_addr = 0xffffffff;
static void run_at(unsigned long addr)
{
	unsigned long start;
	unsigned long len;

	run_at_addr = addr;

	start = (unsigned long) &_start;
	len = _end - _start;
	if (	((start < addr) && ((start + len) >= addr)) ||
		((addr < start) &&  ((addr + len) >= start))) {
		/* Handle overlap by doing an extra relocation */
		if (addr + len < high_test_adr) {
			__run_at(high_test_adr);
		}
		else if (start + len < addr) {
			__run_at(LOW_TEST_ADR);
		}
	}
	__run_at(run_at_addr);
}

/* command line passing using the 'old' boot protocol */
#define MK_PTR(seg,off) ((void*)(((unsigned long)(seg) << 4) + (off)))
#define OLD_CL_MAGIC_ADDR ((unsigned short*) MK_PTR(INITSEG,0x20))
#define OLD_CL_MAGIC 0xA33F
#define OLD_CL_OFFSET_ADDR ((unsigned short*) MK_PTR(INITSEG,0x22))

static void parse_command_line(void)
{
	char *cmdline;

	if (cmdline_parsed)
		return;

	if (*OLD_CL_MAGIC_ADDR != OLD_CL_MAGIC)
		return;

	unsigned short offset = *OLD_CL_OFFSET_ADDR;
	cmdline = MK_PTR(INITSEG, offset);

	/* skip leading spaces */
	while (*cmdline == ' ')
		cmdline++;

	while (*cmdline) {
		if (!strncmp(cmdline, "console=", 8)) {
			cmdline += 8;
			serial_console_setup(cmdline);
		}

		/* go to the next parameter */
		while (*cmdline && *cmdline != ' ')
			cmdline++;
		while (*cmdline == ' ')
			cmdline++;
	}

	cmdline_parsed = 1;
}
#endif /* x86 self-relocation + boot-protocol cmdline — N/A on PPC/OF */


void do_test(void)
{
	int i = 0, j = 0;
	unsigned long chunks;
	unsigned long lo, hi;

	/* x86: parse_command_line() — boot-protocol cmdline, #if 0'd above (N/A). */
	/* parse_command_line(); */

	/* x86: finish a partial self-relocation. We never relocate (in-place), so
	 * there is nothing to finish. */
	/*
	if (run_at_addr == (unsigned long)&_start) {
		run_at_addr = 0xffffffff;
	} else if (run_at_addr != 0xffffffff) {
		__run_at(run_at_addr);
	}
	*/

	/* If first time, initialize test */
	if (firsttime == 0) {
		/* x86: if the image is not at LOW_TEST_ADR, relocate (restart()) before
		 * initializing — N/A in place. */
		/*
		if ((ulong)&_start != LOW_TEST_ADR) {
			restart();
		}
		*/
		init();
		/* x86: seed the relocation window bounds (windows[0].start past the end
		 * of the relocated image; windows[1].end / high_test_adr at 16 MB if RAM
		 * allows). N/A — our single flat window is statically full-range. */
		/*
		windows[0].start =
			( LOW_TEST_ADR + (_end - _start) + 4095) >> 12;
		if (v->pmap[v->msegs-1].end > 0x1100) {
			high_test_adr = 0x01000000;
		}
		windows[1].end = (high_test_adr >> 12);
		*/
		firsttime = 1;
	}
	bail = 0;

	/* Find the memory areas I am going to test */
	compute_segments(window);
	if (segs == 0) {
		goto skip_window;
	}
	/* x86: map_page() programmed the PAE window for v->map[0]; flat no-op on PPC
	 * (always returns 0, never < 0). Kept at the call site for fidelity. */
	if (map_page(v->map[0].pbase_addr) < 0) {
		goto skip_window;
	}

	/* x86: if relocated above LOW_TEST_ADR, the lower memory we vacated still
	 * needs testing and the range is annotated " Relocated". We never relocate,
	 * so always take the "not relocated" path: clear that annotation. */
	/*
	if ((ulong)&_start > LOW_TEST_ADR) {
		v->map[0].start = mapping(v->plim_lower);
		cprint(LINE_RANGE, COL_MID+28, " Relocated");
	} else {
		cprint(LINE_RANGE, COL_MID+28, "          ");
	}
	*/
	cprint(LINE_RANGE, COL_MID+28, "          ");

	/* Update display of memory segments being tested */
	lo = page_of(v->map[0].start);
	hi = page_of(v->map[segs -1].end);
	aprint(LINE_RANGE, COL_MID+9, lo);
	cprint(LINE_RANGE, COL_MID+14, " - ");
	aprint(LINE_RANGE, COL_MID+17, hi);
	aprint(LINE_RANGE, COL_MID+23, v->selected_pages);


#ifdef TEST_TIMES
	{
		ulong l, h, t;

		asm __volatile__ (
			"rdtsc\n\t"
			"subl %%ebx,%%eax\n\t"
			"sbbl %%ecx,%%edx\n\t"
			:"=a" (l), "=d" (h)
			:"b" (v->snapl), "c" (v->snaph)
		);

		cprint(20, 5, ":  :");
		t = h * ((unsigned)0xffffffff / v->clks_msec) / 1000;
		t += (l / v->clks_msec) / 1000;
		i = t % 60;
		dprint(20, 10, i%10, 1, 0);
		dprint(20, 9, i/10, 1, 0);
		t /= 60;
		i = t % 60;
		dprint(20, +7, i % 10, 1, 0);
		dprint(20, +6, i / 10, 1, 0);
		t /= 60;
		dprint(20, 0, t, 5, 0);

		asm __volatile__ ("rdtsc":"=a" (v->snapl),"=d" (v->snaph));
	}
#endif
	/* Now setup the test parameters based on the current test number */
	/* Figure out the next test to run */
	if (v->testsel >= 0) {
		v->test = v->testsel;
	}
	dprint(LINE_TST, COL_MID+6, v->test, 2, 1);
	cprint(LINE_TST, COL_MID+9, tseq[v->test].msg);
	set_cache(tseq[v->test].cache);

	/* Compute the number of SPINSZ memory segments */
	chunks = 0;
	for(i = 0; i < segs; i++) {
		unsigned long len;
		len = v->map[i].end - v->map[i].start;
		chunks += (len + SPINSZ -1)/SPINSZ;
	}
	test_ticks = find_ticks_for_test(chunks, v->test);
	nticks = 0;
	v->tptr = 0;
	cprint(1, COL_MID+8, "                                         ");
	switch(tseq[v->test].pat) {

	/* Now do the testing according to the selected pattern */
	case 0:	/* Moving inversions, all ones and zeros */
	case 4:

		if (tseq[v->test].pat == 1)
			p0 = 0x80808080;
		else
			p0 = 0;

		for ( ; ; ) {
			movinv1(tseq[v->test].iter,p0,~p0);
			BAILOUT;

			/* Switch patterns */
			movinv1(tseq[v->test].iter,~p0,p0);
			BAILOUT
			if ( !((unsigned char)(p0 >>= 1) & 0x7F) )
				break;
		}
		break;

	case 1: /* Moving inversions, 8 bit wide walking ones and zeros. */
		p0 = 0x80;
		for (i=0; i<8; i++, p0=p0>>1) {
			p1 = p0 | (p0<<8) | (p0<<16) | (p0<<24);
			p2 = ~p1;
			movinv1(tseq[v->test].iter,p1,p2);
			BAILOUT;

			/* Switch patterns */
			p2 = p1;
			p1 = ~p2;
			movinv1(tseq[v->test].iter,p1,p2);
			BAILOUT
		}
		break;


	case 2: /* Moving inversions, 32 bit shifting pattern, very long */
		for (i=0, p1=1; p1; p1=p1<<1, i++) {
			movinv32(tseq[v->test].iter,p1, 1, 0x80000000, 0, i);
			BAILOUT
			movinv32(tseq[v->test].iter,~p1, 0xfffffffe, 0x7fffffff, 1, i);
			BAILOUT
		}
		break;

	case 3: /* Modulo 20, random */
		for (j=0; j<tseq[v->test].iter; j++) {
			p1 = rand();
			for (i=0; i<MOD_SZ; i++) {
				p2 = ~p1;
				modtst(i, tseq[v->test].iter, p1, p2);
				BAILOUT

				/* Switch patterns */
				p2 = p1;
				p1 = ~p2;
				modtst(i, tseq[v->test].iter, p1, p2);
				BAILOUT
			}
		}
		break;

	case 5: /* Address test, walking ones */
		addr_tst1();
		BAILOUT;
		break;

	case 6: /* Address test, own address */
		addr_tst2();
		BAILOUT;
		break;

	case 7: /* Block move test */
		block_move(tseq[v->test].iter);
		BAILOUT;
		break;

	case 8: /* Bit fade test */
		if (window == 0 ) {
			bit_fade();
		}
		BAILOUT;
		break;

	case 9: /* Random Data Sequence */
		for (i=0; i < tseq[v->test].iter; i++) {
			movinvr();
			BAILOUT;
		}
		break;
	case 10: /* Random Data */
		for (i=0; i < tseq[v->test].iter; i++) {
			p1 = rand();
			p2 = ~p1;
			movinv1(2,p1,p2);
			BAILOUT;
		}
		break;
	}
 skip_window:
	if (bail) {
		goto bail_test;
	}
	/* Rever to the default mapping and enable the cache */
	paging_off();
	set_cache(1);
	window++;
	if (window >= sizeof(windows)/sizeof(windows[0])) {
		window = 0;
	}
	/* We finished the test so clear the pattern */
	cprint(LINE_PAT, COL_PAT, "            ");
	if (window != 0) {
		/* x86: with multiple PAE windows, advance to the next one by relocating
		 * the image out of the window about to be tested (run_at high/low copy).
		 * PPC has a single flat window, so window always wraps to 0 here and this
		 * branch is never taken. Preserved (commented) for upstream fidelity. */
		/*
		if (windows[window].start <
			((ulong)&_start + (_end - _start)) >> 12) {
			if (v->pmap[v->msegs-1].end >
				(((high_test_adr + (_end - _start)) >> 12)+1)) {
				run_at(high_test_adr);
			} else {
				goto skip_window;
			}
		} else {
			run_at(LOW_TEST_ADR);
		}
		*/
	}
	else {
		/* We have run this test in all of the windows
		 * advance to the next test.
		 */
	skip_test:
		v->test++;
	bail_test:
		/* Revert to the default mapping
		 * and enable the cache.
		 */
		paging_off();
		set_cache(1);
		check_input();
		window = 0;
		cprint(LINE_PAT, COL_PAT-3, "   ");
		/* If this was the last test then we finished a pass */
		if (v->test >= 9 || v->testsel >= 0) {
			v->pass++;
			dprint(LINE_INFO, COL_PASS, v->pass, 5, 0);
			v->test = 0;
			v->total_ticks = 0;
			v->pptr = 0;
			cprint(0, COL_MID+8,
				"                                         ");
			if (v->ecount == 0 && v->testsel < 0) {
			    cprint(LINE_MSG, COL_MSG,
				"Pass complete, no errors, press Esc to exit");
			}
		}

	}
	/* x86: always start the next window/test/pass via a relocation to the low
	 * copy (run_at re-enters do_test through startup). On PPC the loop is in
	 * main(): just return and main() calls do_test() again, with window/v->test/
	 * v->pass carried in the globals above. */
	/* run_at(LOW_TEST_ADR); */
	return;
}

void restart()
{
	int i;
	volatile char *pp;

	/* clear variables */
	firsttime = 0;
	v->test = 0;
	v->pass = 0;
	v->msg_line = 0;
	v->ecount = 0;
	v->ecc_ecount = 0;

	/* Clear the screen */
	for(i=0, pp=(char *)(SCREEN_ADR+0); i<80*24; i++, pp+=2) {
		*pp = ' ';
	}
	/* x86: run_at(LOW_TEST_ADR) relocated the image and re-entered. On PPC the
	 * main() loop continues from a cleared state — just return. (restart() is
	 * not called yet; the config-menu "Restart" lands in Wave 6.) */
	/* run_at(LOW_TEST_ADR); */
}


/* Compute the total number of ticks per pass */
void find_ticks(void)
{
	int i, j, chunks;

	v->pptr = 0;

	/* Compute the number of SPINSZ memory segments in one pass */
	chunks = 0;
	for(j = 0; j < sizeof(windows)/sizeof(windows[0]); j++) {
		compute_segments(j);
		for(i = 0; i < segs; i++) {
			unsigned long len;
			len = v->map[i].end - v->map[i].start;
			chunks += (len + SPINSZ -1)/SPINSZ;
		}
	}
	compute_segments(window);
	window = 0;
	for (v->pass_ticks=0, i=0; ((i<DEFTESTS) && (DEFTESTS != NULL)); i++) {

		/* Test to see if this test is selected for execution */
		if (v->testsel >= 0) {
			if (i != v->testsel) {
				continue;
			}
		}
		v->pass_ticks += find_ticks_for_test(chunks, i);
	}
}

static int find_ticks_for_test(unsigned long chunks, int test)
{
	int ticks;
	ticks = chunks * tseq[test].ticks;
	if (tseq[test].pat == 5) {
		/* Address test, walking ones */
		ticks = 4;
	}
	return ticks;
}

static void compute_segments(int win)
{
	unsigned long wstart, wend;
	int i;

	/* Compute the window I am testing memory in */
	wstart = windows[win].start;
	wend = windows[win].end;
	segs = 0;

	/* Now reduce my window to the area of memory I want to test */
	if (wstart < v->plim_lower) {
		wstart = v->plim_lower;
	}
	if (wend > v->plim_upper) {
		wend = v->plim_upper;
	}
	if (wstart >= wend) {
		return;
	}
	/* List the segments being tested */
	for (i=0; i< v->msegs; i++) {
		unsigned long start, end;
		start = v->pmap[i].start;
		end = v->pmap[i].end;
		if (start <= wstart) {
			start = wstart;
		}
		if (end >= wend) {
			end = wend;
		}
#if 0
		cprint(LINE_SCROLL+(2*i), 0, " (");
		hprint(LINE_SCROLL+(2*i), 2, start);
		cprint(LINE_SCROLL+(2*i), 10, ", ");
		hprint(LINE_SCROLL+(2*i), 12, end);
		cprint(LINE_SCROLL+(2*i), 20, ") ");

		cprint(LINE_SCROLL+(2*i), 22, "r(");
		hprint(LINE_SCROLL+(2*i), 24, wstart);
		cprint(LINE_SCROLL+(2*i), 32, ", ");
		hprint(LINE_SCROLL+(2*i), 34, wend);
		cprint(LINE_SCROLL+(2*i), 42, ") ");

		cprint(LINE_SCROLL+(2*i), 44, "p(");
		hprint(LINE_SCROLL+(2*i), 46, v->plim_lower);
		cprint(LINE_SCROLL+(2*i), 54, ", ");
		hprint(LINE_SCROLL+(2*i), 56, v->plim_upper);
		cprint(LINE_SCROLL+(2*i), 64, ") ");

		cprint(LINE_SCROLL+(2*i+1),  0, "w(");
		hprint(LINE_SCROLL+(2*i+1),  2, windows[win].start);
		cprint(LINE_SCROLL+(2*i+1), 10, ", ");
		hprint(LINE_SCROLL+(2*i+1), 12, windows[win].end);
		cprint(LINE_SCROLL+(2*i+1), 20, ") ");

		cprint(LINE_SCROLL+(2*i+1), 22, "m(");
		hprint(LINE_SCROLL+(2*i+1), 24, v->pmap[i].start);
		cprint(LINE_SCROLL+(2*i+1), 32, ", ");
		hprint(LINE_SCROLL+(2*i+1), 34, v->pmap[i].end);
		cprint(LINE_SCROLL+(2*i+1), 42, ") ");

		cprint(LINE_SCROLL+(2*i+1), 44, "i=");
		hprint(LINE_SCROLL+(2*i+1), 46, i);

		cprint(LINE_SCROLL+(2*i+2), 0,
			"                                        "
			"                                        ");
		cprint(LINE_SCROLL+(2*i+3), 0,
			"                                        "
			"                                        ");
#endif
		if ((start < end) && (start < wend) && (end > wstart)) {
			v->map[segs].pbase_addr = start;
			v->map[segs].start = mapping(start);
			v->map[segs].end = emapping(end);
#if 0
			cprint(LINE_SCROLL+(2*i+1), 54, " segs: ");
			hprint(LINE_SCROLL+(2*i+1), 61, segs);
#endif
			segs++;
		}
	}
}

/* PPC: the test loop.
 *
 * On x86 head.S jumped straight to do_test() and the loop was the relocation
 * (run_at re-entered do_test via startup after each window/test). On PPC head.S
 * calls main() (see head.S), and we test in place, so main() is the loop: each
 * do_test() runs one test over all memory, advances v->test (and v->pass on a
 * completed pass), and returns. The per-test/per-pass state lives in the file
 * globals (window, v->test, v->pass, firsttime). init() is called once from
 * do_test()'s firsttime block (it brings up the framebuffer, identifies the CPU,
 * sizes/claims memory, draws the TUI, and seeds find_ticks()). */
int main(void)
{
	for (;;) {
		do_test();
	}
	return 0;
}
