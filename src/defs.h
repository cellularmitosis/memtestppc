/* defs.h - MemTest-86 assembler/compiler definitions
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady, cbrady@sgi.com
 */

/* ----------------------------------------------------------------------------
 * memtestppc+ port note (session 006, v2.00 -> PowerPC/Open Firmware):
 *
 * defs.h holds the x86 *boot/relocation* constants shared between the C code
 * and the real-mode assembler (bootsect.S/setup.S/head.S). EVERY constant here
 * is x86-specific: real-mode segment addresses and 32-/16-bit GDT selectors for
 * the PC boot protocol, plus the low address the test image relocates itself to.
 *
 * On PPC/Open Firmware NONE of this applies: OF's `load` claims and maps the ELF
 * (MMU on), our PPC head.S sets up the stack in BSS, and linker.ld owns the load
 * address (0x00400000) — there is no real-mode boot, no GDT, and no x86 640K
 * low-memory relocation target. Per the import-everything discipline we bring
 * the file in verbatim and disable the body (wrapped in `#if 0 ... #endif`, which
 * keeps the original lines — and their inner comments — byte-for-byte) rather
 * than deleting it, so the original boot model stays visible.
 *
 * Cross-file note: in upstream main.c, LOW_TEST_ADR and INITSEG are referenced
 * only by the x86 self-relocation / boot-protocol command-line machinery
 * (__run_at/run_at, MK_PTR(INITSEG,...) — main.c:99,132,140,155,195,200,221,
 * 422,456,477). Those are exactly the regions the main.c port comments out
 * (HANDOFF: "comment reloc/window/cmdline"). So disabling these defs here is
 * consistent — the call sites go away in the same pass. The PPC port tests RAM
 * in place (claimed via OF) instead of relocating to a fixed low address.
 * ------------------------------------------------------------------------- */

/* x86 boot/relocation constants — N/A on PPC/OF. Disabled as one region so the
 * upstream definitions (with their original inline comments) survive verbatim
 * for reference. See the header note above for the per-symbol rationale. */
#if 0	/* x86: PC real-mode boot + GDT + relocation constants — N/A on PPC/OF */

#define SETUPSECS	4		/* Number of setup sectors */

/*
 * Caution!! There is magic in the build process.  Read
 * README.build-process before you change anything.
 * Unlike earlier versions all of the settings are in defs.h
 * so the build process should be more robust.
 */
#define LOW_TEST_ADR	0x00002000		/* Final adrs for test code */

#define BOOTSEG		0x07c0			/* Segment adrs for inital boot */
#define INITSEG		0x9000			/* Segment adrs for relocated boot */
#define SETUPSEG	(INITSEG+0x20)		/* Segment adrs for relocated setup */
#define TSTLOAD		0x1000			/* Segment adrs for load of test */

#define KERNEL_CS	0x10			/* 32 bit segment adrs for code */
#define KERNEL_DS	0x18			/* 32 bit segment adrs for data */
#define REAL_CS		0x20			/* 16 bit segment adrs for code */
#define REAL_DS		0x28			/* 16 bit segment adrs for data */

#endif	/* 0 — x86 boot/relocation constants */
