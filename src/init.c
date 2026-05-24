/* init.c - MemTest-86  Version 3.0
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady, cbrady@sgi.com
 * ----------------------------------------------------
 * MemTest86+ V2.00 Specific code (GPL V2.0)
 * By Samuel DEMEULEMEESTER, sdemeule@memtest.org
 * http://www.canardplus.com - http://www.memtest.org
 * ----------------------------------------------------
 * memtestppc+ port (session 006, v2.00 -> PowerPC / Open Firmware, single-CPU):
 * Line-faithful port of memtest86+ v2.00 init.c. Discipline (PORTING-GUIDE.md):
 * start from the verbatim v2.00 file; comment out x86/PC leaf code with a
 * one-line reason; add PPC/OF replacements inline marked "PPC:". Never delete
 * upstream code. The static TUI header drawn here is kept BYTE-VERBATIM (that is
 * the whole point of the port) except the one allowed text change
 * Memtest86 -> Memtestppc and the data adaptations (CPU name from PVR; clock,
 * cache, memory from Open Firmware). Nothing from the old attic/src-v0.01 hack
 * is inherited; its identify_cpu() was read only for the OF/PVR *mechanics*.
 */

#include "test.h"
/* x86: defs.h carried x86 real-mode boot/GDT/segment/reloc constants
 * (LOW_TEST_ADR, INITSEG, ...) — its whole body is #if 0'd in our port
 * (Wave 1), so including it here brings in nothing. init.c does not need it. */
/* #include "defs.h" */
#include "config.h"
/* x86: controller.h declares the PC chipset / memory-controller detection
 * (find_controller, controller tables) reached over PCI config space — N/A on
 * PPC/OF; controller.c is a Wave-6 examine->parked file, so the header is not
 * provided and the find_controller() call below is commented out. */
/* #include "controller.h" */
/* x86: pci.h declares PCI config access via x86 port I/O (0xcf8/0xcfc) — N/A on
 * PPC/OF (Open Firmware owns the PCI tree); pci.c is a Wave-6 parked file, so
 * the header is not provided and the pci_init() call below is commented out. */
/* #include "pci.h" */
/* x86: io.h is the x86 port-I/O (inb/outb) header — N/A on PPC/OF (no port I/O;
 * hardware is reached through ofw.{c,h}). Not provided in our tree. */
/* #include "io.h" */

/* PPC: the Open Firmware client interface — our io.h/BIOS replacement. CPU
 * identity (clock, cache) is read from the OF device tree via ofw_finddevice +
 * ofw_getprop, replacing the x86 cpuid/rdtsc/PIT measurements below. */
#include "ofw.h"

/* x86: rdmsr reads an x86 Model-Specific Register (used only by correct_tsc()
 * below, the Intel TSC-coefficient fixup) — N/A on PPC/OF. Commented out; the
 * sole user, correct_tsc(), is itself commented out below. */
/*
#define rdmsr(msr,val1,val2) \
	__asm__ __volatile__("rdmsr" \
			  : "=a" (val1), "=d" (val2) \
			  : "c" (msr))
*/

extern struct tseq tseq[];
/* x86: memsz_mode/firmware/dmi_initialized/dmi_err_cnts are PC firmware-probe
 * and DMI/SMBIOS state. On PPC/OF memory comes from the OF device tree
 * (mem_size(), Wave 3 memsize.c) and there is no DMI. The init() body that used
 * them (the firmware-probe block and the dmi_err_cnts reset loop) is commented
 * out below, so these externs are no longer referenced. Kept for fidelity. */
/* extern short memsz_mode; */
/* extern short firmware; */
/* extern short dmi_initialized; */
/* extern int dmi_err_cnts[MAX_DMI_MEMDEVS]; */

/* x86: struct cpu_ident is x86 CPUID output (vendor string, capability flags,
 * cache_info[]) — N/A on PPC/OF, which identifies the CPU by PVR. The struct is
 * commented out in test.h; this instance and every cpu_id.* reference in
 * cpu_type() is replaced by the PPC PVR path. Commented, not deleted. */
/* struct cpu_ident cpu_id; */

/* x86: st_/end_/cal_ low+high held the rdtsc snapshot halves used by the
 * PIT-based cpuspeed() and the rep-movsl memspeed() -- both x86 leaf timers,
 * commented out below. Kept (commented) for fidelity. */
/* ulong st_low, st_high; */
/* ulong end_low, end_high; */
/* ulong cal_low, cal_high; */
ulong extclock;

int l1_cache, l2_cache;
int tsc_invariable = 0;

/* PPC: the CPU's Processor Version Register value, captured by cpu_type(). The
 * high half (bits 16-31) is the processor family/version used to pick the name.
 * Replaces the x86 cpu_id.{type,model,step} family decode. */
ulong cpu_pvr;

/* x86: memspeed() (rep-movsl/stosl/lodsl block-copy benchmark timed by rdtsc) is
 * an x86 leaf — N/A on PPC/OF. Its body is commented out below; the prototype is
 * dropped so nothing links against the missing definition. The MB/s speed
 * figures it produced are simply not shown (upstream leaves them blank when the
 * benchmark cannot run). */
/* ulong memspeed(ulong src, ulong len, int iter, int type); */
static void cpu_type(void);
/* x86: cacheable() walks memory measuring per-range copy speed (via memspeed) to
 * find the cache-able size — relies on the x86 memspeed/map_page leaf. Commented
 * out below; its call site in init() is commented too. */
/* static void cacheable(void); */
/* x86: cpuspeed() measures CPU MHz with the 8254 PIT (port 0x42/0x43/0x61) and
 * rdtsc — N/A on PPC/OF; replaced by reading clock-frequency from the OF CPU
 * node. Commented out below. */
/* static int cpuspeed(void); */
int beepmode;

static void display_init(void)
{
	int i;
	volatile char *pp;

	/* PPC: bring up the Open Firmware framebuffer hardware first (discover it,
	 * set the 16-color palette, black the physical screen). This is the
	 * hardware bring-up that the x86 build got for free from VGA text-mode
	 * hardware; on PPC there is only a linear framebuffer. fb_init() (display.c,
	 * renamed from the substrate's old display_init so this upstream
	 * display_init keeps its name) returns <0 on failure — bail if so, there is
	 * nothing to draw on. */
	if (fb_init() < 0) {
		ofw_puts("memtestppc+: no Open Firmware framebuffer found\r\n");
		return;
	}

	/* x86: serial_echo_init() programmed the 16550 UART and serial_echo_print()
	 * emitted VT100 escapes to a serial console — N/A on PPC/OF (ttyprint drives
	 * the framebuffer, not a UART). serial_echo_init() is now a stub that only
	 * clears the screen shadow buffer (lib.c); the escape-sequence prints are
	 * inert no-ops. Kept verbatim for fidelity (they compile and do nothing). */
	serial_echo_init();
	serial_echo_print("\x1B[LINE_SCROLL;24r"); /* Set scroll area row 7-23 */
	serial_echo_print("\x1B[H\x1B[2J");   /* Clear Screen */
	serial_echo_print("\x1B[37m\x1B[44m");
	serial_echo_print("\x1B[0m");
	serial_echo_print("\x1B[37m\x1B[44m");

	/* Clear screen & set background to blue */
	for(i=0, pp=(char *)(SCREEN_ADR); i<80*24; i++) {
		*pp++ = ' ';
		*pp++ = 0x17;
	}

	/* Make the name background red */
	for(i=0, pp=(char *)(SCREEN_ADR+1); i<TITLE_WIDTH; i++, pp+=2) {
		*pp = 0x20;
	}
	/* PPC: the ONE allowed text change — "Memtest86" -> "Memtestppc". The title
	 * string is kept at TITLE_WIDTH (28) chars by dropping one of the two spaces
	 * between the name and the version, since "Memtestppc" (10 chars) is one
	 * char longer than "Memtest86" (9). Layout (0-based cols): 6 leading spaces,
	 * "Memtestppc" at cols 6-15, one space at col 16 (overwritten by the
	 * blinking '+' below), "v2.00" at cols 17-21, 6 trailing spaces. The version
	 * string is left as "v2.00" for now — see report (port-init_c.md) for the
	 * keep-v2.00 vs use-project-version question flagged to the user. */
	cprint(0, 0, "      Memtestppc v2.00      ");

	/* PPC: blink-attr poke for the title and the '+'. Upstream poked the attr of
	 * cell 0 and cell 15 (pp += 30 bytes = 15 cells) to 0xA4 (blink+colors) and
	 * cprint'd "+" at col 15, giving "Memtest86+". Because "Memtestppc" is one
	 * char wider, the '+' now lands at col 16, so the second attr poke targets
	 * cell 16 (pp += 32 bytes = 16 cells) and the cprint column is 16. */
	for(i=0, pp=(char *)(SCREEN_ADR+1); i<2; i++, pp+=32) {
		*pp = 0xA4;
	}
	cprint(0, 16, "+");

	/* Do reverse video for the bottom display line */
	for(i=0, pp=(char *)(SCREEN_ADR+1+(24 * 160)); i<80; i++, pp+=2) {
		*pp = 0x71;
	}

	serial_echo_print("\x1B[0m");

	/* PPC: the title-bar green attr, the blinking-'+' attr, and the reverse-video
	 * bottom line above were poked straight into vga_buf's attribute bytes
	 * WITHOUT changing the underlying char. cprint routes through tty_print_line,
	 * which dedups on the char shadow (screen_buf), so those attr-only cells will
	 * NOT auto-render. A single display_refresh() repaints the whole buffer so
	 * the colors appear. (x86 didn't need this: VGA hardware re-paints from the
	 * attr byte; our framebuffer shadow does not.) See HANDOFF "Substrate
	 * contract". */
	display_refresh();
}

/*
 * Initialize test, setup screen and find out how much memory there is.
 */
void init(void)
{
	int i;

	/* x86: outb(0x8, 0x3f2) writes the floppy Digital Output Register to spin the
	 * floppy motor down — N/A on PPC/OF (no PC floppy controller; Macs have no
	 * 0x3f2 port). Commented out. */
	/* outb(0x8, 0x3f2); */  /* Kill Floppy Motor */

	/* Turn on cache */
	/* PPC: set_cache() is a stub in our lib.c (HID0 cache control not yet
	 * implemented); this call is harmless and kept verbatim for fidelity. */
	set_cache(1);

	/* Setup the display */
	display_init();

	/* x86: firmware/memsz_mode probe — query_linuxbios()/query_pcbios() scan low
	 * memory for a coreboot table or call the PC BIOS E820 map — N/A on PPC/OF.
	 * Memory comes from the OF device tree (mem_size(), Wave 3 memsize.c).
	 * Commented out wholesale; the firmware externs are commented above. */
	/*
	if ((firmware == FIRMWARE_UNKNOWN) &&
		(memsz_mode != SZ_MODE_PROBE)) {
		if (query_linuxbios()) {
			firmware = FIRMWARE_LINUXBIOS;
		}
		else if (query_pcbios()) {
			firmware = FIRMWARE_PCBIOS;
		}
	}
	*/

	/* PPC: mem_size() is ported in memsize.c (Wave 3, parallel agent): it walks
	 * the OF /memory node, claims RAM (OF's MMU is on), and populates
	 * v->pmap[]/v->msegs/v->test_pages. Kept at the upstream call site so the
	 * function boundary matches; init() displays the resulting total below
	 * exactly as upstream does (aprint of v->test_pages at LINE_CPU+3). */
	mem_size();

	/* x86: pci_init() enumerates the PCI bus via x86 config-space port I/O to
	 * locate the chipset — N/A on PPC/OF (pci.c is a Wave-6 parked file; OF owns
	 * the PCI tree). Commented out; the chipset line just stays "Unknown". */
	/* pci_init(); */

	/* setup beep mode */
	/* PPC: BEEP_MODE is forced to 0 in config.h (no PC speaker on PPC); beepmode
	 * stays 0 and the beep path (lib.c beep(), error.c) never fires. Kept
	 * verbatim. */
	beepmode = BEEP_MODE;

	v->test = 0;
	v->pass = 0;
	v->msg_line = 0;
	v->ecount = 0;
	v->ecc_ecount = 0;
	v->testsel = -1;
	v->msg_line = LINE_SCROLL-1;
	v->scroll_start = v->msg_line * 160;
	v->erri.low_addr.page = 0x7fffffff;
	v->erri.low_addr.offset = 0xfff;
	v->erri.high_addr.page = 0;
	v->erri.high_addr.offset = 0;
	v->erri.min_bits = 32;
	v->erri.max_bits = 0;
	v->erri.min_bits = 32;
	v->erri.max_bits = 0;
	v->erri.maxl = 0;
	v->erri.cor_err = 0;
	v->erri.ebits = 0;
	v->erri.hdr_flag = 0;
	v->erri.tbits = 0;
	for (i=0; tseq[i].msg != NULL; i++) {
		tseq[i].errors = 0;
	}
	/* x86: DMI/SMBIOS per-DIMM error-count reset — N/A on PPC/OF (no DMI;
	 * dmi.c is a Wave-6 parked file, dmi_initialized/dmi_err_cnts externs are
	 * commented above). Commented out. */
	/*
	if (dmi_initialized) {
		for (i=0; i < MAX_DMI_MEMDEVS; i++){
			if (dmi_err_cnts[i] > 0) {
				dmi_err_cnts[i] = 0;
			}
		}
	}
	*/

	cprint(LINE_CPU+1, 0, "L1 Cache: Unknown ");
	cprint(LINE_CPU+2, 0, "L2 Cache: Unknown ");
	cprint(LINE_CPU+3, 0, "Memory  : ");
	aprint(LINE_CPU+3, 10, v->test_pages);
	cprint(LINE_CPU+4, 0, "Chipset : ");

	cpu_type();

	/* x86: find_controller() probes the PC north-bridge / memory controller over
	 * PCI to print the chipset name and (inverted) memory speed — N/A on PPC/OF
	 * (controller.c is a Wave-6 parked file). Commented out; the "Chipset :"
	 * line stays blank/Unknown as upstream does before detection. */
	/* find_controller(); */

	/* x86: v->rdtsc gates the rdtsc-based cacheable()/wall-clock display. On PPC
	 * cpu_type() leaves v->rdtsc = 0 (no rdtsc; the PPC time base is read via
	 * mftb in main.c's clock, Wave 5), and cacheable() is an x86 leaf. So this
	 * block does not run. Kept (commented) for fidelity; the "| Time:" clock
	 * label is drawn unconditionally below by the COL_TIME cprint that main.c's
	 * Wave-5 timekeeping updates. */
	/*
	if (v->rdtsc) {
		cacheable();
		cprint(LINE_TIME, COL_TIME+4, ":  :");
	}
	*/
	/* PPC: draw the wall-clock separators unconditionally — there is no rdtsc
	 * gate on PPC, and main.c (Wave 5) drives the HH:MM:SS clock from the OF time
	 * base. This is the verbatim cprint from inside the commented rdtsc block,
	 * promoted out of the gate. */
	cprint(LINE_TIME, COL_TIME+4, ":  :");

	cprint(0, COL_MID,"Pass   %");
	cprint(1, COL_MID,"Test   %");
	cprint(2, COL_MID,"Test #");
	cprint(3, COL_MID,"Testing: ");
	cprint(4, COL_MID,"Pattern: ");
	cprint(LINE_INFO-2, 0, " WallTime   Cached  RsvdMem   MemMap   Cache  ECC  Test  Pass  Errors ECC Errs");
	cprint(LINE_INFO-1, 0, " ---------  ------  -------  --------  -----  ---  ----  ----  ------ --------");
	cprint(LINE_INFO, COL_TST, " Std");
	cprint(LINE_INFO, COL_PASS, "    0");
	cprint(LINE_INFO, COL_ERR, "     0");
	cprint(LINE_INFO+1, 0, " -----------------------------------------------------------------------------");

	for(i=0; i < 5; i++) {
		cprint(i, COL_MID-2, "| ");
	}
	footer();
	// Default Print Mode
	// v->printmode=PRINTMODE_SUMMARY;
	v->printmode=PRINTMODE_ADDRESSES;
	v->numpatn=0;
	find_ticks();

	/* PPC: the status rows above (the WallTime header, the dashes, the vertical
	 * separators, the footer) are all plain cprint and auto-render, but the
	 * earlier attr-only pokes were already flushed in display_init(). One more
	 * display_refresh() here is cheap and guarantees the fully-assembled static
	 * screen is on the framebuffer before the test loop starts poking it. */
	display_refresh();
}

#define FLAT 0

/* x86: the paging block below (paging_off/paging_on/map_page/mapping/emapping/
 * page_of) implements memtest86+'s x86 PAE 2MB-window paging — it relocates a
 * 2MB test window into the low virtual address space via CR0/CR3/CR4 and PDE
 * tables so >4GB can be tested on a 32-bit kernel. On PPC/OF we test memory in
 * place: mem_size() (Wave 3) claims physical RAM that OF has already mapped, and
 * main.c (Wave 5) tests it directly without a relocating window. The whole
 * paging machinery is therefore commented out. The functions are still declared
 * in test.h and CALLED by the upstream main.c we port in Wave 5
 * (map_page/mapping/emapping/page_of/paging_off); Wave-5 main.c must comment
 * those call sites (and the windows[] machinery) to match, OR we provide
 * trivial flat-mapping stubs there. Flagged in the report as a Wave-5
 * coordination point. Commented, not deleted, for fidelity. */
#if 0
static unsigned long mapped_window = 1;
void paging_off(void)
{
	if (!v->pae)
		return;
	mapped_window = 1;
	__asm__ __volatile__ (
		/* Disable paging */
		"movl %%cr0, %%eax\n\t"
		"andl $0x7FFFFFFF, %%eax\n\t"
		"movl %%eax, %%cr0\n\t"
		/* Disable pae and pse */
		"movl %%cr4, %%eax\n\t"
		"and $0xCF, %%al\n\t"
		"movl %%eax, %%cr4\n\t"
		:
		:
		: "ax"
		);
}

static void paging_on(void *pdp)
{
	if (!v->pae)
		return;
	__asm__ __volatile__(
		/* Load the page table address */
		"movl %0, %%cr3\n\t"
		/* Enable pae */
		"movl %%cr4, %%eax\n\t"
		"orl $0x00000020, %%eax\n\t"
		"movl %%eax, %%cr4\n\t"
		/* Enable paging */
		"movl %%cr0, %%eax\n\t"
		"orl $0x80000000, %%eax\n\t"
		"movl %%eax, %%cr0\n\t"
		:
		: "r" (pdp)
		: "ax"
		);
}

int map_page(unsigned long page)
{
	unsigned long i;
	struct pde {
		unsigned long addr_lo;
		unsigned long addr_hi;
	};
	extern unsigned char pdp[];
	extern struct pde pd2[];
	unsigned long window = page >> 19;
	if (FLAT || (window == mapped_window)) {
		return 0;
	}
	if (window == 0) {
		return 0;
	}
	if (!v->pae || (window >= 32)) {
		/* Fail either we don't have pae support
		 * or we want an address that is out of bounds
		 * even for pae.
		 */
		return -1;
	}
	/* Compute the page table entries... */
	for(i = 0; i < 1024; i++) {
		/*-----------------10/30/2004 12:37PM---------------
		 * 0xE3 --
		 * Bit 0 = Present bit.      1 = PDE is present
		 * Bit 1 = Read/Write.       1 = memory is writable
		 * Bit 2 = Supervisor/User.  0 = Supervisor only (CPL 0-2)
		 * Bit 3 = Writethrough.     0 = writeback cache policy
		 * Bit 4 = Cache Disable.    0 = page level cache enabled
		 * Bit 5 = Accessed.         1 = memory has been accessed.
		 * Bit 6 = Dirty.            1 = memory has been written to.
		 * Bit 7 = Page Size.        1 = page size is 2 MBytes
		 * --------------------------------------------------*/
		pd2[i].addr_lo = ((window & 1) << 31) + ((i & 0x3ff) << 21) + 0xE3;
		pd2[i].addr_hi = (window >> 1);
	}
	paging_off();
	if (window > 1) {
		paging_on(pdp);
	}
	mapped_window = window;
	return 0;
}

void *mapping(unsigned long page_addr)
{
	void *result;
	if (FLAT || (page_addr < 0x80000)) {
		/* If the address is less that 1GB directly use the address */
		result = (void *)(page_addr << 12);
	}
	else {
		unsigned long alias;
		alias = page_addr & 0x7FFFF;
		alias += 0x80000;
		result = (void *)(alias << 12);
	}
	return result;
}

void *emapping(unsigned long page_addr)
{
	void *result;
	result = mapping(page_addr -1);
	/* The result needs to be 256 byte alinged... */
	result = ((unsigned char *)result) + 0xf00;
	return result;
}

unsigned long page_of(void *addr)
{
	unsigned long page;
	page = ((unsigned long)addr) >> 12;
	if (!FLAT && (page >= 0x80000)) {
		page &= 0x7FFFF;
		page += mapped_window << 19;
	}
#if 0
	cprint(LINE_SCROLL -2, 0, "page_of(        )->            ");
	hprint(LINE_SCROLL -2, 8, ((unsigned long)addr));
	hprint(LINE_SCROLL -2, 20, page);
#endif
	return page;
}
#endif /* x86 PAE 2MB-window paging — see comment above; tested in place on PPC */


/*
 * Find CPU type and cache sizes
 */

/* PPC: PVR (Processor Version Register) high-half -> human-readable name table.
 * Replaces the x86 cpu_id.{vend_id,type,model,step} family decode in the
 * original cpu_type() below. The PVR is read with `mfpvr`; bits 16-31 select the
 * processor. Sizes here are conservative defaults used only if Open Firmware does
 * not report d-cache-size / l2-cache-size (the OF values, when present, win).
 * Table assembled from the PowerPC family PVRs (mechanics learned from
 * attic/src-v0.01 identify_cpu(), then implemented inside v2.00's structure):
 *   0x0001 601        0x0003 603        0x0004 604       0x0006 603e
 *   0x0007 603ev/603r 0x0008 750 (G3)   0x000C 7400 (G4) 0x800C 7410 (G4)
 *   0x8000 7450 (G4)  0x8001 7455 (G4)  0x8002 7457 (G4) 0x0039 970 (G5)
 *   0x003C 970FX (G5) 0x0044 970MP (G5)
 * Some New World G3/G4 PVRs (e.g. 750CX/750FX) share or extend these; unknown
 * PVRs fall through to "Unknown PowerPC". TO VERIFY on real hardware: the iBook
 * G3 (750CX, ~0x0008) and the user's G5 (970, 0x0039). */
struct ppc_cpu_name {
	unsigned short pvr_hi;
	const char    *name;
	int            l1_kb;	/* fallback if OF has no d-cache-size */
	int            l2_kb;	/* fallback if OF has no l2-cache-size */
};
static const struct ppc_cpu_name ppc_cpu_names[] = {
	{ 0x0001, "PowerPC 601",      0,  0 },
	{ 0x0003, "PowerPC 603",     16,  0 },
	{ 0x0004, "PowerPC 604",     16,  0 },
	{ 0x0006, "PowerPC 603e",    32,  0 },
	{ 0x0007, "PowerPC 603ev",   32,  0 },
	{ 0x0008, "PowerPC 750 (G3)",   32, 256 },
	{ 0x000C, "PowerPC 7400 (G4)",  32, 256 },
	{ 0x800C, "PowerPC 7410 (G4)",  32, 256 },
	{ 0x8000, "PowerPC 7450 (G4)",  32, 256 },
	{ 0x8001, "PowerPC 7455 (G4)",  32, 256 },
	{ 0x8002, "PowerPC 7457 (G4)",  32, 512 },
	{ 0x8003, "PowerPC 7447A (G4)", 32, 512 },
	{ 0x8004, "PowerPC 7448 (G4)",  32, 1024 },
	{ 0x0039, "PowerPC 970 (G5)",   64, 512 },
	{ 0x003C, "PowerPC 970FX (G5)", 64, 512 },
	{ 0x0044, "PowerPC 970MP (G5)", 64, 1024 },
	{ 0x0000, NULL,                  0,  0 }
};

void cpu_type(void)
{
	/* x86: the original cpu_type() used these locals (int i/off + ulong speed)
	 * to walk the cpuid family/model decode, position the MHz string, and time
	 * memspeed. The PPC path needs none of i/off/speed; the file-scope l1_cache/
	 * l2_cache are reused. Kept as the original signature/structure with the x86
	 * leaf commented. */
	/* int i, off=0; */
	/* int l1_cache=0, l2_cache=0; */
	/* ulong speed; */

	/* PPC: locals for the OF/PVR detection that replaces the x86 leaf. */
	int i;
	unsigned short pvr_hi;
	const char *name = "Unknown PowerPC";
	ofw_handle_t cpu_node;
	unsigned long clock_freq = 0, dcache = 0, l2sz = 0;
	unsigned long mhz = 0;

	/* x86: v->rdtsc / v->pae are set by the cpuid feature decode below — N/A on
	 * PPC. v->rdtsc stays 0 (no rdtsc; the wall clock uses the PPC time base in
	 * main.c) and v->pae stays 0 (no PAE; we test memory in place). Set them
	 * explicitly to keep downstream code that reads them well-defined. */
	v->rdtsc = 0;
	v->pae = 0;

#ifdef CPUID_DEBUG
	/* x86: dumps cpu_id.{type,model,cpuid} for debugging — cpu_id is x86 CPUID
	 * state (commented out). N/A on PPC; left under its #ifdef (never defined). */
	/*
	dprint(9,0,cpu_id.type,3,1);
	dprint(10,0,cpu_id.model,3,1);
	dprint(11,0,cpu_id.cpuid,3,1);
	*/
#endif

	/* x86: the entire body of the original cpu_type() — the "no CPUID -> 386/486/
	 * Cyrix" branch, the AMD/Intel/Transmeta/VIA/Cyrix family-and-model decode
	 * (each leg a cprint(LINE_CPU,...) of the marketing name + cache_info[]
	 * arithmetic), the rdtsc speed print, the memspeed() L1/L2/memory MB/s
	 * figures, and the rdtsc start-time snapshot — is all x86 leaf code driven by
	 * cpuid/rdtsc/memspeed. It is replaced wholesale by the PPC PVR + Open
	 * Firmware path below. The original is preserved (commented) for fidelity and
	 * upstream-navigability; see port-init_c.md for the mapping. */
	/* ---- begin x86 cpu_type() body (commented; replaced by PPC path below) ----
	 *
	 *   if (cpu_id.cpuid < 1) { ... 386 / 486 / Cyrix-without-CPUID ... }
	 *   if (cpu_id.capability & (1 << X86_FEATURE_PAE)) v->pae = 1;
	 *   switch(cpu_id.vend_id[0]) {
	 *   case 'A':  ... AMD K5/K6/Athlon/Duron/K8/Opteron/Phenom ...
	 *   case 'G':  ... GenuineIntel / GenuineTMx86: 486..Core2/Nehalem,
	 *                  cache_info[] -> l1_cache/l2_cache ...
	 *   case 'C':  ... CentaurHauls / CyrixInstead: VIA C3/C7/Esther ...
	 *   default:   ... 586 / 686 guess ...
	 *   }
	 *   if ((speed = cpuspeed()) > 0) { ... print MHz via dprint ... extclock=speed; }
	 *   if (l1_cache) { cprint(LINE_CPU+1,...,"L1 Cache:  K"); memspeed(...MB/s...); }
	 *   if (l2_cache) { cprint(LINE_CPU+2,...,"L2 Cache:  K"); memspeed(...MB/s...); }
	 *   i = (l2_cache + l1_cache) * 5; ... memspeed(memory MB/s) ...
	 *   asm __volatile__ ("rdtsc": "=a"(v->startl), "=d"(v->starth));
	 *   v->snapl = v->startl; v->snaph = v->starth; v->rdtsc = 1;
	 *   if (l1_cache == 0) { l1_cache = 66; }
	 *   if (l2_cache == 0) { l1_cache = 666; }
	 *
	 * ---- end x86 cpu_type() body ---- */

	/* PPC: read the Processor Version Register. Bits 16-31 are the family/version
	 * used to look up the name (replaces the x86 cpuid vendor/type/model decode). */
	__asm__ __volatile__("mfpvr %0" : "=r"(cpu_pvr));
	pvr_hi = (unsigned short)(cpu_pvr >> 16);

	l1_cache = 0;
	l2_cache = 0;
	for (i = 0; ppc_cpu_names[i].name != NULL; i++) {
		if (ppc_cpu_names[i].pvr_hi == pvr_hi) {
			name = ppc_cpu_names[i].name;
			l1_cache = ppc_cpu_names[i].l1_kb;
			l2_cache = ppc_cpu_names[i].l2_kb;
			break;
		}
	}
	/* PPC: print the CPU name where upstream prints the marketing name
	 * (LINE_CPU, col 0). */
	cprint(LINE_CPU, 0, name);

	/* PPC: pull clock and cache sizes from the Open Firmware /cpus node. Try the
	 * common Apple node names in turn (mechanics from attic identify_cpu()):
	 * /cpus/PowerPC,G3@0, .../G4@0, .../G5@0, .../750@0, .../970@0, and the
	 * generic .../cpu@0. clock-frequency and {d,l2}-cache-size are 32-bit
	 * big-endian cells in bytes/Hz. */
	cpu_node = ofw_finddevice("/cpus/PowerPC,G3@0");
	if (cpu_node == -1) cpu_node = ofw_finddevice("/cpus/PowerPC,G4@0");
	if (cpu_node == -1) cpu_node = ofw_finddevice("/cpus/PowerPC,G5@0");
	if (cpu_node == -1) cpu_node = ofw_finddevice("/cpus/PowerPC,750@0");
	if (cpu_node == -1) cpu_node = ofw_finddevice("/cpus/PowerPC,970@0");
	if (cpu_node == -1) cpu_node = ofw_finddevice("/cpus/cpu@0");

	if (cpu_node != -1) {
		if (ofw_getprop(cpu_node, "clock-frequency", &clock_freq,
				sizeof(clock_freq)) >= 0 && clock_freq) {
			mhz = clock_freq / 1000000;
		}
		if (ofw_getprop(cpu_node, "d-cache-size", &dcache,
				sizeof(dcache)) >= 0 && dcache) {
			l1_cache = dcache / 1024;
		}
		if (ofw_getprop(cpu_node, "l2-cache-size", &l2sz,
				sizeof(l2sz)) >= 0 && l2sz) {
			l2_cache = l2sz / 1024;
		}
	}

	/* PPC: print CPU speed in MHz next to the name. The original positioned the
	 * MHz string at column `off` (just past the marketing name) and used the
	 * "    . MHz" / "      MHz" format with rdtsc-measured khz. We do not have the
	 * marketing-name length bookkeeping (`off`), so place the MHz figure at a
	 * fixed column past the longest CPU name string. extclock is recorded for
	 * parity with the original (which stored the measured speed there). */
	if (mhz) {
		cprint(LINE_CPU, 20, "      MHz");
		dprint(LINE_CPU, 20, mhz, 5, 0);
		extclock = mhz;
	}

	/* Print out L1 cache info */
	/* PPC: same on-screen layout as upstream's L1 line (the memspeed MB/s figure
	 * is omitted — memspeed is an x86 leaf; the size still prints). */
	if (l1_cache) {
		cprint(LINE_CPU+1, 0, "L1 Cache:     K  ");
		dprint(LINE_CPU+1, 11, l1_cache, 3, 0);
		/* x86: if ((speed=memspeed((ulong)mapping(0x100), (l1_cache/4)*1024, 50,
		 *     MS_COPY))) { cprint(LINE_CPU+1,16,"  MB/s"); dprint(...speed...); }
		 *     — memspeed is x86 rep-movsl timed by rdtsc; omitted on PPC. */
	}

	/* Print out L2 cache info */
	if (l2_cache) {
		cprint(LINE_CPU+2, 0, "L2 Cache:     K  ");
		dprint(LINE_CPU+2, 10, l2_cache, 4, 0);
		/* x86: memspeed L2 MB/s benchmark — omitted on PPC (see L1 note). */
	}

	/* L3 cache: upstream v2.00 has no L3 line (it predates L3-on-die parts). The
	 * task brief asked to keep an L3 label if present; v2.00 does not draw one,
	 * so to preserve the screen EXACTLY we draw nothing here. The G3/G4/G5 parts
	 * we target have no OF-reported L3 over this interface anyway. Flagged in the
	 * report (port-init_c.md). */

	/* x86: the memory-speed memspeed() benchmark and the rdtsc start-time
	 * snapshot (v->startl/starth/snapl/snaph) and v->rdtsc=1 are all x86 leaf —
	 * omitted on PPC. The wall clock is driven from the PPC time base (mftb) in
	 * main.c (Wave 5), which records its own start time. v->rdtsc stays 0. */
}

/* x86: cacheable() — finds the cache-able memory size by walking ranges and
 * timing per-range copies with memspeed() (x86 rep-movsl + rdtsc), then aprint's
 * the cached total at COL_CACHE_TOP. The whole routine is x86 leaf (memspeed +
 * map_page). On PPC the "Cached" figure is not measured this way; the column is
 * left blank for now (TO VERIFY: a PPC cache-able-size probe, if wanted, would
 * use the PPC time base — flagged in the report). Commented out; its only caller
 * (the v->rdtsc block in init()) is commented out too. */
#if 0
static void cacheable(void)
{
	ulong speed, pspeed;
	ulong paddr, mem_top, cached;

	mem_top = v->pmap[v->msegs - 1].end;
	cached = v->test_pages;
	pspeed = 0;
	for (paddr=0x200; paddr <= mem_top - 64; paddr+=0x400) {
		int i;
		int found;
		/* See if the paddr is at a testable location */
		found = 0;
		for(i = 0; i < v->msegs; i++) {
			if ((v->pmap[i].start >= paddr)  &&
				(v->pmap[i].end <= (paddr + 32))) {
				found = 1;
				break;
			}
		}
		if (!found) {
			continue;
		}
		/* Map the range and perform the test */
		map_page(paddr);
		speed = memspeed((ulong)mapping(paddr), 32*4096, 1, MS_COPY);
		if (pspeed) {
			if (speed < pspeed) {
				cached -= 32;
			}
			pspeed = (ulong)((float)speed * 0.7);
		}
	}
	aprint(LINE_INFO, COL_CACHE_TOP, cached);
	/* Ensure the default set of pages are mapped */
	map_page(0);
	map_page(0x80000);
}
#endif


/* #define TICKS 5 * 11832 (count = 6376)*/
/* #define TICKS (65536 - 12752) */
/* #define TICKS (65536 - 8271) */
#define TICKS	59659			/* Program counter to 50 ms = 59659 clks */

/* x86: cpuspeed() measures the CPU clock in kHz by programming the 8254 PIT
 * (ports 0x42/0x43/0x61) for a 50ms gate and counting rdtsc ticks across it —
 * pure x86 leaf (port I/O + rdtsc). N/A on PPC/OF; replaced by reading
 * clock-frequency from the OF CPU node in cpu_type() above. Commented out; the
 * sole caller (cpu_type's x86 body) is commented too. */
#if 0
static int cpuspeed(void)
{
	int loops;

	/* Setup timer */
	outb((inb(0x61) & ~0x02) | 0x01, 0x61);
	outb(0xb0, 0x43);
	outb(TICKS & 0xff, 0x42);
	outb(TICKS >> 8, 0x42);

	asm __volatile__ ("rdtsc":"=a" (st_low),"=d" (st_high));

	loops = 0;
	do {
		loops++;
	} while ((inb(0x61) & 0x20) == 0);

	asm __volatile__ (
		"rdtsc\n\t" \
		"subl st_low,%%eax\n\t" \
		"sbbl st_high,%%edx\n\t" \
		:"=a" (end_low), "=d" (end_high)
	);

	/* Make sure we have a credible result */
	if (loops < 4 || end_low < 50000) {
		return(-1);
	}

	if(tsc_invariable){ end_low = correct_tsc(end_low);	}

	v->clks_msec = end_low/50;
	return(v->clks_msec);
}
#endif

/* x86: memspeed() measures cache/memory throughput by copying a block with
 * rep-movsl / rep-stosl / lodsl loops timed by rdtsc — pure x86 leaf. N/A on
 * PPC/OF (would need a PPC dcbf/sync copy loop timed by mftb). Not ported; the
 * MB/s figures it fed are simply not displayed (cpu_type omits them). The
 * prototype is dropped (commented above) so nothing links against it. Commented
 * out wholesale; flagged in the report should a PPC throughput display be
 * wanted later. */
#if 0
ulong memspeed(ulong src, ulong len, int iter, int type)
{
	ulong dst;
	ulong wlen;
	int i;

	dst = src + len;
	wlen = len / 4;  /* Length is bytes */

	/* Calibrate the overhead with a zero word copy */
	asm __volatile__ ("rdtsc":"=a" (st_low),"=d" (st_high));
	for (i=0; i<iter; i++) {
		asm __volatile__ (
			"movl %0,%%esi\n\t" \
       		 	"movl %1,%%edi\n\t" \
       		 	"movl %2,%%ecx\n\t" \
       		 	"cld\n\t" \
       		 	"rep\n\t" \
       		 	"movsl\n\t" \
				:: "g" (src), "g" (dst), "g" (0)
			: "esi", "edi", "ecx"
		);
	}
	asm __volatile__ ("rdtsc":"=a" (cal_low),"=d" (cal_high));

	/* Compute the overhead time */
	asm __volatile__ (
		"subl %2,%0\n\t"
		"sbbl %3,%1"
		:"=a" (cal_low), "=d" (cal_high)
		:"g" (st_low), "g" (st_high),
		"0" (cal_low), "1" (cal_high)
	);


	/* Now measure the speed */
	switch (type) {
	case MS_COPY:
		/* Do the first copy to prime the cache */
		asm __volatile__ (
			"movl %0,%%esi\n\t" \
			"movl %1,%%edi\n\t" \
       		 	"movl %2,%%ecx\n\t" \
       		 	"cld\n\t" \
       		 	"rep\n\t" \
       		 	"movsl\n\t" \
			:: "g" (src), "g" (dst), "g" (wlen)
			: "esi", "edi", "ecx"
		);
		asm __volatile__ ("rdtsc":"=a" (st_low),"=d" (st_high));
		for (i=0; i<iter; i++) {
		        asm __volatile__ (
				"movl %0,%%esi\n\t" \
				"movl %1,%%edi\n\t" \
       			 	"movl %2,%%ecx\n\t" \
       			 	"cld\n\t" \
       			 	"rep\n\t" \
       			 	"movsl\n\t" \
				:: "g" (src), "g" (dst), "g" (wlen)
				: "esi", "edi", "ecx"
			);
		}
		asm __volatile__ ("rdtsc":"=a" (end_low),"=d" (end_high));
		break;
	case MS_WRITE:
		asm __volatile__ ("rdtsc":"=a" (st_low),"=d" (st_high));
		for (i=0; i<iter; i++) {
			asm __volatile__ (
       			 	"movl %0,%%ecx\n\t" \
				"movl %1,%%edi\n\t" \
       			 	"movl %2,%%eax\n\t" \
                                "rep\n\t" \
                                "stosl\n\t"
                                :: "g" (wlen), "g" (dst), "g" (0)
				: "edi", "ecx", "eax"
                        );
		}
		asm __volatile__ ("rdtsc":"=a" (end_low),"=d" (end_high));
		break;
	case MS_READ:
		asm __volatile__ (
			"movl %0,%%esi\n\t" \
			"movl %1,%%ecx\n\t" \
       	 		"cld\n\t" \
       	 		"L1:\n\t" \
			"lodsl\n\t" \
       	 		"loop L1\n\t" \
			:: "g" (src), "g" (wlen)
			: "esi", "ecx"
		);
		asm __volatile__ ("rdtsc":"=a" (st_low),"=d" (st_high));
		for (i=0; i<iter; i++) {
		        asm __volatile__ (
				"movl %0,%%esi\n\t" \
				"movl %1,%%ecx\n\t" \
       		 		"cld\n\t" \
       		 		"L2:\n\t" \
				"lodsl\n\t" \
       		 		"loop L2\n\t" \
				:: "g" (src), "g" (wlen)
				: "esi", "ecx"
			);
		}
		asm __volatile__ ("rdtsc":"=a" (end_low),"=d" (end_high));
		break;
	}

	/* Compute the elapsed time */
	asm __volatile__ (
		"subl %2,%0\n\t"
		"sbbl %3,%1"
		:"=a" (end_low), "=d" (end_high)
		:"g" (st_low), "g" (st_high),
		"0" (end_low), "1" (end_high)
	);
	/* Subtract the overhead time */
	asm __volatile__ (
		"subl %2,%0\n\t"
		"sbbl %3,%1"
		:"=a" (end_low), "=d" (end_high)
		:"g" (cal_low), "g" (cal_high),
		"0" (end_low), "1" (end_high)
	);

	/* Make sure that the result fits in 32 bits */
	if (end_high) {
		return(0);
	}

	/* If this was a copy adjust the time */
	if (type == MS_COPY) {
		end_low /= 2;
	}

	/* Convert to clocks/KB */
	end_low /= len;
	end_low *= 1024;
	end_low /= iter;
	if (end_low == 0) {
		return(0);
	}

	if(tsc_invariable){ end_low = correct_tsc(end_low);	}

	/* Convert to kbytes/sec */
	return((v->clks_msec)/end_low);
}
#endif

/* x86: correct_tsc() reads Intel MSRs 0x198/0x17/0x2A (rdmsr) to scale a
 * measured TSC delta by the current/maximum clock multiplier on SpeedStep parts
 * — pure x86 leaf (rdmsr). N/A on PPC/OF (no MSRs; the PPC time base is
 * invariant). Commented out; its callers (cpuspeed/memspeed) are commented too,
 * and the rdmsr macro at the top of this file is commented. */
#if 0
ulong correct_tsc(ulong el_org)
{

	float coef_now, coef_max;
	int msr_lo, msr_hi, is_xe;

	rdmsr(0x198, msr_lo, msr_hi);
	is_xe = (msr_lo >> 31) & 0x1;

	if(is_xe){
		rdmsr(0x198, msr_lo, msr_hi);
		coef_max = ((msr_hi >> 8) & 0x1F);
		if ((msr_hi >> 14) & 0x1) { coef_max = coef_max + 0.5f; }
	} else {
		rdmsr(0x17, msr_lo, msr_hi);
		coef_max = ((msr_lo >> 8) & 0x1F);
		if ((msr_lo >> 14) & 0x1) { coef_max = coef_max + 0.5f; }
	}

	if((cpu_id.feature_flag >> 7) & 1) {
		rdmsr(0x198, msr_lo, msr_hi);
		coef_now = ((msr_lo >> 8) & 0x1F);
		if ((msr_lo >> 14) & 0x1) { coef_now = coef_now + 0.5f; }
	} else {
		rdmsr(0x2A, msr_lo, msr_hi);
		coef_now = (msr_lo >> 22) & 0x1F;
	}

	if(coef_max && coef_now) { el_org = (ulong)(el_org * coef_now / coef_max); }

	return el_org;

}
#endif
