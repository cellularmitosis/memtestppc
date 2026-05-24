
/* error.c - MemTest-86  Version 3.4
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady, cbrady@sgi.com
 * ----------------------------------------------------
 * MemTest86+ V2.00 Specific code (GPL V2.0)
 * By Samuel DEMEULEMEESTER, sdemeule@memtest.org
 * http://www.x86-secret.com - http://www.memtest.org
 * ----------------------------------------------------
 * memtestppc+ port: line-faithful port of memtest86+ v2.00 error.c to
 * PowerPC / Open Firmware (single-CPU). Discipline (see
 * docs/sessions/006-port-fidelity-restructure/PORTING-GUIDE.md): start from
 * the verbatim v2.00 file; comment out x86/PC leaf code with a one-line
 * reason; add PPC code inline marked "PPC:"; never delete; keep buildable.
 * Nothing from the old attic/src-v0.01 hack is inherited.
 *
 * The deliverable here is the verbatim error table (common_err's
 * PRINTMODE_ADDRESSES branch and the rest of the print modes). Only three
 * leaves change for PPC: (1) the red error-row attr poke now needs an explicit
 * fb_render_cell() because vga_buf is a shadow buffer, not auto-painting VGA
 * text RAM; (2) DMI/ECC/parity/beep are N/A; (3) do_tick()'s rdtsc elapsed-time
 * block becomes a PPC timebase read.
 */

#include "test.h"
#include "config.h"
/* x86: <sys/io.h> provides inb/outb x86 port-I/O — N/A on PPC/OF. error.c's
 * only port-I/O user was beep() (PC speaker), commented out below. Replaced by
 * ofw.h (the OF client interface = the io.h/BIOS replacement) which do_tick()'s
 * timebase divisor (ofw_get_timebase_freq) needs. */
/* #include <sys/io.h> */
#include "ofw.h"
/* x86: dmi.h declares the SMBIOS/DMI memory-device table (add_dmi_err,
 * dmi_err_cnts[], dmi_initialized) — N/A on PPC/OF (no SMBIOS). The DMI display
 * code below is commented out; this include is dropped. */
/* #include "dmi.h" */

/* PPC: ppc.h (mftb/mftbu) is pulled in transitively via test.h, used by
 * do_tick()'s PPC timebase elapsed-time read. */

extern int test_ticks, nticks, beepmode;
extern struct tseq tseq[];
/* x86: dmi_err_cnts[]/dmi_initialized come from dmi.c (SMBIOS per-slot error
 * counts) — N/A on PPC/OF. Commented with the DMI display code that used them. */
/* extern int dmi_err_cnts[MAX_DMI_MEMDEVS]; */
/* extern short dmi_initialized; */
void poll_errors();

static void update_err_counts(void);
static void print_err_counts(void);
static void common_err();
static int syn, chan, len=1;

/*
 * Display data error message. Don't display duplicate errors.
 */
void error(ulong *adr, ulong good, ulong bad)
{
	ulong xor;

	xor = good ^ bad;
#ifdef USB_WAR
	/* Skip any errrors that appear to be due to the BIOS using location
	 * 0x4e0 for USB keyboard support.  This often happens with Intel
         * 810, 815 and 820 chipsets.  It is possible that we will skip
	 * a real error but the odds are very low.
	 */
	/* x86: USB_WAR is #undef'd in config.h (BIOS USB legacy-keyboard low-memory
	 * corruption is N/A on PPC/OF; OF owns ADB/USB input), so this block compiles
	 * out. Left verbatim inside its dormant #ifdef. */
	if ((ulong)adr == 0x4e0 || (ulong)adr == 0x410) {
		return;
	}
#endif
	common_err(adr, good, bad, xor, 0);
}

/*
 * Display address error message.
 * Since this is strictly an address test, trying to create BadRAM
 * patterns does not make sense.  Just report the error.
 */
void ad_err1(ulong *adr1, ulong *mask, ulong bad, ulong good)
{
	common_err(adr1, good, bad, (ulong)mask, 1);
}

/*
 * Display address error message.
 * Since this type of address error can also report data errors go
 * ahead and generate BadRAM patterns.
 */
void ad_err2(ulong *adr, ulong bad)
{
	common_err(adr, (ulong)adr, bad, ((ulong)adr) ^ bad, 0);
}

static void update_err_counts(void)
{
	/* x86: beep() programs the PC speaker via PIT ports 0x43/0x61 — N/A on
	 * PPC/OF (no PC speaker). beepmode is forced to 0 via config.h's BEEP_MODE
	 * (init.c sets beepmode = BEEP_MODE), so this block is dead anyway; the
	 * beep() calls are commented so the (also-commented) PC-speaker beep() in
	 * test.c is never referenced from here. */
	if (beepmode){
		/* beep(600); */
		/* beep(1000); */
	}

	if (v->pass && v->ecount == 0) {
		cprint(LINE_MSG, COL_MSG,
			"                                            ");
	}
	++(v->ecount);
	tseq[v->test].errors++;

}

static void print_err_counts(void)
{
	int i;
	char *pp;

	if ((v->ecount > 4096) && (v->ecount % 256 != 0)) return;

	dprint(LINE_INFO, COL_ERR, v->ecount, 6, 0);
	dprint(LINE_INFO, COL_ECC_ERR, v->ecc_ecount, 6, 0);

	/* Paint the error messages on the screen red to provide a vivid */
	/* indicator that an error has occured */
	if ((v->printmode == PRINTMODE_ADDRESSES ||
			v->printmode == PRINTMODE_PATTERNS) &&
			v->msg_line < 24) {
		for(i=0, pp=(char *)((SCREEN_ADR+v->msg_line*160+1));
				 i<76; i++, pp+=2) {
			*pp = 0x47;
		}
		/* PPC: the 0x47 (white-on-red) attribute bytes above were poked
		 * straight into vga_buf's attribute halves WITHOUT changing the
		 * underlying char. On x86 the VGA text hardware re-paints from the
		 * attr byte automatically; our vga_buf is a shadow buffer and cprint
		 * routes through tty_print_line, which dedups on the char shadow
		 * (screen_buf) — so an attr-only change does NOT auto-render. Re-blit
		 * each of those cells explicitly so the red row actually appears. The
		 * loop mirrors the 0x47 fill above: row v->msg_line, columns 1..76.
		 * (See HANDOFF "Substrate contract"; init.c does the same for its
		 * green title bar / reverse footer.) */
		for (i = 1; i <= 76; i++) {
			fb_render_cell(v->msg_line, i);
		}
	}
}

/*
 * Print an individual error
 */
void common_err( ulong *adr, ulong good, ulong bad, ulong xor, int type)
{
	int i, n, x, j, flag=0;
	ulong page, offset;
	int patnchg;
	ulong mb;

	update_err_counts();
	/* x86: add_dmi_err() tallies the failing address against the SMBIOS DMI
	 * memory-device map (so the summary screen can attribute errors to a DIMM
	 * slot) — N/A on PPC/OF (no SMBIOS/DMI). The per-slot display below is
	 * commented out too, so there is nothing to feed. */
	/* add_dmi_err((ulong)adr); */

	switch(v->printmode) {
	case PRINTMODE_SUMMARY:
		/* Don't do anything for a parity error. */
		if (type == 3) {
			return;
		}

		/* Address error */
		if (type == 1) {
			xor = good ^ bad;
		}

		/* Ecc correctable errors */
		if (type == 2) {
			/* the bad value is the corrected flag */
			if (bad) {
				v->erri.cor_err++;
			}
			page = (ulong)adr;
			offset = good;
		} else {
			page = page_of(adr);
			offset = (ulong)adr & 0xFFF;
		}

		/* Calc upper and lower error addresses */
		if (v->erri.low_addr.page > page) {
			v->erri.low_addr.page = page;
			v->erri.low_addr.offset = offset;
			flag++;
		} else if (v->erri.low_addr.page == page &&
				v->erri.low_addr.offset > offset) {
			v->erri.low_addr.offset = offset;
			v->erri.high_addr.offset = offset;
			flag++;
		} else if (v->erri.high_addr.page < page) {
			v->erri.high_addr.page = page;
			flag++;
		}
		if (v->erri.high_addr.page == page &&
				v->erri.high_addr.offset < offset) {
			v->erri.high_addr.offset = offset;
			flag++;
		}

		/* Calc bits in error */
		for (i=0, n=0; i<32; i++) {
			if (xor>>i & 1) {
				n++;
			}
		}
		v->erri.tbits += n;
		if (n > v->erri.max_bits) {
			v->erri.max_bits = n;
			flag++;
		}
		if (n < v->erri.min_bits) {
			v->erri.min_bits = n;
			flag++;
		}
		if (v->erri.ebits ^ xor) {
			flag++;
		}
		v->erri.ebits |= xor;

	 	/* Calc max contig errors */
		len = 1;
		if ((ulong)adr == (ulong)v->erri.eadr+4 ||
				(ulong)adr == (ulong)v->erri.eadr-4 ) {
			len++;
		}
		if (len > v->erri.maxl) {
			v->erri.maxl = len;
			flag++;
		}
		v->erri.eadr = (ulong)adr;

		if (v->erri.hdr_flag == 0) {
			clear_scroll();
			cprint(LINE_HEADER+0, 1,  "Error Confidence Value:");
			cprint(LINE_HEADER+1, 1,  "  Lowest Error Address:");
			cprint(LINE_HEADER+2, 1,  " Highest Error Address:");
			cprint(LINE_HEADER+3, 1,  "    Bits in Error Mask:");
			cprint(LINE_HEADER+4, 1,  " Bits in Error - Total:");
			cprint(LINE_HEADER+4, 29,  "Min:    Max:    Avg:");
			cprint(LINE_HEADER+5, 1,  " Max Contiguous Errors:");
			cprint(LINE_HEADER+6, 1,  "ECC Correctable Errors:");
			cprint(LINE_HEADER+7, 1,  "Errors per Memory Slot:");
			/* x86: this block prints the per-slot ("Errors per Memory Slot")
			 * column headers from the SMBIOS DMI memory-device map — N/A on
			 * PPC/OF (no DMI; dmi_initialized/dmi_err_cnts[] are gone). The
			 * "Errors per Memory Slot:" label above is left verbatim (a static
			 * label costs nothing and keeps the summary layout identical); only
			 * the data population is commented out. The `x`/`j` it used are now
			 * unused in this branch (`n` is still consumed below). */
			/*
			x = 24;
			if (dmi_initialized) {
			  for ( i=0; i < MAX_DMI_MEMDEVS;){
			    n = LINE_HEADER+7;
			    for (j=0; j<4; j++) {
				if (dmi_err_cnts[i] >= 0) {
					dprint(n, x, i, 2, 0);
					cprint(n, x+2, ": 0");
				}
				i++;
				n++;
			    }
			    x += 10;
			  }
			}
			*/
			/* PPC: x/j were only used by the two commented-out DMI loops
			 * (the per-slot header here and the per-slot counts below);
			 * silence the now-unused-variable warning without removing the
			 * declarations (kept verbatim for fidelity). */
			(void)x; (void)j;

			cprint(LINE_HEADER+0, 64,   "Test  Errors");
			v->erri.hdr_flag++;
		}
		if (flag) {
		  /* Calc bits in error */
		  for (i=0, n=0; i<32; i++) {
			if (v->erri.ebits>>i & 1) {
				n++;
			}
		  }
		  page = v->erri.low_addr.page;
		  offset = v->erri.low_addr.offset;
		  mb = page >> 8;
		  hprint(LINE_HEADER+1, 25, page);
		  hprint2(LINE_HEADER+1, 33, offset, 3);
		  cprint(LINE_HEADER+1, 36, " -      . MB");
		  dprint(LINE_HEADER+1, 39, mb, 5, 0);
		  dprint(LINE_HEADER+1, 45, ((page & 0xF)*10)/16, 1, 0);
		  page = v->erri.high_addr.page;
		  offset = v->erri.high_addr.offset;
		  mb = page >> 8;
		  hprint(LINE_HEADER+2, 25, page);
		  hprint2(LINE_HEADER+2, 33, offset, 3);
		  cprint(LINE_HEADER+2, 36, " -      . MB");
		  dprint(LINE_HEADER+2, 39, mb, 5, 0);
		  dprint(LINE_HEADER+2, 45, ((page & 0xF)*10)/16, 1, 0);
		  hprint(LINE_HEADER+3, 25, v->erri.ebits);
		  dprint(LINE_HEADER+4, 25, n, 2, 1);
		  dprint(LINE_HEADER+4, 34, v->erri.min_bits, 2, 1);
		  dprint(LINE_HEADER+4, 42, v->erri.max_bits, 2, 1);
		  dprint(LINE_HEADER+4, 50, v->erri.tbits/v->ecount, 2, 1);
		  dprint(LINE_HEADER+5, 25, v->erri.maxl, 7, 1);
		  /* x86: per-slot error counts from the SMBIOS DMI map — N/A on
		   * PPC/OF (no DMI). Commented out with the header block above; the
		   * "Test Errors" per-test column below is platform-neutral and kept. */
		  /*
		  x = 28;
		  for ( i=0; i < MAX_DMI_MEMDEVS;){
		  	n = LINE_HEADER+7;
			for (j=0; j<4; j++) {
				if (dmi_err_cnts[i] > 0) {
					dprint (n, x, dmi_err_cnts[i], 7, 1);
				}
				i++;
				n++;
			}
			x += 10;
		  }
		  */

		  for (i=0; tseq[i].msg != NULL; i++) {
			dprint(LINE_HEADER+1+i, 66, i, 2, 0);
			dprint(LINE_HEADER+1+i, 68, tseq[i].errors, 8, 0);
	  	  }
		}
		if (v->erri.cor_err) {
		  dprint(LINE_HEADER+6, 25, v->erri.cor_err, 8, 1);
		}
		break;

	case PRINTMODE_ADDRESSES:
		/* Don't display duplicate errors */
		if ((ulong)adr == (ulong)v->erri.eadr &&
				 xor == v->erri.exor) {
			return;
		}
		if (v->erri.hdr_flag == 0) {
			clear_scroll();
			cprint(LINE_HEADER, 0,
"Tst  Pass   Failing Address          Good       Bad     Err-Bits  Count Chan");
			cprint(LINE_HEADER+1, 0,
"---  ----  -----------------------  --------  --------  --------  ----- ----");
			v->erri.hdr_flag++;
		}
		/* Check for keyboard input */
		check_input();
		scroll();

		if ( type == 2 || type == 3) {
			page = (ulong)adr;
			offset = good;
		} else {
			page = page_of(adr);
			offset = ((unsigned long)adr) & 0xFFF;
		}
		mb = page >> 8;
		dprint(v->msg_line, 0, v->test, 3, 0);
		dprint(v->msg_line, 4, v->pass, 5, 0);
		hprint(v->msg_line, 11, page);
		hprint2(v->msg_line, 19, offset, 3);
		cprint(v->msg_line, 22, " -      . MB");
		dprint(v->msg_line, 25, mb, 5, 0);
		dprint(v->msg_line, 31, ((page & 0xF)*10)/16, 1, 0);

		if (type == 3) {
			/* ECC error */
			cprint(v->msg_line, 36,
			  bad?"corrected           ": "uncorrected         ");
			hprint2(v->msg_line, 60, syn, 4);
			cprint(v->msg_line, 68, "ECC");
			dprint(v->msg_line, 74, chan, 2, 0);
		} else if (type == 2) {
			cprint(v->msg_line, 36, "Parity error detected                ");
		} else {
			hprint(v->msg_line, 36, good);
			hprint(v->msg_line, 46, bad);
			hprint(v->msg_line, 56, xor);
			dprint(v->msg_line, 66, v->ecount, 5, 0);
			v->erri.exor = xor;
		}
		v->erri.eadr = (ulong)adr;
		print_err_counts();
		break;

	case PRINTMODE_PATTERNS:
		if (v->erri.hdr_flag == 0) {
			clear_scroll();
			v->erri.hdr_flag++;
		}
		/* Do not do badram patterns from test 0 or 5 */
		if (v->test == 0 || v->test == 5) {
			return;
		}
		/* Only do patterns for data errors */
		if ( type != 0) {
			return;
		}
		/* Process the address in the pattern administration */
		patnchg=insertaddress ((ulong) adr);
		if (patnchg) {
			printpatn();
		}
		break;

	case PRINTMODE_NONE:
		if (v->erri.hdr_flag == 0) {
			clear_scroll();
			v->erri.hdr_flag++;
		}
		break;
	}
}

/*
 * Print an ecc error
 */
/* x86: print_ecc_err() reports an ECC syndrome/channel from the x86
 * memory-controller poll (poll_errors() in controller.c) — N/A on PPC/OF: we
 * have no ECC-controller register access and controller.c is a Wave-6 ⛔ file.
 * Its only caller is poll_errors(); do_tick()'s poll_errors() call is commented
 * out below. Body wrapped in #if 0 (kept verbatim for fidelity); the (also
 * commented) common_err type==2 ECC branch above stays so the table layout is
 * intact if ECC is ever wired up. */
#if 0
void print_ecc_err(unsigned long page, unsigned long offset,
	int corrected, unsigned short syndrome, int channel)
{
	++(v->ecc_ecount);
	syn = syndrome;
	chan = channel;
	common_err((ulong *)page, offset, corrected, 0, 2);
}
#endif

#ifdef PARITY_MEM
/*
 * Print a parity error message
 */
/* x86: parity_err() is invoked from the x86 NMI parity-error path (inter() in
 * lib.c) — N/A on PPC/OF (no x86 parity NMI bus). PARITY_MEM is #undef'd in
 * config.h, so this block compiles out; left verbatim inside its dormant
 * #ifdef per the discipline. */
void parity_err( unsigned long edi, unsigned long esi)
{
	unsigned long addr;

	if (v->test == 5) {
		addr = esi;
	} else {
		addr = edi;
	}
	common_err((ulong *)addr, addr & 0xFFF, 0, 0, 3);
}
#endif

/*
 * Print the pattern array as a LILO boot option addressing BadRAM support.
 */
void printpatn (void)
{
       int idx=0;
       int x;

	/* Check for keyboard input */
	check_input();

       if (v->numpatn == 0)
               return;

       scroll();

       cprint (v->msg_line, 0, "badram=");
       x=7;

       for (idx = 0; idx < v->numpatn; idx++) {

               if (x > 80-22) {
                       scroll();
                       x=7;
               }
               cprint (v->msg_line, x, "0x");
               hprint (v->msg_line, x+2,  v->patn[idx].adr );
               cprint (v->msg_line, x+10, ",0x");
               hprint (v->msg_line, x+13, v->patn[idx].mask);
               if (idx+1 < v->numpatn)
                       cprint (v->msg_line, x+21, ",");
               x+=22;
       }
}

/*
 * Show progress by displaying elapsed time and update bar graphs
 */
void do_tick(void)
{
	int i, n, pct;
	ulong h, l, t;
	/* PPC: timebase elapsed-time state (replaces the x86 rdtsc start ref that
	 * upstream kept in v->startl/v->starth + v->clks_msec). tb_freq caches
	 * ofw_get_timebase_freq() (ticks/sec); start_{h,l} latch the 64-bit
	 * timebase on the first tick so elapsed time runs from the first tick
	 * onward. The brief permits a static start reference here. */
	static unsigned long tb_freq = 0;
	static unsigned long start_h, start_l;
	static int tb_init = 0;
	unsigned long now_h, now_l, now_h2, elapsed;

	/* FIXME only print serial error messages from the tick handler */
	if (v->ecount) {
		print_err_counts();
	}

	nticks++;
	v->total_ticks++;

	pct = 100*nticks/test_ticks;
	dprint(1, COL_MID+4, pct, 3, 0);
	i = (BAR_SIZE * pct) / 100;
	while (i > v->tptr) {
		if (v->tptr >= BAR_SIZE) {
			break;
		}
		cprint(1, COL_MID+9+v->tptr, "#");
		v->tptr++;
	}

	pct = 100*v->total_ticks/v->pass_ticks;
	dprint(0, COL_MID+4, pct, 3, 0);
	i = (BAR_SIZE * pct) / 100;
	while (i > v->pptr) {
		if (v->pptr >= BAR_SIZE) {
			break;
		}
		cprint(0, COL_MID+9+v->pptr, "#");
		v->pptr++;
	}

	if (v->ecount && v->printmode == PRINTMODE_SUMMARY) {
		/* Compute confidence score */
		pct = 0;

		/* If there are no errors within 1mb of start - end addresses */
		h = v->pmap[v->msegs - 1].end - 0x100;
		if (v->erri.low_addr.page >  0x100 &&
				 v->erri.high_addr.page < h) {
			pct += 8;
		}

		/* Errors for only some tests */
		if (v->pass) {
			for (i=0, n=0; tseq[i].msg != NULL; i++) {
				if (tseq[i].errors == 0) {
					n++;
				}
			}
			pct += n*3;
		} else {
			for (i=0, n=0; i<v->test; i++) {
				if (tseq[i].errors == 0) {
					n++;
				}
			}
			pct += n*2;

		}

		/* Only some bits in error */
		n = 0;
		if (v->erri.ebits & 0xf) n++;
		if (v->erri.ebits & 0xf0) n++;
		if (v->erri.ebits & 0xf00) n++;
		if (v->erri.ebits & 0xf000) n++;
		if (v->erri.ebits & 0xf0000) n++;
		if (v->erri.ebits & 0xf00000) n++;
		if (v->erri.ebits & 0xf000000) n++;
		if (v->erri.ebits & 0xf0000000) n++;
		pct += (8-n)*2;

		/* Adjust the score */
		pct = pct*100/22;
/*
		if (pct > 100) {
			pct = 100;
		}
*/
		dprint(LINE_HEADER+0, 25, pct, 3, 1);
	}


	/* x86: the original read the elapsed time from the Time Stamp Counter via
	 * rdtsc, subtracted the start TSC (v->startl/v->starth), and divided by
	 * v->clks_msec (CPU MHz, measured in init.c's cpuspeed()), gated on
	 * v->rdtsc (set when CPUID reports a TSC) — N/A on PPC/OF: there is no
	 * rdtsc, and we identify the CPU via PVR, not CPUID. Original preserved:
	 *
	 *	if (v->rdtsc) {
	 *		asm __volatile__(
	 *			"rdtsc":"=a" (l),"=d" (h));
	 *		asm __volatile__ (
	 *			"subl %2,%0\n\t"
	 *			"sbbl %3,%1"
	 *			:"=a" (l), "=d" (h)
	 *			:"g" (v->startl), "g" (v->starth),
	 *			"0" (l), "1" (h));
	 *		t = h * ((unsigned)0xffffffff / v->clks_msec) / 1000;
	 *		t += (l / v->clks_msec) / 1000;
	 *		i = t % 60;
	 *		dprint(LINE_TIME, COL_TIME+9, i%10, 1, 0);
	 *		dprint(LINE_TIME, COL_TIME+8, i/10, 1, 0);
	 *		t /= 60;
	 *		i = t % 60;
	 *		dprint(LINE_TIME, COL_TIME+6, i % 10, 1, 0);
	 *		dprint(LINE_TIME, COL_TIME+5, i / 10, 1, 0);
	 *		t /= 60;
	 *		dprint(LINE_TIME, COL_TIME, t, 4, 0);
	 *	}
	 */
	/* PPC: read the 64-bit Time Base (mftbu:mftb, with the standard re-read of
	 * the upper half to guard against a low-half carry between reads), latch
	 * the start reference on the first tick, and divide elapsed ticks by the
	 * OF-reported timebase frequency to get seconds. The HH:MM:SS digits go to
	 * the exact same LINE_TIME / COL_TIME positions as the x86 path above, so
	 * the WallTime field renders identically. */
	if (tb_freq == 0 && !tb_init) {
		tb_freq = ofw_get_timebase_freq();
		do {
			start_h = mftbu();
			start_l = mftb();
			now_h2 = mftbu();
		} while (start_h != now_h2);
		tb_init = 1;
	}
	if (tb_freq) {
		do {
			now_h = mftbu();
			now_l = mftb();
			now_h2 = mftbu();
		} while (now_h != now_h2);
		/* 64-bit subtraction (now - start) via the two 32-bit halves. */
		l = now_l - start_l;
		h = now_h - start_h - (now_l < start_l ? 1 : 0);
		/* Elapsed seconds = (h<<32 | l) / tb_freq. Done in 64-bit. */
		elapsed = (unsigned long)(((((unsigned long long)h) << 32)
				| (unsigned long long)l) / (unsigned long long)tb_freq);
		t = elapsed;
		i = t % 60;
		dprint(LINE_TIME, COL_TIME+9, i%10, 1, 0);
		dprint(LINE_TIME, COL_TIME+8, i/10, 1, 0);
		t /= 60;
		i = t % 60;
		dprint(LINE_TIME, COL_TIME+6, i % 10, 1, 0);
		dprint(LINE_TIME, COL_TIME+5, i / 10, 1, 0);
		t /= 60;
		dprint(LINE_TIME, COL_TIME, t, 4, 0);
	}


	/* Check for keyboard input */
	check_input();

	/* x86: poll_errors() reads the x86 memory-controller's ECC error
	 * registers (controller.c, a Wave-6 ⛔ file) — N/A on PPC/OF (no such
	 * controller register access). print_ecc_err() (its consumer) is #if 0'd
	 * above. Commented out so do_tick() links without controller.c. */
	/* poll_errors(); */
}
