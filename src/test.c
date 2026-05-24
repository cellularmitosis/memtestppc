
/* test.c - MemTest-86  Version 3.2
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady, cbrady@sgi.com
 * ----------------------------------------------------
 * MemTest86+ V2.00 Specific code (GPL V2.0)
 * By Samuel DEMEULEMEESTER, sdemeule@memtest.org
 * http://www.x86-secret.com - http://www.memtest.org
 * ----------------------------------------------------
 * memtestppc+ port: line-faithful port of memtest86+ v2.00 test.c to
 * PowerPC / Open Firmware (single-CPU). Discipline (see
 * docs/sessions/006-port-fidelity-restructure/PORTING-GUIDE.md):
 *   - start from the verbatim v2.00 file;
 *   - each test routine has a hand-tuned x86 `asm __volatile__` inner loop
 *     with the equivalent plain-C loop usually left commented right above it.
 *     For EACH routine we COMMENT OUT the x86 asm block (one-line reason) and
 *     ENABLE the plain-C equivalent (the upstream-commented C where present,
 *     otherwise faithful C written to match the asm), marked "PPC:";
 *   - the outer structure (the `for (j=0;j<segs;j++)` segment loops, the
 *     `do { ... pe += SPINSZ; ... } while(!done)` SPINSZ chunking with its
 *     overflow guards, forward/reverse passes) is preserved VERBATIM;
 *   - never delete upstream code; keep it buildable.
 * v2.00 routines are already cpu-free (no trailing `int cpu` arg), so there is
 * no signature stripping and no calculate_chunk()/get_cpu_range() splitting to
 * collapse — the whole segment (start=v->map[j].start, end=v->map[j].end) is
 * already the unit of work. Nothing from the old attic/src-v0.01 hack is
 * inherited.
 */

#include "test.h"
#include "config.h"
#include "ofw.h"   /* PPC: sleep() (bit-fade dwell) uses ofw_get_timebase_freq() */
/* x86: <sys/io.h> declares the x86 port-I/O primitives (inb/outb) used only by
 * beep() below — N/A on PPC/OF (no PC speaker, no port I/O). beep() is commented
 * out wholesale at the bottom of this file, so its header is too. */
/* #include <sys/io.h> */
/* x86: dmi.h is the SMBIOS/DMI interface — N/A on PPC/OF and unused by any code
 * in this file. */
/* #include "dmi.h" */

extern int segs, bail;
extern volatile ulong *p;
extern ulong p1, p2;
extern int test_ticks, nticks;
extern struct tseq tseq[];
extern void update_err_counts(void);
extern void print_err_counts(void);
void poll_errors();

int ecount = 0;

static inline ulong roundup(ulong value, ulong mask)
{
	return (value + mask) & ~mask;
}
/*
 * Memory address test, walking ones
 */
void addr_tst1()
{
	int i, j, k;
	volatile ulong *pt;
	volatile ulong *end;
	ulong bad, mask, bank;

	/* Test the global address bits */
	for (p1=0, j=0; j<2; j++) {
		hprint(LINE_PAT, COL_PAT, p1);

		/* Set pattern in our lowest multiple of 0x20000 */
		p = (ulong *)roundup((ulong)v->map[0].start, 0x1ffff);
		*p = p1;

		/* Now write pattern compliment */
		p1 = ~p1;
		end = v->map[segs-1].end;
		for (i=0; i<1000; i++) {
			mask = 4;
			do {
				pt = (ulong *)((ulong)p | mask);
				if (pt == p) {
					mask = mask << 1;
					continue;
				}
				if (pt >= end) {
					break;
				}
				*pt = p1;
				if ((bad = *p) != ~p1) {
					ad_err1((ulong *)p, (ulong *)mask,
					        bad, ~p1);
					i = 1000;
				}
				mask = mask << 1;
			} while(mask);
		}
		do_tick();
		BAILR
	}

	/* Now check the address bits in each bank */
	/* If we have more than 8mb of memory then the bank size must be */
	/* bigger than 256k.  If so use 1mb for the bank size. */
	if (v->pmap[v->msegs - 1].end > (0x800000 >> 12)) {
		bank = 0x100000;
	} else {
		bank = 0x40000;
	}
	for (p1=0, k=0; k<2; k++) {
		hprint(LINE_PAT, COL_PAT, p1);

		for (j=0; j<segs; j++) {
			p = v->map[j].start;
			/* Force start address to be a multiple of 256k */
			p = (ulong *)roundup((ulong)p, bank - 1);
			end = v->map[j].end;
			while (p < end) {
				*p = p1;

				p1 = ~p1;
				for (i=0; i<200; i++) {
					mask = 4;
					do {
						pt = (ulong *)
						    ((ulong)p | mask);
						if (pt == p) {
							mask = mask << 1;
							continue;
						}
						if (pt >= end) {
							break;
						}
						*pt = p1;
						if ((bad = *p) != ~p1) {
							ad_err1((ulong *)p,
							        (ulong *)mask,
							        bad,~p1);
							i = 200;
						}
						mask = mask << 1;
					} while(mask);
				}
				if (p + bank/4 > p) {
					p += bank/4;
				} else {
					p = end;
				}
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
void addr_tst2()
{
	int j, done;
	volatile ulong *pe;
	volatile ulong *end, *start;

	cprint(LINE_PAT, COL_PAT, "        ");

	/* Write each address with it's own address */
	for (j=0; j<segs; j++) {
		start = v->map[j].start;
		end = v->map[j].end;
		pe = (ulong *)start;
		p = start;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ > pe) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}

/* Original C code replaced with hand tuned assembly code */
			/* PPC: enable the upstream plain-C fill loop; the x86
			 * asm below is just this loop hand-scheduled. */
			for (; p < pe; p++) {
				*p = (ulong)p;
			}
/* x86 asm: store-own-address fill loop (movl %edi,(%edi); addl $4) —
 * replaced by the C loop above.
 *			asm __volatile__ (
 *				"jmp L90\n\t"
 *
 *				".p2align 4,,7\n\t"
 *				"L90:\n\t"
 *				"movl %%edi,(%%edi)\n\t"
 *				"addl $4,%%edi\n\t"
 *				"cmpl %%edx,%%edi\n\t"
 *				"jb L90\n\t"
 *				: "=D" (p)
 *				: "D" (p), "d" (pe)
 *			);
 */
			do_tick();
			BAILR
		} while (!done);
	}

	/* Each address should have its own address */
	for (j=0; j<segs; j++) {
		start = v->map[j].start;
		end = v->map[j].end;
		pe = (ulong *)start;
		p = start;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ > pe) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
/* Original C code replaced with hand tuned assembly code */
			/* PPC: enable the upstream plain-C check loop. */
			{
				ulong bad;
				for (; p < pe; p++) {
					if((bad = *p) != (ulong)p) {
						ad_err2((ulong)p, bad);
					}
				}
			}
/* x86 asm: load-and-compare-own-address loop (movl (%edi),%ecx; cmpl %edi,%ecx;
 * jne -> push args; call ad_err2) — replaced by the C loop above.
 *			asm __volatile__ (
 *				"jmp L91\n\t"
 *
 *				".p2align 4,,7\n\t"
 *				"L91:\n\t"
 *				"movl (%%edi),%%ecx\n\t"
 *				"cmpl %%edi,%%ecx\n\t"
 *				"jne L93\n\t"
 *				"L92:\n\t"
 *				"addl $4,%%edi\n\t"
 *				"cmpl %%edx,%%edi\n\t"
 *				"jb L91\n\t"
 *				"jmp L94\n\t"
 *
 *				"L93:\n\t"
 *				"pushl %%edx\n\t"
 *				"pushl %%ecx\n\t"
 *				"pushl %%edi\n\t"
 *				"call ad_err2\n\t"
 *				"popl %%edi\n\t"
 *				"popl %%ecx\n\t"
 *				"popl %%edx\n\t"
 *				"jmp L92\n\t"
 *
 *				"L94:\n\t"
 *				: "=D" (p)
 *				: "D" (p), "d" (pe)
 *				: "ecx"
 *			);
 */
			do_tick();
			BAILR
		} while (!done);
	}
}

/*
 * Test all of memory using a "half moving inversions" algorithm using random
 * numbers and their complment as the data pattern. Since we are not able to
 * produce random numbers in reverse order testing is only done in the forward
 * direction.
 */
void movinvr()
{
	int i, j, done, seed1, seed2;
	volatile ulong *pe;
	volatile ulong *start,*end;
	ulong num;

	/* Initialize memory with initial sequence of random numbers.  */
	/* PPC: v->rdtsc selects a hardware time-source seed. The x86 source
	 * read the TSC via `rdtsc`; we read the PowerPC Time Base instead.
	 * mftb() is the low 32 bits and mftbu() the high 32 bits — the exact
	 * analog of rdtsc's eax/edx. The non-rdtsc default path (521288629 ±
	 * v->pass) is kept verbatim. */
	if (v->rdtsc) {
/* x86 asm: read TSC into seed1:seed2 (rdtsc) — replaced by the PPC Time Base
 * read below.
 *		asm __volatile__ ("rdtsc":"=a" (seed1),"=d" (seed2));
 */
		seed1 = (int)mftb();	/* PPC: low 32 bits of the Time Base */
		seed2 = (int)mftbu();	/* PPC: high 32 bits of the Time Base */
	} else {
		seed1 = 521288629 + v->pass;
		seed2 = 362436069 - v->pass;
	}

	/* Display the current seed */
	hprint(LINE_PAT, COL_PAT, seed1);
	rand_seed(seed1, seed2);
	for (j=0; j<segs; j++) {
		start = v->map[j].start;
		end = v->map[j].end;
		pe = start;
		p = start;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ > pe) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
/* Original C code replaced with hand tuned assembly code */
			/* PPC: enable the upstream plain-C fill loop. */
			for (; p < pe; p++) {
				*p = rand();
			}

/* x86 asm: random fill loop (call rand; movl %eax,(%edi); addl $4) — replaced
 * by the C loop above.
 *			asm __volatile__ (
 *				"jmp L200\n\t"
 *				".p2align 4,,7\n\t"
 *				"L200:\n\t"
 *				"call rand\n\t"
 *				"movl %%eax,(%%edi)\n\t"
 *				"addl $4,%%edi\n\t"
 *				"cmpl %%ebx,%%edi\n\t"
 *				"jb L200\n\t"
 *				: "=D" (p)
 *				: "D" (p), "b" (pe)
 *				: "eax"
 *			);
 */

			do_tick();
			BAILR
		} while (!done);
	}

	/* Do moving inversions test. Check for initial pattern and then
	 * write the complement for each memory location. Test from bottom
	 * up and then from the top down.  */
	for (i=0; i<2; i++) {
		rand_seed(seed1, seed2);
		for (j=0; j<segs; j++) {
			start = v->map[j].start;
			end = v->map[j].end;
			pe = start;
			p = start;
			done = 0;
			do {
				/* Check for overflow */
				if (pe + SPINSZ > pe) {
					pe += SPINSZ;
				} else {
					pe = end;
				}
				if (pe >= end) {
					pe = end;
					done++;
				}
				if (p == pe ) {
					break;
				}
/* Original C code replaced with hand tuned assembly code */
				/* PPC: enable the upstream plain-C
				 * check-and-complement loop. (The asm version
				 * folded the i==0/i==1 complement into an xor
				 * with num=0/0xffffffff; the plain C uses the
				 * `if (i) num = ~num;` form directly.) */
				{
					ulong bad;
					for (; p < pe; p++) {
						num = rand();
						if (i) {
							num = ~num;
						}
						if ((bad=*p) != num) {
							error((ulong*)p, num, bad);
						}
						*p = ~num;
					}
				}
/* x86 asm: check-and-complement loop with xor-folded complement (call rand;
 * xorl %ebx,%eax; cmpl; jne -> call error; movl $0xffffffff xor; store ~num) —
 * replaced by the C loop above. The `num = 0 / 0xffffffff` set just before the
 * asm was the xor mask; the C loop folds it back into `if (i) num = ~num`.
 *				if (i) {
 *					num = 0xffffffff;
 *				} else {
 *					num = 0;
 *				}
 *				asm __volatile__ (
 *					"jmp L26\n\t" \
 *
 *					".p2align 4,,7\n\t" \
 *					"L26:\n\t" \
 *					"call rand\n\t"
 *					"xorl %%ebx,%%eax\n\t" \
 *					"movl (%%edi),%%ecx\n\t" \
 *					"cmpl %%eax,%%ecx\n\t" \
 *					"jne L23\n\t" \
 *					"L25:\n\t" \
 *					"movl $0xffffffff,%%edx\n\t" \
 *					"xorl %%edx,%%eax\n\t" \
 *					"movl %%eax,(%%edi)\n\t" \
 *					"addl $4,%%edi\n\t" \
 *					"cmpl %%esi,%%edi\n\t" \
 *					"jb L26\n\t" \
 *					"jmp L24\n" \
 *
 *					"L23:\n\t" \
 *					"pushl %%esi\n\t" \
 *					"pushl %%ecx\n\t" \
 *					"pushl %%eax\n\t" \
 *					"pushl %%edi\n\t" \
 *					"call error\n\t" \
 *					"popl %%edi\n\t" \
 *					"popl %%eax\n\t" \
 *					"popl %%ecx\n\t" \
 *					"popl %%esi\n\t" \
 *					"jmp L25\n" \
 *
 *					"L24:\n\t" \
 *					: "=D" (p)
 *					: "D" (p), "S" (pe), "b" (num)
 *					: "eax", "ecx", "edx"
 *				);
 */
				do_tick();
				BAILR
			} while (!done);
		}
	}
}

/*
 * Test all of memory using a "moving inversions" algorithm using the
 * pattern in p1 and it's complement in p2.
 */
void movinv1(int iter, ulong p1, ulong p2)
{
	int i, j, done;
	volatile ulong *pe;
	volatile ulong len;
	volatile ulong *start,*end;

	/* Display the current pattern */
	hprint(LINE_PAT, COL_PAT, p1);

	/* Initialize memory with the initial pattern.  */
	for (j=0; j<segs; j++) {
		start = v->map[j].start;
		end = v->map[j].end;
		pe = start;
		p = start;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ > pe) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			len = pe - p;
			if (p == pe ) {
				break;
			}
/* Original C code replaced with hand tuned assembly code */
			/* PPC: enable the upstream plain-C fill loop. The x86
			 * asm was `rep stosl` (fill `len` longs with p1); the C
			 * loop is the faithful equivalent. */
			for (; p < pe; p++) {
				*p = p1;
			}
/* x86 asm: rep stosl pattern fill — replaced by the C loop above.
 *			asm __volatile__ (
 *				"rep\n\t" \
 *				"stosl\n\t"
 *				: "=D" (p)
 *				: "c" (len), "0" (p), "a" (p1)
 *			);
 */
			do_tick();
			BAILR
		} while (!done);
	}

	/* Do moving inversions test. Check for initial pattern and then
	 * write the complement for each memory location. Test from bottom
	 * up and then from the top down.  */
	for (i=0; i<iter; i++) {
		for (j=0; j<segs; j++) {
			start = v->map[j].start;
			end = v->map[j].end;
			pe = start;
			p = start;
			done = 0;
			do {
				/* Check for overflow */
				if (pe + SPINSZ > pe) {
					pe += SPINSZ;
				} else {
					pe = end;
				}
				if (pe >= end) {
					pe = end;
					done++;
				}
				if (p == pe ) {
					break;
				}
/* Original C code replaced with hand tuned assembly code */
				/* PPC: enable the upstream plain-C forward
				 * check-and-write loop. */
				{
					ulong bad;
					for (; p < pe; p++) {
						if ((bad=*p) != p1) {
							error((ulong*)p, p1, bad);
						}
						*p = p2;
					}
				}
/* x86 asm: forward check-p1-then-write-p2 loop — replaced by the C loop above.
 *				asm __volatile__ (
 *					"jmp L2\n\t" \
 *
 *					".p2align 4,,7\n\t" \
 *					"L2:\n\t" \
 *					"movl (%%edi),%%ecx\n\t" \
 *					"cmpl %%eax,%%ecx\n\t" \
 *					"jne L3\n\t" \
 *					"L5:\n\t" \
 *					"movl %%ebx,(%%edi)\n\t" \
 *					"addl $4,%%edi\n\t" \
 *					"cmpl %%edx,%%edi\n\t" \
 *					"jb L2\n\t" \
 *					"jmp L4\n" \
 *
 *					"L3:\n\t" \
 *					"pushl %%edx\n\t" \
 *					"pushl %%ebx\n\t" \
 *					"pushl %%ecx\n\t" \
 *					"pushl %%eax\n\t" \
 *					"pushl %%edi\n\t" \
 *					"call error\n\t" \
 *					"popl %%edi\n\t" \
 *					"popl %%eax\n\t" \
 *					"popl %%ecx\n\t" \
 *					"popl %%ebx\n\t" \
 *					"popl %%edx\n\t" \
 *					"jmp L5\n" \
 *
 *					"L4:\n\t" \
 *					: "=D" (p)
 *					: "a" (p1), "0" (p), "d" (pe), "b" (p2)
 *					: "ecx"
 *				);
 */
				do_tick();
				BAILR
			} while (!done);
		}
		for (j=segs-1; j>=0; j--) {
			start = v->map[j].start;
			end = v->map[j].end;
			pe = end -1;
			p = end -1;
			done = 0;
			do {
				/* x86: this underflow guard relied on pointer-arithmetic
				 * wraparound — `pe - SPINSZ` underflowing below address 0 becomes
				 * a huge value, so `< pe` is false and the else clamps to start.
				 * That is undefined behavior; powerpc-linux-gnu-gcc -O1 folds
				 * `pe - SPINSZ < pe` to always-true, killing the clamp. So for a
				 * segment whose start is below SPINSZ (e.g. our low 8 MB seg) the
				 * reverse walk steps past the bottom, wraps to ~0xFFFFFFFF, and
				 * never satisfies `pe <= start` (the SPINSZ stride skips [0,start])
				 * -> infinite do_tick loop. PPC: underflow-safe check using the
				 * byte distance to start (well-defined; in-segment pe >= start). */
				if ((ulong)pe - (ulong)start >= (ulong)SPINSZ * sizeof(ulong)) {
					pe -= SPINSZ;
				} else {
					pe = start;
				}
				if (pe <= start) {
					pe = start;
					done++;
				}
				if (p == pe ) {
					break;
				}
/* Original C code replaced with hand tuned assembly code */
				/* PPC: enable the upstream plain-C reverse
				 * check-and-write loop. Note the post-decrement
				 * `while (p-- > pe)` walks DOWN to and including
				 * pe, matching the asm's `subl $4`/`cmpl %edi,%edx`
				 * structure. */
				{
					ulong bad;
					do {
						if ((bad=*p) != p2) {
							error((ulong*)p, p2, bad);
						}
						*p = p1;
					} while (p-- > pe);
				}
/* x86 asm: reverse check-p2-then-write-p1 loop — replaced by the C loop above.
 *				asm __volatile__ (
 *					"addl $4, %%edi\n\t"
 *					"jmp L9\n\t"
 *
 *					".p2align 4,,7\n\t"
 *					"L9:\n\t"
 *					"subl $4, %%edi\n\t"
 *					"movl (%%edi),%%ecx\n\t"
 *					"cmpl %%ebx,%%ecx\n\t"
 *					"jne L6\n\t"
 *					"L10:\n\t"
 *					"movl %%eax,(%%edi)\n\t"
 *					"cmpl %%edi, %%edx\n\t"
 *					"jne L9\n\t"
 *					"subl $4, %%edi\n\t"
 *					"jmp L7\n\t"
 *
 *					"L6:\n\t"
 *					"pushl %%edx\n\t"
 *					"pushl %%eax\n\t"
 *					"pushl %%ecx\n\t"
 *					"pushl %%ebx\n\t"
 *					"pushl %%edi\n\t"
 *					"call error\n\t"
 *					"popl %%edi\n\t"
 *					"popl %%ebx\n\t"
 *					"popl %%ecx\n\t"
 *					"popl %%eax\n\t"
 *					"popl %%edx\n\t"
 *					"jmp L10\n"
 *
 *					"L7:\n\t"
 *					: "=D" (p)
 *					: "a" (p1), "0" (p), "d" (pe), "b" (p2)
 *					: "ecx"
 *				);
 */
				do_tick();
				BAILR
			} while (!done);
		}
	}
}

void movinv32(int iter, ulong p1, ulong lb, ulong hb, int sval, int off)
{
	int i, j, k=0, done;
	volatile ulong *pe;
	volatile ulong *start, *end;
	ulong pat = 0;

	ulong p3 = sval << 31;	/* PPC: re-enabled — needed by the plain-C
				 * reverse pass below. Upstream commented it out
				 * (the "CDH" asm rewrite folded it into the
				 * roll/ror), but our C reverse loop uses it. */

	/* Display the current pattern */
	hprint(LINE_PAT, COL_PAT, p1);

	/* Initialize memory with the initial pattern.  */
	for (j=0; j<segs; j++) {
		start = v->map[j].start;
		end = v->map[j].end;
		pe = start;
		p = start;
		done = 0;
		k = off;
		pat = p1;
		do {
			/* Check for overflow */
			if (pe + SPINSZ > pe) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
			/* Do a SPINSZ section of memory */
/* Original C code replaced with hand tuned assembly code */
			/* PPC: enable the upstream plain-C fill loop. NOTE the
			 * asm (the "CDH" rewrite) shifts the pattern via
			 * `roll $1,%ecx` (a plain 1-bit rotate, sval folded out)
			 * and tracks k in %bl with `incb`/`andb $31`. The
			 * upstream-commented C uses the original lb/sval form
			 * (`pat = lb; ... pat = pat<<1; pat |= sval`). We enable
			 * the original-C form for fidelity to the documented
			 * algorithm. */
			while (p < pe) {
				*p = pat;
				if (++k >= 32) {
					pat = lb;
					k = 0;
				} else {
					pat = pat << 1;
					pat |= sval;
				}
				p++;
			}
/* x86 asm: forward pattern fill, rotate-left pattern advance (movl %ecx,(%edi);
 * incb %bl; roll $1,%ecx; andb $31,%bl) — replaced by the C loop above.
 *			asm __volatile__ (
 *				"jmp L20\n\t"
 *				".p2align 4,,7\n\t"
 *
 *				"L20:\n\t"
 *				"movl %%ecx,(%%edi)\n\t"
 *				"incb %%bl\n\t"
 *				"addl $4,%%edi\n\t"
 *				"roll $1,%%ecx\n\t"
 *				"cmpl %%edx,%%edi\n\t"
 *				"jb L20\n\t"
 *				"andb $31,%%bl\n\t"
 *				: "=b" (k), "=D" (p), "=c" (pat)
 *				: "D" (p),"d" (pe),"b" (k),"c" (pat)
 *			);
 */
			do_tick();
			BAILR
		} while (!done);
	}

	/* Do moving inversions test. Check for initial pattern and then
	 * write the complement for each memory location. Test from bottom
	 * up and then from the top down.  */
	for (i=0; i<iter; i++) {
		for (j=0; j<segs; j++) {
			start = v->map[j].start;
			end = v->map[j].end;
			pe = start;
			p = start;
			done = 0;
			k = off;
			pat = p1;
			do {
				/* Check for overflow */
				if (pe + SPINSZ > pe) {
					pe += SPINSZ;
				} else {
					pe = end;
				}
				if (pe >= end) {
					pe = end;
					done++;
				}
				if (p == pe ) {
					break;
				}
/* Original C code replaced with hand tuned assembly code */
				/* PPC: enable the upstream plain-C forward
				 * check-and-complement loop. */
				{
					ulong bad;
					while (p < pe) {
						if ((bad=*p) != pat) {
							error((ulong*)p, pat, bad);
						}
						*p = ~pat;
						if (++k >= 32) {
							pat = lb;
							k = 0;
						} else {
							pat = pat << 1;
							pat |= sval;
						}
						p++;
					}
				}
/* x86 asm: forward check-pat-then-write-~pat loop, rotate-left pattern advance
 * (the "CDH" rewrite) — replaced by the C loop above.
 *				asm __volatile__ (
 *					"pushl %%ebp\n\t"
 *					"jmp L30\n\t"
 *
 *					".p2align 4,,7\n\t"
 *					"L30:\n\t"
 *					"movl (%%edi),%%ebp\n\t"
 *					"cmpl %%ecx,%%ebp\n\t"
 *					"jne L34\n\t"
 *
 *					"L35:\n\t"
 *					"notl %%ecx\n\t"
 *					"movl %%ecx,(%%edi)\n\t"
 *					"notl %%ecx\n\t"
 *					"addl $4,%%edi\n\t"
 *					"incb %%bl\n\t"
 *					"roll $1,%%ecx\n\t"
 *					"cmpl %%edx,%%edi\n\t"
 *					"jb L30\n\t"
 *					"jmp L33\n\t"
 *
 *					"L34:\n\t" \
 *					"pushl %%esi\n\t"
 *					"pushl %%eax\n\t"
 *					"pushl %%ebx\n\t"
 *					"pushl %%edx\n\t"
 *					"pushl %%ebp\n\t"
 *					"pushl %%ecx\n\t"
 *					"pushl %%edi\n\t"
 *					"call error\n\t"
 *					"popl %%edi\n\t"
 *					"popl %%ecx\n\t"
 *					"popl %%ebp\n\t"
 *					"popl %%edx\n\t"
 *					"popl %%ebx\n\t"
 *					"popl %%eax\n\t"
 *					"popl %%esi\n\t"
 *					"jmp L35\n"
 *
 *					"L33:\n\t"
 *					"andb $31,%%bl\n\t"
 *					"popl %%ebp\n\t"
 *					: "=b" (k), "=D" (p), "=c" (pat)
 *					: "D" (p),"d" (pe),"b" (k),"c" (pat)
 *				);
 */
				do_tick();
				BAILR
			} while (!done);
		}

		/* Since we already adjusted k and the pattern this
		 * code backs both up one step
		 */
/* Original C code replaced with hand tuned assembly code */
		/* PPC: enable the upstream plain-C "back up one step" of k and
		 * the pattern before the reverse pass. The asm (decl/andl/roll
		 * %cl/incb) is the same arithmetic. */
		pat = lb;
		if ( 0 != (k = (k-1) & 31) ) {
			pat = (pat << k);
			if ( sval )
			pat |= ((sval << k) - 1);
		}
		k++;
/* x86 asm: back-up-one-step of k and pat (decl %ecx; andl $31; roll %cl,%ebx;
 * incb %cl) — replaced by the C above.
 *			asm __volatile__ (
 *			"decl %%ecx\n\t"
 *			"andl $31,%%ecx\n\t"
 *			"roll %%cl,%%ebx\n\t"
 *			"incb %%cl\n\t"
 *			: "=c" (k), "=b" (pat)
 *			: "c" (k), "b" (lb)
 *			);
 */

		for (j=segs-1; j>=0; j--) {
			start = v->map[j].start;
			end = v->map[j].end;
			p = end -1;
			pe = end -1;
			done = 0;
			do {
				/* x86: this underflow guard relied on pointer-arithmetic
				 * wraparound — `pe - SPINSZ` underflowing below address 0 becomes
				 * a huge value, so `< pe` is false and the else clamps to start.
				 * That is undefined behavior; powerpc-linux-gnu-gcc -O1 folds
				 * `pe - SPINSZ < pe` to always-true, killing the clamp. So for a
				 * segment whose start is below SPINSZ (e.g. our low 8 MB seg) the
				 * reverse walk steps past the bottom, wraps to ~0xFFFFFFFF, and
				 * never satisfies `pe <= start` (the SPINSZ stride skips [0,start])
				 * -> infinite do_tick loop. PPC: underflow-safe check using the
				 * byte distance to start (well-defined; in-segment pe >= start). */
				if ((ulong)pe - (ulong)start >= (ulong)SPINSZ * sizeof(ulong)) {
					pe -= SPINSZ;
				} else {
					pe = start;
				}
				if (pe <= start) {
					pe = start;
					done++;
				}
				if (p == pe ) {
					break;
				}
/* Original C code replaced with hand tuned assembly code */
				/* PPC: enable the upstream plain-C reverse
				 * check-and-write loop. Uses p3 (= sval<<31)
				 * to fold sval into the right-shift, matching
				 * the documented original. */
				{
					ulong bad;
					do {
						if ((bad=*p) != ~pat) {
							error((ulong*)p, ~pat, bad);
						}
						*p = pat;
						if (--k <= 0) {
							pat = hb;
							k = 32;
						} else {
							pat = pat >> 1;
							pat |= p3;
						}
					} while (p-- > pe);
				}
/* x86 asm: reverse check-~pat-then-write-pat loop, rotate-right pattern advance
 * (the "CDH" rewrite) — replaced by the C loop above.
 *				asm __volatile__ (
 *					"pushl %%ebp\n\t"
 *					"addl $4,%%edi\n\t"
 *					"jmp L40\n\t"
 *
 *					".p2align 4,,7\n\t"
 *					"L40:\n\t"
 *					"subl $4,%%edi\n\t"
 *					"movl (%%edi),%%ebp\n\t"
 *					"notl %%ecx\n\t"
 *					"cmpl %%ecx,%%ebp\n\t"
 *					"jne L44\n\t"
 *
 *					"L45:\n\t"
 *					"notl %%ecx\n\t"
 *					"movl %%ecx,(%%edi)\n\t"
 *					"decb %%bl\n\t"
 *					"rorl $1,%%ecx\n\t"
 *					"cmpl %%edx,%%edi\n\t"
 *					"ja L40\n\t"
 *					"jmp L43\n\t"
 *
 *					"L44:\n\t" \
 *					"pushl %%esi\n\t"
 *					"pushl %%eax\n\t"
 *					"pushl %%ebx\n\t"
 *					"pushl %%edx\n\t"
 *					"pushl %%ebp\n\t"
 *					"pushl %%ecx\n\t"
 *					"pushl %%edi\n\t"
 *					"call error\n\t"
 *					"popl %%edi\n\t"
 *					"popl %%ecx\n\t"
 *					"popl %%ebp\n\t"
 *					"popl %%edx\n\t"
 *					"popl %%ebx\n\t"
 *					"popl %%eax\n\t"
 *					"popl %%esi\n\t"
 *					"jmp L45\n"
 *
 *					"L43:\n\t"
 *					"andb $31,%%bl\n\t"
 *					"subl $4,%%edi\n\t"
 *					"popl %%ebp\n\t"
 *					: "=b" (k), "=D" (p), "=c" (pat)
 *					: "D" (p),"d" (pe),"b" (k),"c" (pat)
 *				);
 */
				do_tick();
				BAILR
			} while (!done);
		}
	}
}

/*
 * Test all of memory using modulo X access pattern.
 */
void modtst(int offset, int iter, ulong p1, ulong p2)
{
	int j, k, l, done;
	volatile ulong *pe;
	volatile ulong *start, *end;

	/* Display the current pattern */
	hprint(LINE_PAT, COL_PAT-2, p1);
	cprint(LINE_PAT, COL_PAT+6, "-");
	dprint(LINE_PAT, COL_PAT+7, offset, 2, 1);

	/* Write every nth location with pattern */
	for (j=0; j<segs; j++) {
		start = v->map[j].start;
		end = v->map[j].end;
		pe = (ulong *)start;
		p = start+offset;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ > pe) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
/* Original C code replaced with hand tuned assembly code */
			/* PPC: enable the upstream plain-C every-nth fill loop.
			 * The asm advances %edi by 80 bytes (= 20 longs =
			 * MOD_SZ longs) per store, matching `p += MOD_SZ`. */
			for (; p < pe; p += MOD_SZ) {
				*p = p1;
			}
/* x86 asm: every-MOD_SZ-th store of p1 (movl %eax,(%edi); addl $80) — replaced
 * by the C loop above.
 *			asm __volatile__ (
 *				"jmp L60\n\t" \
 *				".p2align 4,,7\n\t" \
 *
 *				"L60:\n\t" \
 *				"movl %%eax,(%%edi)\n\t" \
 *				"addl $80,%%edi\n\t" \
 *				"cmpl %%edx,%%edi\n\t" \
 *				"jb L60\n\t" \
 *				: "=D" (p)
 *				: "D" (p), "d" (pe), "a" (p1)
 *			);
 */
			do_tick();
			BAILR
		} while (!done);
	}

	/* Write the rest of memory "iter" times with the pattern complement */
	for (l=0; l<iter; l++) {
		for (j=0; j<segs; j++) {
			start = v->map[j].start;
			end = v->map[j].end;
			pe = (ulong *)start;
			p = start;
			done = 0;
			k = 0;
			do {
				/* Check for overflow */
				if (pe + SPINSZ > pe) {
					pe += SPINSZ;
				} else {
					pe = end;
				}
				if (pe >= end) {
					pe = end;
					done++;
				}
				if (p == pe ) {
					break;
				}
/* Original C code replaced with hand tuned assembly code */
				/* PPC: enable the upstream plain-C "fill all but
				 * every nth" loop. The asm's `cmpl $19` (= MOD_SZ-1)
				 * / `xorl %ebx,%ebx` is the `if (++k > MOD_SZ-1)
				 * k = 0;` wrap. */
				for (; p < pe; p++) {
					if (k != offset) {
						*p = p2;
					}
					if (++k > MOD_SZ-1) {
						k = 0;
					}
				}
/* x86 asm: fill-all-but-every-nth with p2 (cmpl %ebx,%ecx; je skip; store;
 * incl %ebx; cmpl $19; wrap) — replaced by the C loop above.
 *				asm __volatile__ (
 *					"jmp L50\n\t" \
 *					".p2align 4,,7\n\t" \
 *
 *					"L50:\n\t" \
 *					"cmpl %%ebx,%%ecx\n\t" \
 *					"je L52\n\t" \
 *					  "movl %%eax,(%%edi)\n\t" \
 *					"L52:\n\t" \
 *					"incl %%ebx\n\t" \
 *					"cmpl $19,%%ebx\n\t" \
 *					"jle L53\n\t" \
 *					  "xorl %%ebx,%%ebx\n\t" \
 *					"L53:\n\t" \
 *					"addl $4,%%edi\n\t" \
 *					"cmpl %%edx,%%edi\n\t" \
 *					"jb L50\n\t" \
 *					: "=D" (p), "=b" (k)
 *					: "D" (p), "d" (pe), "a" (p2),
 *						"b" (k), "c" (offset)
 *				);
 */
				do_tick();
				BAILR
			} while (!done);
		}
	}

	/* Now check every nth location */
	for (j=0; j<segs; j++) {
		start = v->map[j].start;
		end = v->map[j].end;
		pe = (ulong *)start;
		p = start+offset;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ > pe) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
/* Original C code replaced with hand tuned assembly code */
			/* PPC: enable the upstream plain-C every-nth check loop. */
			{
				ulong bad;
				for (; p < pe; p += MOD_SZ) {
					if ((bad=*p) != p1) {
						error((ulong*)p, p1, bad);
					}
				}
			}
/* x86 asm: every-MOD_SZ-th check of p1 (movl (%edi),%ecx; cmpl; jne -> call
 * error; addl $80) — replaced by the C loop above.
 *			asm __volatile__ (
 *				"jmp L70\n\t" \
 *				".p2align 4,,7\n\t" \
 *
 *				"L70:\n\t" \
 *				"movl (%%edi),%%ecx\n\t" \
 *				"cmpl %%eax,%%ecx\n\t" \
 *				"jne L71\n\t" \
 *				"L72:\n\t" \
 *				"addl $80,%%edi\n\t" \
 *				"cmpl %%edx,%%edi\n\t" \
 *				"jb L70\n\t" \
 *				"jmp L73\n\t" \
 *
 *				"L71:\n\t" \
 *				"pushl %%edx\n\t"
 *				"pushl %%ecx\n\t"
 *				"pushl %%eax\n\t"
 *				"pushl %%edi\n\t"
 *				"call error\n\t"
 *				"popl %%edi\n\t"
 *				"popl %%eax\n\t"
 *				"popl %%ecx\n\t"
 *				"popl %%edx\n\t"
 *				"jmp L72\n"
 *
 *				"L73:\n\t" \
 *				: "=D" (p)
 *				: "D" (p), "d" (pe), "a" (p1)
 *				: "ecx"
 *			);
 */
			do_tick();
			BAILR
		} while (!done);
	}
	cprint(LINE_PAT, COL_PAT, "          ");
}



/*
 * Test memory using block moves
 * Adapted from Robert Redelmeier's burnBX test
 */
void block_move(int iter)
{
	int i, j, done;
	ulong len;
	volatile ulong p, pe, pp;
	volatile ulong start, end;

	cprint(LINE_PAT, COL_PAT-2, "          ");

	/* Initialize memory with the initial pattern.  */
	for (j=0; j<segs; j++) {
		start = (ulong)v->map[j].start;
#ifdef USB_WAR
		/* We can't do the block move test on low memory beacuase
		 * BIOS USB support clobbers location 0x410 and 0x4e0
		 */
		if (start < 0x4f0) {
			start = 0x4f0;
		}
#endif
		end = (ulong)v->map[j].end;
		pe = start;
		p = start;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ*4 > pe) {
				pe += SPINSZ*4;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
			len  = ((ulong)pe - (ulong)p) / 64;
/* x86 asm: 64-byte block pattern init. Each 64-byte block writes the pattern
 * (%eax) into 12 of the 16 longs and its complement (%edx = ~%eax) into the
 * other 4 (offsets 16,20,40,44 / 56,60... per the original burnBX layout),
 * then advances the pattern by `rcll $1, %%eax` — a 32-bit ROTATE THROUGH CARRY
 * (33-bit rotate: bit31 -> CF, old CF -> bit0). The `decl %%ecx` that follows
 * does NOT touch CF, and `leal` doesn't either, so CF survives between
 * iterations: the pattern is a genuine 33-bit rotate, NOT a plain rotate.
 * Replaced by the carry-faithful C loop below (a plain `<<` or a `rol` would be
 * WRONG — the carry bit matters).
 *			asm __volatile__ (
 *				"jmp L100\n\t"
 *
 *				".p2align 4,,7\n\t"
 *				"L100:\n\t"
 *				"movl %%eax, %%edx\n\t"
 *				"notl %%edx\n\t"
 *				"movl %%eax,0(%%edi)\n\t"
 *				"movl %%eax,4(%%edi)\n\t"
 *				"movl %%eax,8(%%edi)\n\t"
 *				"movl %%eax,12(%%edi)\n\t"
 *				"movl %%edx,16(%%edi)\n\t"
 *				"movl %%edx,20(%%edi)\n\t"
 *				"movl %%eax,24(%%edi)\n\t"
 *				"movl %%eax,28(%%edi)\n\t"
 *				"movl %%eax,32(%%edi)\n\t"
 *				"movl %%eax,36(%%edi)\n\t"
 *				"movl %%edx,40(%%edi)\n\t"
 *				"movl %%edx,44(%%edi)\n\t"
 *				"movl %%eax,48(%%edi)\n\t"
 *				"movl %%eax,52(%%edi)\n\t"
 *				"movl %%edx,56(%%edi)\n\t"
 *				"movl %%edx,60(%%edi)\n\t"
 *				"rcll $1, %%eax\n\t"
 *				"leal 64(%%edi), %%edi\n\t"
 *				"decl %%ecx\n\t"
 *				"jnz  L100\n\t"
 *				: "=D" (p)
 *				: "D" (p), "c" (len), "a" (1)
 *				: "edx"
 *			);
 */
			/* PPC: carry-faithful C reproduction of the x86 init
			 * loop. `pat` starts at 1 with carry=0 (matches eax=1,
			 * and the freshly-entered asm whose CF is the boot/prior
			 * state — but the original burnBX relies only on the
			 * 33-bit rotation pattern, and a 33-bit rotate from a
			 * defined start of carry=0 is the faithful, deterministic
			 * choice). `rcl` through a 33-bit accumulator: new bit0
			 * is the old carry; new carry is old bit31. */
			{
				ulong pat = 1;	/* %eax = 1 */
				ulong carry = 0;	/* CF on entry, taken as 0 */
				volatile ulong *pp2 = (volatile ulong *)p;
				ulong cnt;
				for (cnt = 0; cnt < len; cnt++) {
					ulong inv = ~pat;	/* %edx = ~%eax */
					ulong newcarry;
					pp2[0]  = pat; pp2[1]  = pat;
					pp2[2]  = pat; pp2[3]  = pat;
					pp2[4]  = inv; pp2[5]  = inv;
					pp2[6]  = pat; pp2[7]  = pat;
					pp2[8]  = pat; pp2[9]  = pat;
					pp2[10] = inv; pp2[11] = inv;
					pp2[12] = pat; pp2[13] = pat;
					pp2[14] = inv; pp2[15] = inv;
					/* rcll $1, %%eax: 33-bit rotate-through-carry */
					newcarry = (pat >> 31) & 1;
					pat = ((pat << 1) | carry) & 0xffffffffUL;
					carry = newcarry;
					pp2 += 16;
				}
				p = (ulong)pp2;
			}
			do_tick();
			BAILR
		} while (!done);
	}

	/* Now move the data around
	 * First move the data up half of the segment size we are testing
	 * Then move the data to the original location + 32 bytes
	 */
	for (j=0; j<segs; j++) {
		start = (ulong)v->map[j].start;
#ifdef USB_WAR
		/* We can't do the block move test on low memory beacuase
		 * BIOS USB support clobbers location 0x410 and 0x4e0
		 */
		if (start < 0x4f0) {
			start = 0x4f0;
		}
#endif
		end = (ulong)v->map[j].end;
		pe = start;
		p = start;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ*4 > pe) {
				pe += SPINSZ*4;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
			pp = p + ((pe - p) / 2);
			len  = ((ulong)pe - (ulong)p) / 8;
			for(i=0; i<iter; i++) {
/* x86 asm: three `rep movsl` block copies implementing the burnBX shuffle:
 *   (1) copy `len` longs from p  -> pp        (first half to second half)
 *   (2) copy `len-8` longs from pp -> p+32    (second half back to first+32)
 *   (3) copy 8 longs from pp+... -> p         (the trailing 8 longs to start)
 * Wait: the asm uses esi=source, edi=dest, movsl copies ecx longs src->dst,
 * incrementing both (cld). Replaced by the equivalent memmove copies below.
 * Note operands 0,1,2 are p, pp, len respectively.
 *				asm __volatile__ (
 *					"cld\n"
 *					"jmp L110\n\t"
 *
 *					".p2align 4,,7\n\t"
 *					"L110:\n\t"
 *					"movl %1,%%edi\n\t"	// edi = pp (dest)
 *					"movl %0,%%esi\n\t"	// esi = p  (src)
 *					"movl %2,%%ecx\n\t"	// ecx = len
 *					"rep\n\t"
 *					"movsl\n\t"		// pp[0..len) = p[0..len)
 *					"movl %0,%%edi\n\t"	// edi = p (dest)
 *					"addl $32,%%edi\n\t"	// edi = p+32
 *					"movl %1,%%esi\n\t"	// esi = pp (src)
 *					"movl %2,%%ecx\n\t"
 *					"subl $8,%%ecx\n\t"	// ecx = len-8
 *					"rep\n\t"
 *					"movsl\n\t"		// (p+32)[0..len-8) = pp[0..len-8)
 *					"movl %0,%%edi\n\t"	// edi = p (dest)
 *					"movl $8,%%ecx\n\t"	// ecx = 8
 *					"rep\n\t"
 *					"movsl\n\t"		// p[0..8) = <esi advanced from prev copy>
 *					:: "g" (p), "g" (pp), "g" (len)
 *					: "edi", "esi", "ecx"
 *				);
 */
				/* PPC: faithful C reproduction of the three
				 * sequential rep-movsl block copies. The third
				 * copy's source (esi) is wherever the SECOND copy
				 * left it: esi started at pp and advanced by
				 * (len-8) longs, so it points at pp+(len-8). Each
				 * `rep movsl` copies forward (cld) ecx longs of 4
				 * bytes. We use memmove for correctness even though
				 * the regions don't overlap in the intended use. */
				/* (1) pp[0..len)   <- p[0..len)            */
				memmove((void *)pp, (void *)p, len * 4);
				/* (2) (p+32)[0..len-8) <- pp[0..len-8)     */
				memmove((void *)(p + 32), (void *)pp, (len - 8) * 4);
				/* (3) p[0..8)      <- (pp+(len-8))[0..8)   */
				memmove((void *)p,
				        (void *)(pp + (len - 8) * 4), 8 * 4);
				do_tick();
				BAILR
			}
			p = pe;
		} while (!done);
	}

	/* Now check the data
	 * The error checking is rather crude.  We just check that the
	 * adjacent words are the same.
	 */
	for (j=0; j<segs; j++) {
		start = (ulong)v->map[j].start;
#ifdef USB_WAR
		/* We can't do the block move test on low memory beacuase
		 * BIOS USB support clobbers location 0x4e0 and 0x410
		 */
		if (start < 0x4f0) {
			start = 0x4f0;
		}
#endif
		end = (ulong)v->map[j].end;
		pe = start;
		p = start;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ*4 > pe) {
				pe += SPINSZ*4;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
/* x86 asm: adjacent-word compare. Walks p in 8-byte steps comparing the long at
 * (edi) against the long at 4(edi); on mismatch pushes edx(pe), the second word
 * 4(edi), ecx(first word), edi and calls error(). Replaced by the C loop below.
 *			asm __volatile__ (
 *				"jmp L120\n\t"
 *
 *				".p2align 4,,7\n\t"
 *				"L120:\n\t"
 *				"movl (%%edi),%%ecx\n\t"
 *				"cmpl 4(%%edi),%%ecx\n\t"
 *				"jnz L121\n\t"
 *
 *				"L122:\n\t"
 *				"addl $8,%%edi\n\t"
 *				"cmpl %%edx,%%edi\n\t"
 *				"jb L120\n"
 *				"jmp L123\n\t"
 *
 *				"L121:\n\t"
 *				"pushl %%edx\n\t"
 *				"pushl 4(%%edi)\n\t"
 *				"pushl %%ecx\n\t"
 *				"pushl %%edi\n\t"
 *				"call error\n\t"
 *				"popl %%edi\n\t"
 *				"addl $8,%%esp\n\t"
 *				"popl %%edx\n\t"
 *				"jmp L122\n"
 *				"L123:\n\t"
 *				: "=D" (p)
 *				: "D" (p), "d" (pe)
 *				: "ecx"
 *			);
 */
			/* PPC: faithful C reproduction. Compare each long with
			 * the next; on mismatch call error(addr, first, second)
			 * matching the asm's push order
			 * (edi=addr, ecx=first=good, 4(edi)=second=bad). */
			{
				volatile ulong *q  = (volatile ulong *)p;
				volatile ulong *qe = (volatile ulong *)pe;
				ulong w0, w1;
				while (q < qe) {
					w0 = q[0];
					w1 = q[1];
					if (w0 != w1) {
						error((ulong *)q, w0, w1);
					}
					q += 2;	/* addl $8 */
				}
				p = (ulong)q;
			}
			do_tick();
			BAILR
		} while (!done);
	}
}

/*
 * Test memory for bit fade.
 */
#define STIME 5400
void bit_fade()
{
	int j;
	volatile ulong *pe;
	volatile ulong bad;
	volatile ulong *start,*end;

	test_ticks += (STIME * 2);
	v->pass_ticks += (STIME * 2);

	/* Do -1 and 0 patterns */
	p1 = 0;
	while (1) {

		/* Display the current pattern */
		hprint(LINE_PAT, COL_PAT, p1);

		/* Initialize memory with the initial pattern.  */
		for (j=0; j<segs; j++) {
			start = v->map[j].start;
			end = v->map[j].end;
			pe = start;
			p = start;
			for (p=start; p<end; p++) {
				*p = p1;
			}
			do_tick();
			BAILR
		}
		/* Snooze for 90 minutes */
		sleep (STIME, 0);

		/* Make sure that nothing changed while sleeping */
		for (j=0; j<segs; j++) {
			start = v->map[j].start;
			end = v->map[j].end;
			pe = start;
			p = start;
			for (p=start; p<end; p++) {
				if ((bad=*p) != p1) {
					error((ulong*)p, p1, bad);
				}
			}
			do_tick();
			BAILR
		}
		if (p1 == 0) {
			p1=-1;
		} else {
			break;
		}
	}
}


/* Sleep function */

void sleep(int n, int sms)
{
	int i, ip;
	ulong sh, sl, l, h, t;
	/* PPC: timebase frequency (ticks/sec) for the busy-wait delay; replaces
	 * the x86 v->clks_msec (TSC-cycles per millisecond) math below. Read
	 * once per call from Open Firmware. */
	ulong tb_freq = ofw_get_timebase_freq();

	ip = 0;
	/* save the starting time */
/* x86: read the starting TSC into sh:sl via rdtsc. Replaced by the PPC Time
 * Base read below (mftbu = high, mftb = low). The Time Base is a free-running
 * 64-bit counter ticking at `timebase-frequency` Hz — the PPC analog of the
 * x86 TSC for wall-clock delays. */
/*	asm __volatile__(
 *		"rdtsc":"=a" (sl),"=d" (sh));
 */
	/* PPC: read 64-bit Time Base atomically (re-read high half to guard
	 * against a low-half rollover between the two reads). */
	{
		ulong sh2;
		do {
			sh  = mftbu();
			sl  = mftb();
			sh2 = mftbu();
		} while (sh != sh2);
	}

	/* loop for n seconds */
	while (1) {
/* x86: read current TSC, then 64-bit subtract (start - now) via subl/sbbl into
 * h:l. Replaced by the PPC Time Base read + 64-bit subtract below. */
/*		asm __volatile__(
 *			"rdtsc":"=a" (l),"=d" (h));
 *		asm __volatile__ (
 *			"subl %2,%0\n\t"
 *			"sbbl %3,%1"
 *			:"=a" (l), "=d" (h)
 *			:"g" (sl), "g" (sh),
 *			"0" (l), "1" (h));
 */
		/* PPC: read the current Time Base. */
		{
			ulong h2;
			do {
				h  = mftbu();
				l  = mftb();
				h2 = mftbu();
			} while (h != h2);
		}
		/* PPC: 64-bit elapsed = (h:l) - (sh:sl), borrow-aware. The x86
		 * subl/sbbl did exactly this; we open-code the borrow. */
		{
			ulong nl = l - sl;
			ulong borrow = (l < sl) ? 1UL : 0UL;
			h = h - sh - borrow;
			l = nl;
		}

/* x86: convert the elapsed TSC delta (h:l) to milliseconds (sms!=0) or seconds
 * (sms==0) using v->clks_msec (TSC cycles per ms). Replaced by the PPC version
 * below, which uses the Time Base frequency (ticks/sec) directly. */
/*		if (sms != 0) {
 *			t = h * ((unsigned)0xffffffff / v->clks_msec);
 *			t += (l / v->clks_msec);
 *		} else {
 *			t = h * ((unsigned)0xffffffff / v->clks_msec) / 1000;
 *			t += (l / v->clks_msec) / 1000;
 *		}
 */
		/* PPC: elapsed ticks (h:l, 64-bit) -> ms or s. tb_freq is
		 * ticks/sec; ticks/ms = tb_freq/1000. The high-word term scales
		 * 2^32 ticks into the chosen unit; the low-word term divides the
		 * low 32 bits directly. Mirrors the structure of the x86 math
		 * (high*0xffffffff/clks + low/clks) with tb_freq in the divisor. */
		if (sms != 0) {
			ulong tb_ms = tb_freq / 1000;
			if (tb_ms == 0) tb_ms = 1;
			t = h * ((unsigned)0xffffffff / tb_ms);
			t += (l / tb_ms);
		} else {
			ulong tb_s = tb_freq;
			if (tb_s == 0) tb_s = 1;
			t = h * ((unsigned)0xffffffff / tb_s);
			t += (l / tb_s);
		}

		/* Is the time up? */
		if (t >= n) {
			break;
		}

		/* Display the elapsed time on the screen */
		if (sms == 0) {

			i = t % 60;
			dprint(LINE_TIME, COL_TIME+9, i%10, 1, 0);
			dprint(LINE_TIME, COL_TIME+8, i/10, 1, 0);

			if (i != ip) {
				check_input();
				ip = i;
			}

			t /= 60;
			i = t % 60;
			dprint(LINE_TIME, COL_TIME+6, i % 10, 1, 0);
			dprint(LINE_TIME, COL_TIME+5, i / 10, 1, 0);
			t /= 60;
			dprint(LINE_TIME, COL_TIME, t, 4, 0);
			BAILR
		}
	}
}

/* Beep function */
/* x86: beep() drives the PC speaker via the 8254 timer (port 0x42/0x43) and the
 * keyboard controller port 0x61 — all x86 port I/O, N/A on PPC/OF (no PC
 * speaker; OF gives us no equivalent). v2.00 only calls beep() from the
 * beep-on-error path in main.c (v->beepmode), which we leave inert (config.h
 * keeps beepmode defined at an inert value). Commented out wholesale rather
 * than deleted so the original is on the record; if any caller survives the
 * main.c port it must be commented there too. */
/*
void beep(unsigned int frequency)
{
	unsigned int count = 1193180 / frequency;

	// Switch on the speaker
	outb_p(inb_p(0x61)|3, 0x61);

	// Set command for counter 2, 2 byte write
	outb_p(0xB6, 0x43);

	// Select desired Hz
	outb_p(count & 0xff, 0x42);
	outb((count >> 8) & 0xff, 0x42);

	// Block for 100 microseconds
	sleep(100, 1);

	// Switch off the speaker
	outb(inb_p(0x61)&0xFC, 0x61);
}
*/
