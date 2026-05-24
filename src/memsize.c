/* memsize.c - MemTest-86  Version 3.2
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady
 * ----------------------------------------------------
 * MemTest86+ V2.00 Specific code (GPL V2.0)
 * By Samuel DEMEULEMEESTER, sdemeule@memtest.org
 * http://www.x86-secret.com - http://www.memtest.org
 * ----------------------------------------------------
 * memtestppc+ port: line-faithful port of memtest86+ v2.00 memsize.c to
 * PowerPC / Open Firmware (single-CPU). Discipline (see
 * docs/sessions/006-port-fidelity-restructure/PORTING-GUIDE.md): start from
 * the verbatim v2.00 file; comment out x86/BIOS leaf code with a one-line
 * reason; add PPC/OF replacements inline, marked "PPC:". Nothing from the old
 * attic/src-v0.01 hack is inherited.
 *
 * On x86, memory discovery is the BIOS's job: the bootloader queries the
 * BIOS (E820 / E801 / E88 memory maps), or coreboot/LinuxBIOS tables, and
 * leaves the result in `mem_info`; mem_size() then sanitizes that map into
 * v->pmap[]. None of that exists on a PowerPC Mac. Open Firmware owns the
 * machine: it reports installed RAM in the device tree (the /memory node's
 * "reg" property, a list of (addr,size) pairs), and — crucially — it runs
 * with the MMU ON, so we must ofw_claim() each physical region before we may
 * touch it (claim both allocates and maps). So the whole BIOS probing body of
 * mem_size() is replaced by OF discovery + a 1 MB-chunk claim, while the
 * platform-neutral scaffolding (sort_pmap, the pmap-page units, plim_*,
 * adj_mem) is preserved verbatim so downstream (init.c display, main.c
 * windowing, config.c address-range) sees the exact same shape.
 */

#include "test.h"
/* #include "defs.h" */ /* x86: defs.h body is #if 0'd in the PPC port (real-mode
                         * boot/GDT/reloc segment defs) — nothing here needs it. */
#include "config.h"
#include "ofw.h" /* PPC: OF client interface — ofw_finddevice/getprop/claim.
                  * This is the io.h/BIOS replacement (PORTING-GUIDE §4). */

/* x86: these globals describe BIOS-supplied memory state and the sizing mode.
 * On PPC/OF there is no BIOS E820 map and no LinuxBIOS, so the BIOS sizing
 * modes are inert. We keep memsz_mode/firmware *defined* (config.c and init.c
 * reference them via `extern` in later waves) at neutral values; e820_nr stays
 * 0 so config.c's "Memory Sizing" menu correctly treats the BIOS map as empty.
 * Their x86 producers (setup.S / linuxbios.c) are not ported. */
short e820_nr = 0;
short memsz_mode = SZ_MODE_PROBE;        /* PPC: not BIOS; we discover via OF */
short firmware = FIRMWARE_UNKNOWN;       /* PPC: no PC BIOS / LinuxBIOS */

/* x86: scratch for the BIOS E801/E88 extended-memory sizes and a working copy
 * of the E820 map. N/A on PPC/OF — our map comes straight from OF's /memory
 * "reg". Commented out so memsize.c no longer references the (undefined-on-PPC)
 * `mem_info` symbol that setup.S would have populated. */
/*
static ulong alt_mem_k;
static ulong ext_mem_k;
static struct e820entry e820[E820MAX];
*/

extern ulong p1, p2;
extern volatile ulong *p;

static void sort_pmap(void);
/* x86: BIOS/probe helpers — see each definition below for disposition. */
/* static int check_ram(void); */
/* static void memsize_bios(int res); */
/* static void memsize_820(int res); */
/* static void memsize_801(void); */
/* static int sanitize_e820_map(struct e820entry *orig_map, struct e820entry *new_bios,
	short old_nr, short res); */
/* static void memsize_linuxbios(); */
/* static void memsize_probe(void); */
/* static int check_ram(void); */
static void memsize_ofw(void); /* PPC: OF discovery + claim, replaces the BIOS probe */

/*
 * Find out how much memory there is.
 */
void mem_size(void)
{
	/* int i; */ /* PPC: only used by the commented-out mem_info copy below */
	v->reserved_pages = 0;
	v->test_pages = 0;

	/* x86: On the first call, snapshot the BIOS memory-info table so the map
	 * can be re-evaluated later (e.g. when the user toggles sizing mode in the
	 * config menu). N/A on PPC/OF — there is no `mem_info`; OF's device tree is
	 * the authoritative source and is re-read each call by memsize_ofw().
	 *
	 * if (e820_nr == 0 && alt_mem_k == 0 && ext_mem_k == 0) {
	 * 	ext_mem_k = mem_info.e88_mem_k;
	 * 	alt_mem_k = mem_info.e801_mem_k;
	 * 	e820_nr   = mem_info.e820_nr;
	 * 	for (i=0; i< mem_info.e820_nr; i++) {
	 * 		e820[i].addr = mem_info.e820[i].addr;
	 * 		e820[i].size = mem_info.e820[i].size;
	 * 		e820[i].type = mem_info.e820[i].type;
	 * 	}
	 * }
	 */

	/* x86: dispatch on the BIOS sizing mode (E820/E820+reserved/probe). On
	 * PPC/OF there is exactly one source of truth — Open Firmware — so all
	 * three modes collapse to a single OF discovery pass. memsize_ofw() reads
	 * /memory "reg", builds v->pmap[] (in 4K pages) and ofw_claims the RAM in
	 * 1 MB chunks. Kept the cprint label so the LINE_INFO/COL_MMAP cell that
	 * upstream's SZ_MODE_PROBE printed (" Probed ") still gets written.
	 *
	 * switch (memsz_mode) {
	 * case SZ_MODE_BIOS:
	 * 	memsize_bios(0);
	 * 	break;
	 * case SZ_MODE_BIOS_RES:
	 * 	memsize_bios(1);
	 * 	break;
	 * case SZ_MODE_PROBE:
	 * 	memsize_probe();
	 * 	cprint(LINE_INFO, COL_MMAP, " Probed ");
	 * 	break;
	 * }
	 */
	/* PPC: single OF discovery pass (allocates+maps via ofw_claim). */
	memsize_ofw();
	cprint(LINE_INFO, COL_MMAP, "OFW Map ");

	/* Guarantee that pmap entries are in ascending order */
	sort_pmap();
	v->plim_lower = 0;
	v->plim_upper = v->pmap[v->msegs-1].end;

	adj_mem();
	aprint(LINE_INFO, COL_RESERVED, v->reserved_pages);
}

/* x86: memsize_bios() picks between the PC-BIOS E820 path and the LinuxBIOS
 * (coreboot) table path based on `firmware`. Both are PC firmware mechanisms
 * with no PPC counterpart — replaced wholesale by memsize_ofw(). Kept for
 * reference. */
/*
static void memsize_bios(int res)
{
	if (firmware == FIRMWARE_PCBIOS) {
		memsize_820(res);
	}
	else if (firmware == FIRMWARE_LINUXBIOS) {
		memsize_linuxbios();
	}
}
*/

/* PPC: discover memory from Open Firmware, then claim it.
 *
 * Mechanics (proven on QEMU OpenBIOS and real Apple New World OF; carried from
 * the old attic/src-v0.01 discover_memory(), re-implemented here inside v2.00's
 * structure):
 *   1. finddevice("/memory") and read its "reg" property — a list of
 *      (addr,size) cells. On 32-bit PPC these are 1 cell (ulong) each, so we
 *      walk reg[] in (addr,size) pairs.
 *   2. Skip the low region (< LOW_TEST_ADR). Real Apple OF maps our loaded ELF,
 *      OF itself, the OF stack, exception vectors and the framebuffer/structures
 *      into low memory; touching it would fault or trash OF. (See the
 *      "Low-memory skip" note in port-memsize_c.md for why 8 MB.) This is the
 *      PPC analog of upstream's RES_START..RES_END (640K..1M) BIOS hole — same
 *      idea, different platform reservation.
 *   3. ofw_claim() the RAM in 1 MB chunks. With OF's MMU on, claim both
 *      reserves and maps the page. Claiming the whole range at once fails if OF
 *      already owns anything inside it, so we go chunk by chunk and coalesce
 *      runs of successful claims into v->pmap[] segments; an in-use chunk
 *      simply ends the current run.
 *
 * Units: v->pmap[].start/.end are 4K PAGE numbers (addr >> 12), exactly as
 * upstream's BIOS paths produced them, so init.c/main.c/adj_mem read the same
 * shape. v->test_pages is the sum of (end-start) over all segments, in pages.
 */
#define LOW_TEST_ADR  0x800000UL   /* PPC: skip first 8 MB (code/OF/fb) */
#define CLAIM_CHUNK   0x100000UL    /* PPC: 1 MB claim granularity */

static void memsize_ofw(void)
{
	ofw_handle_t mem_node;
	ulong reg[64];                 /* PPC: (addr,size) pairs from /memory "reg" */
	int len, i, n;

	n = 0;
	v->test_pages = 0;

	mem_node = ofw_finddevice("/memory");
	if (mem_node == -1) {
		mem_node = ofw_finddevice("/memory@0");
	}
	if (mem_node == -1) {
		/* PPC: no /memory node — should not happen on a real Mac. Leave the
		 * map empty; main.c reports "No memory segments to test". */
		v->msegs = 0;
		return;
	}

	len = ofw_getprop(mem_node, "reg", reg, sizeof(reg));
	if (len < 0) {
		len = 0;
	}

	for (i = 0; i < len / (int)sizeof(ulong); i += 2) {
		ulong base = reg[i];
		ulong size = reg[i + 1];
		ulong addr, end_addr;

		if (size == 0) {
			continue;
		}

		/* Skip the low region (OF/our image/framebuffer live here). */
		if (base < LOW_TEST_ADR) {
			if (base + size <= LOW_TEST_ADR) {
				/* Entire region is below the line — count as reserved
				 * (matches upstream: memory we choose not to test). */
				v->reserved_pages += size >> 12;
				continue;
			}
			v->reserved_pages += (LOW_TEST_ADR - base) >> 12;
			size -= (LOW_TEST_ADR - base);
			base = LOW_TEST_ADR;
		}

		/* Claim in 1 MB chunks, coalescing successful claims into segments. */
		end_addr = base + size;
		for (addr = base; addr < end_addr; addr += CLAIM_CHUNK) {
			ulong chunk = end_addr - addr;
			if (chunk > CLAIM_CHUNK) {
				chunk = CLAIM_CHUNK;
			}

			if (ofw_claim((void *)addr, chunk, 0) == (void *)-1) {
				/* In use by OF — count as reserved, end the current run. */
				v->reserved_pages += chunk >> 12;
				continue;
			}

			/* Extend the current segment if this chunk is adjacent, else
			 * open a new one. pmap units are 4K pages (addr >> 12). */
			if (n > 0 && v->pmap[n - 1].end == (addr >> 12)) {
				v->pmap[n - 1].end = (addr + chunk) >> 12;
			} else if (n < MAX_MEM_SEGMENTS) {
				v->pmap[n].start = addr >> 12;
				v->pmap[n].end = (addr + chunk) >> 12;
				n++;
			}
		}
	}

	v->msegs = n;
	for (i = 0; i < n; i++) {
		v->test_pages += v->pmap[i].end - v->pmap[i].start;
	}
	/* PPC: set selected_pages too. Upstream relies on mem_size()'s call to
	 * adj_mem() (config.c, ported in a later wave) to derive selected_pages
	 * from pmap + plim_*; setting it here means the value is correct even at
	 * the Wave-3 checkpoint before adj_mem() exists, and adj_mem() recomputes
	 * the identical value (plim covers the whole map) once it lands. */
	v->selected_pages = v->test_pages;
}

static void sort_pmap(void)
{
	int i, j;
	/* Do an insertion sort on the pmap, on an already sorted
	 * list this should be a O(1) algorithm.
	 */
	for(i = 0; i < v->msegs; i++) {
		/* Find where to insert the current element */
		for(j = i -1; j >= 0; j--) {
			if (v->pmap[i].start > v->pmap[j].start) {
				j++;
				break;
			}
		}
		/* Insert the current element */
		if (i != j) {
			struct pmap temp;
			temp = v->pmap[i];
			memmove(&v->pmap[j], &v->pmap[j+1],
				(i -j)* sizeof(temp));
			v->pmap[j] = temp;
		}
	}
}

/* x86: memsize_linuxbios() turns a coreboot/LinuxBIOS E820-style table into
 * v->pmap[]. LinuxBIOS does not exist on a PowerPC Mac (firmware is always
 * Open Firmware). Replaced by memsize_ofw(). Kept verbatim for reference.
 * Note: the pmap construction here (addr>>12 page units, test_pages summing)
 * is exactly what memsize_ofw() reproduces. */
/*
static void memsize_linuxbios(void)
{
	int i, n;
	n = 0;
	for (i=0; i < e820_nr; i++) {
		unsigned long long end;
		if (e820[i].type != E820_RAM) {
			continue;
		}
		end = e820[i].addr;
		end += e820[i].size;
		v->pmap[n].start = (e820[i].addr + 4095) >> 12;
		v->pmap[n].end = end >> 12;
		v->test_pages += v->pmap[n].end - v->pmap[n].start;
		n++;
	}
	v->msegs = n;
	cprint(LINE_INFO, COL_MMAP, "LinuxBIOS");
}
*/

/* x86: memsize_820() sanitizes the BIOS E820 map and skips the 640K..1M PC
 * hole (RES_START..RES_END). All BIOS-specific; replaced by memsize_ofw()
 * (which does the analogous low-memory skip via LOW_TEST_ADR). Kept verbatim. */
/*
static void memsize_820(int res)
{
	int i, n, nr;
	struct e820entry nm[E820MAX];

	nr = sanitize_e820_map(e820, nm, e820_nr, res);

	if (nr < 1 || nr > E820MAX) {
		memsize_801();
		return;
	}

	n = 0;
	for (i=0; i<nr; i++) {
		if (nm[i].type == E820_RAM) {
			unsigned long long start;
			unsigned long long end;
			start = nm[i].addr;
			end = start + nm[i].size;

			if (start > RES_START && start < RES_END) {
				if (end < RES_END) {
					continue;
				}
				start = RES_END;
			}
			if (end > RES_START && end < RES_END) {
				end = RES_START;
			}
			v->pmap[n].start = (start + 4095) >> 12;
			v->pmap[n].end = end >> 12;
			v->test_pages += v->pmap[n].end - v->pmap[n].start;
			n++;
		} else {
			if (nm[i].addr < 0xff000000) {
				v->reserved_pages += nm[i].size >> 12;
			}
		}
	}
	v->msegs = n;
	if (res) {
		cprint(LINE_INFO, COL_MMAP, "e820-All");
	} else {
		cprint(LINE_INFO, COL_MMAP, "e820-Std");
	}
}
*/

/* x86: memsize_801() is the fallback when no usable E820 map exists — it uses
 * the BIOS E801/E88 extended-memory sizes (1K units) and hardcodes the PC
 * 640K/1M split. Pure BIOS; N/A on PPC/OF. Kept verbatim for reference. */
/*
static void memsize_801(void)
{
	ulong mem_size;

	if (alt_mem_k < ext_mem_k) {
		mem_size = ext_mem_k;
		cprint(LINE_INFO, COL_MMAP, "e88-Std ");
	} else {
		mem_size = alt_mem_k;
		cprint(LINE_INFO, COL_MMAP, "e801-Std");
	}
	v->pmap[0].start = 0;
	v->pmap[0].end = RES_START >> 12;
	v->test_pages = RES_START >> 12;

	v->pmap[1].start = (RES_END + 4095) >> 12;
	v->pmap[1].end = (mem_size + 1024) >> 2;
	v->test_pages += mem_size >> 2;
	v->msegs = 2;
}
*/

/*
 * x86: Sanitize the BIOS e820 map.
 *
 * Some e820 responses include overlapping entries.  The following
 * replaces the original e820 map with a new one, removing overlaps.
 *
 * Entirely BIOS-map machinery — there is no E820 map on PPC/OF. Open Firmware
 * hands us a clean, non-overlapping /memory "reg" list, so no sanitization is
 * needed. Kept verbatim (inside an #if 0 region, since it is large dead code
 * that references the commented-out e820[] scratch) for upstream fidelity.
 */
#if 0
static int sanitize_e820_map(struct e820entry *orig_map, struct e820entry *new_bios,
	short old_nr, short res)
{
	struct change_member {
		struct e820entry *pbios; /* pointer to original bios entry */
		unsigned long long addr; /* address for this change point */
	};
	struct change_member change_point_list[2*E820MAX];
	struct change_member *change_point[2*E820MAX];
	struct e820entry *overlap_list[E820MAX];
	struct e820entry biosmap[E820MAX];
	struct change_member *change_tmp;
	ulong current_type, last_type;
	unsigned long long last_addr;
	int chgidx, still_changing;
	int overlap_entries;
	int new_bios_entry;
	int i;

	/*
		Visually we're performing the following (1,2,3,4 = memory types)...
		Sample memory map (w/overlaps):
		   ____22__________________
		   ______________________4_
		   ____1111________________
		   _44_____________________
		   11111111________________
		   ____________________33__
		   ___________44___________
		   __________33333_________
		   ______________22________
		   ___________________2222_
		   _________111111111______
		   _____________________11_
		   _________________4______

		Sanitized equivalent (no overlap):
		   1_______________________
		   _44_____________________
		   ___1____________________
		   ____22__________________
		   ______11________________
		   _________1______________
		   __________3_____________
		   ___________44___________
		   _____________33_________
		   _______________2________
		   ________________1_______
		   _________________4______
		   ___________________2____
		   ____________________33__
		   ______________________4_
	*/
	/* First make a copy of the map */
	for (i=0; i<old_nr; i++) {
		biosmap[i].addr = orig_map[i].addr;
		biosmap[i].size = orig_map[i].size;
		biosmap[i].type = orig_map[i].type;
	}

	/* bail out if we find any unreasonable addresses in bios map */
	for (i=0; i<old_nr; i++) {
		if (biosmap[i].addr + biosmap[i].size < biosmap[i].addr)
			return 0;
		if (res) {
			/* If we want to test the reserved memory include
			 * everything except for reserved segments that start
			 * at the  the top of memory
			 */
			if (biosmap[i].type == E820_RESERVED &&
					biosmap[i].addr > 0xff000000) {
				continue;
			}
			biosmap[i].type = E820_RAM;
		} else {
			/* It is always be safe to test ACPI ram */
			if ( biosmap[i].type == E820_ACPI) {
				biosmap[i].type = E820_RAM;
			}
		}
	}

	/* create pointers for initial change-point information (for sorting) */
	for (i=0; i < 2*old_nr; i++)
		change_point[i] = &change_point_list[i];

	/* record all known change-points (starting and ending addresses) */
	chgidx = 0;
	for (i=0; i < old_nr; i++)	{
		change_point[chgidx]->addr = biosmap[i].addr;
		change_point[chgidx++]->pbios = &biosmap[i];
		change_point[chgidx]->addr = biosmap[i].addr + biosmap[i].size;
		change_point[chgidx++]->pbios = &biosmap[i];
	}

	/* sort change-point list by memory addresses (low -> high) */
	still_changing = 1;
	while (still_changing)	{
		still_changing = 0;
		for (i=1; i < 2*old_nr; i++)  {
			/* if <current_addr> > <last_addr>, swap */
			/* or, if current=<start_addr> & last=<end_addr>, swap */
			if ((change_point[i]->addr < change_point[i-1]->addr) ||
				((change_point[i]->addr == change_point[i-1]->addr) &&
				 (change_point[i]->addr == change_point[i]->pbios->addr) &&
				 (change_point[i-1]->addr != change_point[i-1]->pbios->addr))
			   )
			{
				change_tmp = change_point[i];
				change_point[i] = change_point[i-1];
				change_point[i-1] = change_tmp;
				still_changing=1;
			}
		}
	}

	/* create a new bios memory map, removing overlaps */
	overlap_entries=0;	 /* number of entries in the overlap table */
	new_bios_entry=0;	 /* index for creating new bios map entries */
	last_type = 0;		 /* start with undefined memory type */
	last_addr = 0;		 /* start with 0 as last starting address */
	/* loop through change-points, determining affect on the new bios map */
	for (chgidx=0; chgidx < 2*old_nr; chgidx++)
	{
		/* keep track of all overlapping bios entries */
		if (change_point[chgidx]->addr == change_point[chgidx]->pbios->addr)
		{
			/* add map entry to overlap list (> 1 entry implies an overlap) */
			overlap_list[overlap_entries++]=change_point[chgidx]->pbios;
		}
		else
		{
			/* remove entry from list (order independent, so swap with last) */
			for (i=0; i<overlap_entries; i++)
			{
				if (overlap_list[i] == change_point[chgidx]->pbios)
					overlap_list[i] = overlap_list[overlap_entries-1];
			}
			overlap_entries--;
		}
		/* if there are overlapping entries, decide which "type" to use */
		/* (larger value takes precedence -- 1=usable, 2,3,4,4+=unusable) */
		current_type = 0;
		for (i=0; i<overlap_entries; i++)
			if (overlap_list[i]->type > current_type)
				current_type = overlap_list[i]->type;
		/* continue building up new bios map based on this information */
		if (current_type != last_type)	{
			if (last_type != 0)	 {
				new_bios[new_bios_entry].size =
					change_point[chgidx]->addr - last_addr;
				/* move forward only if the new size was non-zero */
				if (new_bios[new_bios_entry].size != 0)
					if (++new_bios_entry >= E820MAX)
						break;	/* no more space left for new bios entries */
			}
			if (current_type != 0)	{
				new_bios[new_bios_entry].addr = change_point[chgidx]->addr;
				new_bios[new_bios_entry].type = current_type;
				last_addr=change_point[chgidx]->addr;
			}
			last_type = current_type;
		}
	}
	return(new_bios_entry);
}
#endif /* sanitize_e820_map — x86 BIOS-map machinery, N/A on PPC/OF */

/* x86: memsize_probe() finds RAM by writing-and-reading-back across the address
 * space, starting just past the relocated image (&_end), skipping the PC video
 * hole (RES_START..RES_END), and stopping at the address-wrap limit. It pokes
 * raw physical addresses directly — fatal under OF's MMU, where unclaimed pages
 * are unmapped, and redundant since OF already enumerates RAM in /memory. The
 * proven PPC discover-then-claim approach lives in memsize_ofw() instead. Kept
 * verbatim (inside #if 0; it references set_cache/_end/check_ram/p,p1,p2) for
 * reference. */
#if 0
static void memsize_probe(void)
{
	int i, n;
	ulong m_lim;
	static unsigned long magic = 0x1234569;

	/* Since all address bits may not be decoded, the search for memory
	 * must be limited.  The max address is found by checking for
	 * memory wrap from 1MB to 4GB.  */
	p1 = (ulong)&magic;
	m_lim = 0xfffffffc;
	for (p2 = 0x100000; p2; p2 <<= 1) {
		p = (ulong *)(p1 + p2);
		if (*p == 0x1234569) {
			m_lim = --p2;
			break;
		}
	}

	/* Turn on cache */
	set_cache(1);

	/* Find all segments of RAM */

	i = 0;
	v->pmap[i].start = ((ulong)&_end + (1 << 12) - 1) >> 12;
	p = (ulong *)(v->pmap[i].start << 12);

	/* Limit search for memory to m_lim and make sure we don't
	 * overflow the 32 bit size of p.  */
	while ((ulong)p < m_lim && (ulong)p >= (ulong)&_end) {
		/*
		 * Skip over reserved memory
		 */
		if ((ulong)p < RES_END && (ulong)p >= RES_START) {
			v->pmap[i].end = RES_START >> 12;
			v->test_pages += (v->pmap[i].end - v->pmap[i].start);
			p = (ulong *)RES_END;
			i++;
			v->pmap[i].start = 0;
			goto fstart;
		}

		if (check_ram() == 0) {
			/* ROM or nothing at this address, record end addrs */
			v->pmap[i].end = ((ulong)p) >> 12;
			v->test_pages += (v->pmap[i].end - v->pmap[i].start);
			i++;
			v->pmap[i].start = 0;
fstart:

			/* We get here when there is a gap in memory.
			 * Loop until we find more ram, the gap is more
			 * than 32768k or we hit m_lim */
			n = 32768 >> 2;
			while ((ulong)p < m_lim && (ulong)p >= (ulong)&_end) {

				/* Skip over video memory */
				if ((ulong)p < RES_END &&
					(ulong)p >= RES_START) {
					p = (ulong *)RES_END;
				}
				if (check_ram() == 1) {
					/* More RAM, record start addrs */
					v->pmap[i].start = (ulong)p >> 12;
					break;
				}

				/* If the gap is 32768k or more then there
				 * is probably no more memory so bail out */
				if (--n <= 0) {
					p = (ulong *)m_lim;
					break;
				}
				p += 0x1000;
			}
		}
		p += 0x1000;
	}

	/* If there is ram right up to the memory limit this will record
	 * the last address.  */
	if (v->pmap[i].start) {
		v->pmap[i].end = m_lim >> 12;
		v->test_pages += (v->pmap[i].end - v->pmap[i].start);
		i++;
	}
	v->msegs = i;
}

/* check_ram - Determine if this address points to memory by checking
 * for a wrap pattern and then reading and then writing the complement.
 * We then check that at least one bit changed in each byte before
 * believing that it really is memory.
 *
 * x86: platform-neutral *logic*, but only ever called by memsize_probe(),
 * which we replaced with OF discovery (OF enumerates RAM authoritatively, so a
 * destructive probe is unnecessary and — writing unmapped addresses under OF's
 * MMU — unsafe). Parked alongside memsize_probe() inside the #if 0. */
static int check_ram(void)
{
	int s;

	p1 = *p;

	/* write the complement */
	*p = ~p1;
	p2 = *p;
	s = 0;

	/* Now make sure a bit changed in each byte */
	if ((0xff & p1) != (0xff & p2)) {
		s++;
	}
	if ((0xff00 & p1) != (0xff00 & p2)) {
		s++;
	}
	if ((0xff0000 & p1) != (0xff0000 & p2)) {
		s++;
	}
	if ((0xff000000 & p1) != (0xff000000 & p2)) {
		s++;
	}
	if (s == 4) {
		/* RAM at this address */
		return 1;
	}

	return 0;
}
#endif /* memsize_probe / check_ram — x86 destructive probe, replaced by OF discovery */
