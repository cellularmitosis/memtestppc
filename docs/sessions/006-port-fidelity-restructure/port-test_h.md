# port-test_h.md — porting memtest86+ v2.00 `test.h` → `src/test.h`

## File & purpose
`ref/memtest86+-2.00/test.h` is the project's master header: it declares every
shared struct (`vars`, `mmap`, `pmap`, `tseq`, `xadr`, `err_info`, `pair`,
`cpu_ident`, `e820entry`, `mem_info_t`, `eregs` fwd-decl), all the screen-layout
`#define`s (TITLE_WIDTH, LINE_*, COL_*, BAR_SIZE, POP_*), the test/sizing/print
constants (SPINSZ, MOD_SZ, BAILOUT/BAILR, MS_*, SZ_MODE_*, PRINTMODE_*), and
prototypes for essentially every function in the program (lines 100–177). It also
carries three x86 inline-asm leaf helpers — `cache_off`/`cache_on` (cr0 + wbinvd,
lines 194–212) and `reboot` (lines 214–235) — plus the `getCx86` port-I/O macro
(line 100). Guarded by `#ifndef __ASSEMBLY__` (line 20) so the asm build can pull
the top `E8xx` constants without the C content.

## Disposition
**Ported** (Wave-1 header). Started from a verbatim copy of v2.00 test.h. All
structs, layout constants, and prototypes preserved byte-for-byte except the x86
leaf regions noted below (commented, not deleted) and two `#include`s added at the
top for the PPC substrate. The file is header-only; nothing to build standalone —
the lead links it at the Wave checkpoint.

## x86 leaf code commented out (each region + reason)
1. **`SCREEN_ADR` / `SCREEN_END_ADR`** (orig lines 53–54). `#define SCREEN_ADR
   0xb8000` is x86 VGA text RAM — N/A on PPC/OF. Commented out; the active
   definition comes from `display.h` (see SCREEN_ADR reconciliation). No PPC
   replacement needed in test.h itself.
2. **`getCx86(reg)`** macro (orig line 100). `outb(reg,0x22); inb(0x23)` reads a
   Cyrix/x86 chipset register via port I/O — N/A on PPC/OF. Commented. Its only
   caller is `init.c:344` (x86 cache detection), handled when init.c is ported.
3. **`cache_off()` / `cache_on()`** (orig lines 194–212). Toggle the x86 cr0
   cache-disable bit + `wbinvd` — N/A on PPC. Commented as a block. Inner asm
   comments (`/* Set CD */` etc.) converted to `//` so the wrapping `/* … */`
   doesn't nest. PPC cache control will arrive via `set_cache()` / HID0 / dcbf
   when lib.c's callers (`lib.c:528,532`) are ported.
4. **`reboot()`** (orig lines 214–235). x86 cr0/cr3 reset + segment reload +
   far-jump to reset vector — N/A on PPC. Commented as a block (inner asm
   comment converted to `//`). PPC reboot is `ofw_reset()`/`ofw_exit()` from
   `ofw.{c,h}`; call sites rewired in later waves.
5. **`struct cpu_ident`** (orig lines 257–269). Mirrors x86 CPUID output (vendor
   string, capability/feature flags, cache_info) — N/A on PPC, which uses PVR +
   OF. Commented (not deleted) for fidelity. No current consumer in the ported
   set; revisit at init.c.

Left **verbatim and active** even though clearly PC-flavored, because they are
harmless data/decls and commenting them risks breaking ported consumers or hurts
fidelity: the `E8xx`/E820 BIOS-map constants and `struct e820entry`/`mem_info_t`
(may be referenced by memsize.c), `DMI_*`, `RES_START/RES_END`, `X86_FEATURE_PAE`,
`FIRMWARE_*`, the `extern startup_32[]` / `_size`/`_pages` / `mem_info` externs,
and every function prototype (unused prototypes are harmless and preserve the
diff). `struct eregs;` forward-decl + `inter()` prototype kept verbatim (the real
`struct eregs` lives in lib.c).

## PPC/OF additions
- **`#include "ppc.h"`** (right after `#ifndef __ASSEMBLY__`). Gives every ported
  `.c` the mftb/mftbu/dcbf/sync/isync intrinsics. v2.00 test.h had none (its x86
  leaf used rdtsc/wbinvd inline); the old attic memtest.h carried them inline — we
  instead include ppc.h and do NOT redefine them here.
- **`#include "display.h"`** — supplies the one active `SCREEN_ADR` (= vga_buf)
  plus `SCREEN_WIDTH/HEIGHT`, the fb backend prototypes, and `vga_palette`.
- Updated the top-of-file banner to record the port discipline.

## struct vars — fields restored vs the old attic hack
The full v2.00 `struct vars` is restored **verbatim** (29 fields). The old
`attic/src-v0.01/memtest.h` dropped many; restored-vs-hack:

| field | in old hack? |
|---|---|
| `int test` | dropped — restored |
| `unsigned long *eadr` | dropped — restored |
| `unsigned long exor` | dropped — restored |
| `int ecc_ecount` | dropped — restored |
| `int rdtsc` | dropped — restored |
| `int pae` | dropped — restored |
| `int beepmode` | dropped — restored |
| `ulong snaph` | dropped — restored |
| `ulong snapl` | dropped — restored |
| `ulong extclock` | dropped — restored |
| `ulong plim_upper` | dropped — restored |
| `struct pair patn[BADRAM_MAXPATNS]` | dropped — restored |
| `ulong reserved_pages` | dropped — restored |

Also note the old hack had `volatile struct mmap map[]`; v2.00 is plain
`struct mmap map[]` — restored to v2.00 (non-volatile). All other v2.00 fields
(`pass, msg_line, ecount, msegs, testsel, scroll_start, pass_ticks, total_ticks,
pptr, tptr, erri, pmap[], map[], plim_lower, clks_msec, starth, startl, printmode,
numpatn, test_pages, selected_pages`) match v2.00 ordering exactly. The other
structs (`mmap, pmap, tseq, xadr, err_info, pair`) are verbatim v2.00 — notably
`struct tseq` keeps v2.00's `{short cache; short pat; short iter; short ticks;
short errors; char *msg;}` (the old hack used a different `{sel, cpu_sel, …}`
shape; NOT inherited).

## SCREEN_ADR reconciliation
v2.00 test.h hard-defines `SCREEN_ADR 0xb8000` (x86 text RAM). Our `display.h`
already defines `#define SCREEN_ADR ((unsigned long)vga_buf)`. To keep exactly one
active definition and avoid a `-Wmacro-redefined` warning:
- the two upstream defines (`SCREEN_ADR`, `SCREEN_END_ADR`) are **commented out**
  with the x86 reason, and
- test.h **`#include "display.h"`**, so `SCREEN_ADR` resolves to `vga_buf`.

`SCREEN_END_ADR` was only the byte-extent of the text buffer; display.h does not
provide it. No ported file in the current set references `SCREEN_END_ADR` (it's an
x86-only artifact). If a later wave needs it, add it as `(SCREEN_ADR + 80*25*2)`
in display.h. **TO VERIFY at integration:** no consumer needs `SCREEN_END_ADR`.

## Dropped-`cpu`-arg prototype list
The task brief expected v2.00 test-routine prototypes to carry a trailing
`int cpu`. **They do not** — v2.00 is already single-CPU. Verified against the
actual definitions: `test.c:35 addr_tst1()`, `:136 addr_tst2()`, `:257 movinvr()`,
`:411 movinv1(int iter, ulong p1, ulong p2)`, `:600 movinv32(…)`, `:889
modtst(…)`, `:1075 block_move(int iter)`, and `error.c:429 do_tick(void)` — none
take a `cpu`. So the test.h prototypes for **addr_tst1, addr_tst2, movinv1,
movinvr, movinv32, modtst, block_move, do_tick** are already cpu-free and kept
**verbatim** (no edit required). They match the Wave-4 definitions as-is.

**`bit_fade_fill` / `bit_fade_chk` do not exist in v2.00.** v2.00 has a single
`void bit_fade(void)` (def `test.c:1289`, proto `test.h:152`, caller
`main.c:369`). The split fill/chk pair was an invention of the old attic hack
(`attic/src-v0.01/memtest.h`), which the discipline forbids inheriting. So I kept
the verbatim `void bit_fade(void);` prototype and did **not** add
`bit_fade_fill`/`bit_fade_chk`. **Flag for the lead:** if a later wave's error.c/
test.c was written against the old split API, reconcile to v2.00's single
`bit_fade()` (the faithful choice), not the other way around.

## Open questions / TO VERIFY
- `SCREEN_END_ADR` intentionally not provided (no consumer in scope). Confirm at
  link time.
- `struct eregs;` is a forward decl here; the definition is in lib.c (Wave 2).
  `inter()`'s prototype stays verbatim; whether `inter()`/exception frames get a
  PPC port is a later-wave decision (likely parked — OF owns exceptions).
- The asm `__ASSEMBLY__` guard: head.S does not currently include test.h, but the
  guard + top `E8xx` constants are preserved verbatim in case the asm path ever
  pulls them.
- `NULL` is `#define`d to `0` here (verbatim v2.00). If any substrate/std header
  later also defines `NULL`, watch for a redefinition warning — none today.

## Build impact (what includes what)
- `src/test.h` now `#include`s `src/ppc.h` and `src/display.h`. Both are guarded
  (`PPC_H`, `DISPLAY_H`) so double-inclusion is safe.
- Every ported `.c` that includes `test.h` transitively gets the PPC intrinsics
  and the vga_buf-backed `SCREEN_ADR` — no per-file `#include "ppc.h"` needed.
- No Makefile change (header only). No new objects.
- Depends on the Wave-0 substrate already in `src/` (ppc.h, display.h). No
  circular include: display.h/ppc.h do not include test.h.
