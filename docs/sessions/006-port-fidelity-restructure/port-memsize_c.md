# port-memsize_c.md — memtest86+ v2.00 `memsize.c` → memtestppc+

**File & purpose.** `memsize.c` is memtest86+'s memory-discovery module. On x86,
`mem_size()` (ref lines 36–78) turns BIOS-supplied memory information into the
tester's physical map `v->pmap[]`: it snapshots `mem_info` (the table the
bootloader/`setup.S` fills from the BIOS), dispatches on `memsz_mode`
(`SZ_MODE_BIOS` / `_BIOS_RES` / `_PROBE`), sorts the map (`sort_pmap`), sets the
test limits (`v->plim_lower/upper`), calls `adj_mem()` to derive
`v->selected_pages`, and prints the reserved-page count. The map-building helpers
are: `memsize_820`/`sanitize_e820_map` (the E820 path), `memsize_801` (E801/E88
fallback), `memsize_linuxbios` (coreboot tables), and `memsize_probe`/`check_ram`
(a destructive write-readback scan). All of those consume PC-firmware artifacts
or poke raw physical addresses.

**Disposition: ported (body replaced with OF discovery).** The function skeleton,
units, post-processing (`sort_pmap` → `plim_*` → `adj_mem` → `aprint`) and
`sort_pmap()` itself are preserved verbatim. Every BIOS/probe leaf is commented
out with a reason and replaced by a single OF pass, `memsize_ofw()`.

## What `mem_size()` now does
1. Zeroes `v->reserved_pages` and `v->test_pages` (verbatim).
2. **`memsize_ofw()` (PPC, new):**
   - `ofw_finddevice("/memory")` (fallback `"/memory@0"`); read the `"reg"`
     property as `ulong` (addr,size) pairs — on 32-bit PPC each cell is one
     `ulong`, so we walk `reg[]` two cells at a time.
   - Skip the low region (`< LOW_TEST_ADR = 0x800000`, 8 MB); the skipped pages
     are added to `v->reserved_pages`.
   - `ofw_claim()` the rest in **1 MB chunks** (`CLAIM_CHUNK = 0x100000`).
     Successful adjacent claims coalesce into a `v->pmap[]` segment; a failed
     claim (OF already owns it → returns `(void*)-1`) ends the run and counts the
     chunk as reserved. This is the proven mechanic from the old
     `attic/src-v0.01/main.c::discover_memory()`, re-expressed inside v2.00's
     structure.
   - Sets `v->msegs`, sums `v->test_pages`, and also sets
     `v->selected_pages = v->test_pages` (see note).
3. Prints `"OFW Map "` at `LINE_INFO,COL_MMAP` (the cell upstream's
   `SZ_MODE_PROBE` wrote `" Probed "` into).
4. `sort_pmap()`, `v->plim_lower=0`, `v->plim_upper = v->pmap[v->msegs-1].end`,
   `adj_mem()`, `aprint(reserved_pages)` — all verbatim.

## `v->` fields populated (UNITS — all page counts are 4 KB pages)
| field | value | unit |
|---|---|---|
| `v->pmap[i].start/.end` | claimed-region bounds, `addr >> 12` | 4 KB **page number** |
| `v->msegs` | number of coalesced segments | count |
| `v->test_pages` | Σ`(end-start)` over segments | 4 KB pages |
| `v->selected_pages` | = `test_pages` (whole map selected) | 4 KB pages |
| `v->reserved_pages` | low-region + claim-failed chunks | 4 KB pages |
| `v->plim_lower` / `v->plim_upper` | `0` / last seg `.end` | 4 KB pages |

Units match upstream's BIOS paths exactly, so init.c's `aprint`, main.c's
windowing, and config.c's address-range all see the identical shape.

## x86 leaf code commented out (with replacement)
- **`mem_info` snapshot** (first-call copy of E88/E801/E820) — commented; OF's
  device tree is re-read each call instead. This also removes the only reference
  to the `extern struct mem_info_t mem_info` symbol, which has no producer on
  PPC (it was `setup.S`'s job).
- **`switch(memsz_mode)` dispatch** → single `memsize_ofw()` call.
- **`memsize_bios`** (PCBIOS vs LinuxBIOS chooser) — commented (block).
- **`memsize_820` + `sanitize_e820_map`** — commented; `sanitize_*` is in `#if 0`
  (large, references the commented `e820[]` scratch). OF hands us a clean,
  non-overlapping map, so no sanitization is needed. The 640K–1M PC hole
  (`RES_START..RES_END`) skip is replaced by the `LOW_TEST_ADR` skip.
- **`memsize_801`** (E801/E88 fallback) — commented (block).
- **`memsize_linuxbios`** (coreboot tables) — commented (block). Its pmap math
  (`addr>>12`, `test_pages +=`) is exactly what `memsize_ofw` reproduces.
- **`memsize_probe` + `check_ram`** — `#if 0` (they reference `set_cache`, `_end`,
  `p/p1/p2`). The destructive write-readback scan is both unsafe under OF's MMU
  (writing unmapped pages faults) and redundant (OF enumerates RAM
  authoritatively). `check_ram`'s logic is platform-neutral but has no caller
  once `memsize_probe` is gone, so it's parked with it.
- **Globals:** `alt_mem_k`, `ext_mem_k`, `e820[E820MAX]` scratch — commented.

`sort_pmap()` is platform-neutral (an insertion sort on `pmap`) and kept verbatim
and live.

## Low-memory skip rationale
We skip `< 0x800000` (8 MB), matching the proven attic port. Real Apple OF runs
with the **MMU on** and keeps the loaded ELF image, OF itself, the OF/exception
stacks, and framebuffer-related structures in low memory; claiming/writing there
would fault or corrupt OF. This is the PPC analog of upstream's `RES_START`
(0xa0000)…`RES_END` (0x100000) BIOS/video hole — same intent, larger reservation
because OF + our image occupy more than the legacy 640K–1M window. **TO VERIFY:**
8 MB is inherited, not measured; the exact OF/image footprint on the iBook G3 and
on a G5 could be smaller or larger. It is conservative (loses a little testable
RAM) rather than dangerous. The skipped pages are now counted into
`reserved_pages` so the displayed reserved total is honest.

## Globals this file defines / needs
- **Defines** (these were defined *only* in memsize.c upstream; init.c/config.c
  reference them via `extern`): `short e820_nr = 0`, `short memsz_mode =
  SZ_MODE_PROBE`, `short firmware = FIRMWARE_UNKNOWN`. Kept defined at neutral
  values so later-wave `extern` references resolve and config.c's "Memory Sizing"
  menu sees an empty BIOS map (`e820_nr == 0`).
- **Needs:** `ofw_finddevice`/`ofw_getprop`/`ofw_claim` (ofw.h, present);
  `v`, `struct pmap`, `MAX_MEM_SEGMENTS`, `cprint`, `aprint`, `memmove`,
  `sort_pmap` (test.h / lib.c, present); **`adj_mem()`** — declared in test.h but
  **defined in config.c (Wave 6)**. At the Wave-3 checkpoint `adj_mem` will be an
  undefined symbol unless the lead stubs it; since `memsize_ofw` already sets
  `selected_pages`, a no-op stub is harmless. Includes `ofw.h` (new include for
  this file).

## Open questions / TO VERIFY
- **8 MB low skip** — see above; verify against real OF footprint, tune if RAM is
  needlessly lost or if OF claims regions above 8 MB.
- **Claiming ALL memory at init — does it leave room for us?** Our stack lives in
  BSS (head.S), inside the loaded image, which sits *below* `LOW_TEST_ADR` and is
  therefore **never claimed/tested** here — so claiming everything from 8 MB up
  does not eat our stack or code. **Risk to watch:** if OF placed anything we
  depend on (e.g. its own callbacks, the CIF) *above* 8 MB, claiming it could be
  a problem — but `ofw_claim` *failing* on OF-owned chunks (we skip those) is the
  built-in guard. Real-hardware run should confirm we don't claim something OF
  still needs. The old attic port used this exact approach successfully on the
  iBook G3, which is the main evidence it's safe.
- **`reg` cell width.** Assumed 1 `ulong` per addr and per size (32-bit PPC,
  `#address-cells=1`/`#size-cells=1`). True on G3/G4 Macs and QEMU mac99. A G5
  with `#address-cells=2` would change the stride — **TO VERIFY** on G5; not a
  Wave-3 concern (G5 isn't the bring-up target).
- **`memsz_mode`/`firmware` type.** Upstream config.c declares `extern char
  memsz_mode` while init.c (and we) use `short`. This is an upstream
  inconsistency; resolve when porting config.c (Wave 6). Not a memsize.c bug.

## Build impact
- New source file `src/memsize.c`; add to `OBJS` in the Makefile.
- New include of `ofw.h` from memsize.c.
- Defines `e820_nr`/`memsz_mode`/`firmware` (canonical defs; no collision —
  everyone else uses `extern`).
- No longer references `mem_info` (the x86-only, producer-less symbol).
- **Wave-3 link dependency:** `adj_mem()` is not defined until Wave 6 (config.c).
  Provide a stub (or `#if 0` the `adj_mem()` call) for the Wave-3 checkpoint;
  `selected_pages` is already set by `memsize_ofw`, so a no-op stub is correct.
- Syntax-checked locally with host `gcc -fsyntax-only -I src` (clean). Cross
  compile/link is the lead's Wave-3 checkpoint on opti7050.
