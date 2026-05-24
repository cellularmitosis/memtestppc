# port-init_c.md — porting memtest86+ v2.00 `init.c` → memtestppc+

**File:** `src/init.c` (from `ref/memtest86+-2.00/init.c`, 1302 lines, ISO-8859).
**Disposition:** ported. The static-TUI draw is kept byte-verbatim; the x86
hardware-detection leaf (cpuid/rdtsc/PIT/PCI/DMI/PAE-paging) is commented with
reasons and replaced by a PPC PVR + Open Firmware path.

## File & purpose
`init.c` is the one-time startup file. It:
1. `display_init()` — clears `SCREEN_ADR` to blue, paints the green title bar +
   blinking `+`, the reverse-video bottom line (the **static TUI header**).
2. `init()` — kills the floppy motor, turns on cache, calls `display_init()`,
   probes firmware, calls `mem_size()`, sets up `pci`, zeroes `struct vars`,
   draws the Pass/Test/Test#/Testing/Pattern labels, the WallTime status header
   + dashes, the vertical separators, the footer; calls `cpu_type()`,
   `find_controller()`, and `find_ticks()`.
3. `cpu_type()` — x86 CPUID family/model decode → CPU name, `cpuspeed()` (PIT),
   `memspeed()` (rep-movsl) for L1/L2/mem MB/s, rdtsc start snapshot.
4. The x86 PAE 2MB-window paging (`paging_off/on`, `map_page`, `mapping`,
   `emapping`, `page_of`), and the x86 timers `cpuspeed`/`memspeed`/`correct_tsc`.

`init()` is called by `main.c`'s `do_test()` the first time through
(`ref/.../main.c:198`), after `restart()` if relocated. Order in `init()`:
`set_cache(1)` → `display_init()` → firmware probe → **`mem_size()`** →
`pci_init()` → zero vars → cache/memory/chipset labels → `cpu_type()` →
`find_controller()` → status header → `footer()` → `find_ticks()`. Our port keeps
that order; only the x86-only steps are commented.

## The verbatim TUI block + the one title edit
`display_init()` keeps the upstream screen-content draw **byte-verbatim**:
the blue fill of `SCREEN_ADR` (`' '`, attr `0x17`), the title-bar attr loop
(`*pp = 0x20` for `TITLE_WIDTH` cells — note: upstream's comment says "red" but
`0x20` is green-bg/black-fg, the classic memtest green title bar; comment kept as
upstream wrote it), the blink-attr poke (`0xA4`), the reverse-video bottom line
(`0x71`). All status rows in `init()` (Pass/Test/Test#/Testing/Pattern at
`COL_MID`, the `WallTime …` header at `LINE_INFO-2`, the dashes at `LINE_INFO-1`
and `LINE_INFO+1`, the `| ` separators, `footer()`) are kept verbatim at their
exact v2.00 coordinates.

**The one allowed text change** — `Memtest86` → `Memtestppc` — with the
title-width / blinking-`+` math:

| | original | ported |
|---|---|---|
| title string | `"      Memtest86  v2.00      "` | `"      Memtestppc v2.00      "` |
| length | 28 (= `TITLE_WIDTH`) | 28 (= `TITLE_WIDTH`) |
| name cols (0-based) | 6–14 (`Memtest86`, 9 ch) | 6–15 (`Memtestppc`, 10 ch) |
| gap before `v2.00` | 2 spaces (cols 15–16) | **1 space** (col 16) |
| `+` poke column | 15 (`cprint(0,15,"+")`) | **16** (`cprint(0,16,"+")`) |
| blink-attr 2nd poke | `pp += 30` (cell 15) | `pp += 32` (cell 16) |

`Memtestppc` is one char wider than `Memtest86`, so the `+` moves one column
right (15→16) and one of the two spaces before `v2.00` is dropped to hold the
title at exactly 28 chars. The `+` still lands immediately after the name, giving
`Memtestppc+ v2.00`. Verified by counting against the actual v2.00 string (not
guessed). The blink-attr loop pokes the attr of cell 0 **and** the `+` cell so
the `+` blinks (`0xA4` = blink + green bg + red fg).

## display_init() / fb_init() boundary
The upstream function stays named **`display_init()`** and keeps the verbatim
screen draw. At its top it now calls **`fb_init()`** (display.c) — the OF
framebuffer **hardware** bring-up (discover fb, set palette, black the screen),
renamed from the substrate's old `display_init` in Wave 3 specifically so this
upstream `display_init()` keeps its name. If `fb_init() < 0` it prints a message
via `ofw_puts` and returns (nothing to draw on). The x86 cursor-hide (there was
none in v2.00 init.c) is absent; the `serial_echo_init()` / `serial_echo_print()`
VT100 escapes are kept verbatim — they are inert (`serial_echo_init()` only
clears the screen shadow now; `serial_echo_print()` is a no-op stub in lib.c).

## PPC/OF hardware detection (replacing the x86 leaf)
- **CPU identity:** `cpu_type()` reads the PVR via
  `__asm__ __volatile__("mfpvr %0":"=r"(cpu_pvr))`, takes bits 16–31, and looks
  it up in a `ppc_cpu_names[]` table, then `cprint(LINE_CPU, 0, name)` —
  the column where upstream printed the marketing name. The PVR table used:

  | PVR hi | name | L1 KB | L2 KB |
  |---|---|---|---|
  | 0x0001 | PowerPC 601 | 0 | 0 |
  | 0x0003 | PowerPC 603 | 16 | 0 |
  | 0x0004 | PowerPC 604 | 16 | 0 |
  | 0x0006 | PowerPC 603e | 32 | 0 |
  | 0x0007 | PowerPC 603ev | 32 | 0 |
  | 0x0008 | PowerPC 750 (G3) | 32 | 256 |
  | 0x000C | PowerPC 7400 (G4) | 32 | 256 |
  | 0x800C | PowerPC 7410 (G4) | 32 | 256 |
  | 0x8000 | PowerPC 7450 (G4) | 32 | 256 |
  | 0x8001 | PowerPC 7455 (G4) | 32 | 256 |
  | 0x8002 | PowerPC 7457 (G4) | 32 | 512 |
  | 0x8003 | PowerPC 7447A (G4) | 32 | 512 |
  | 0x8004 | PowerPC 7448 (G4) | 32 | 1024 |
  | 0x0039 | PowerPC 970 (G5) | 64 | 512 |
  | 0x003C | PowerPC 970FX (G5) | 64 | 512 |
  | 0x0044 | PowerPC 970MP (G5) | 64 | 1024 |
  | (other) | Unknown PowerPC | 0 | 0 |

  The L1/L2 figures here are only **fallbacks**; OF values override them.
- **CPU clock (MHz):** `ofw_finddevice` over a list of Apple CPU-node paths
  (`/cpus/PowerPC,G3@0`, `…,G4@0`, `…,G5@0`, `…,750@0`, `…,970@0`,
  `/cpus/cpu@0`), then `ofw_getprop("clock-frequency")` / 1e6 → MHz, printed at
  `LINE_CPU` col 20 (`"      MHz"` + `dprint`). Replaces the PIT/rdtsc
  `cpuspeed()`.
- **Caches:** `ofw_getprop("d-cache-size")` → L1 KB and
  `ofw_getprop("l2-cache-size")` → L2 KB, printed in the verbatim
  `"L1 Cache:     K  "` / `"L2 Cache:     K  "` lines at the upstream coords.
  The memspeed MB/s figures are omitted (memspeed is x86 leaf). **L3:** v2.00
  has no L3 line, so to keep the screen exactly we draw none (G3/G4/G5 over OF
  here have no L3 anyway). **Flagged.**

## x86 / PC-module code commented (with reason)
| region | reason | replacement |
|---|---|---|
| `#include controller.h/pci.h/io.h`, `#include defs.h` | PC chipset/PCI/port-IO headers; defs.h body is `#if 0`'d | `#include "ofw.h"` |
| `rdmsr` macro | x86 MSR read (only `correct_tsc` user) | none |
| `extern memsz_mode/firmware/dmi_initialized/dmi_err_cnts` | PC firmware-probe + DMI state | OF `/memory` via `mem_size()` |
| `struct cpu_ident cpu_id` | x86 CPUID state (commented in test.h) | `ulong cpu_pvr` (PVR) |
| `st/end/cal_low/high` | rdtsc snapshot halves for PIT/memspeed | none |
| `memspeed()` proto + body | x86 rep-movsl/stosl/lodsl timed by rdtsc | none (MB/s not shown) |
| `cacheable()` proto + body + call | x86 memspeed+map_page cache-size walk; gated on `v->rdtsc` | "Cached" col left blank (flagged) |
| `cpuspeed()` proto + body | 8254 PIT (0x42/0x43/0x61) + rdtsc | OF `clock-frequency` |
| `outb(0x8,0x3f2)` | kill floppy motor — no PC floppy on Macs | dropped |
| firmware probe (`query_linuxbios/query_pcbios`) | coreboot scan / BIOS E820 | `mem_size()` (OF) |
| `pci_init()` | PCI enum via x86 port I/O | dropped (chipset stays Unknown) |
| DMI `dmi_err_cnts` reset loop | SMBIOS per-DIMM counters | dropped (no DMI) |
| `find_controller()` | PC north-bridge probe over PCI | dropped (Chipset line blank) |
| `if (v->rdtsc){cacheable();…}` block | rdtsc-gated; rdtsc N/A on PPC | the inner `":  :"` cprint promoted out (clock drawn unconditionally) |
| x86 `cpu_type()` body (AMD/Intel/VIA decode, MHz print, memspeed L1/L2/mem, rdtsc start snapshot) | cpuid/rdtsc/memspeed leaf | PVR table + OF props |
| PAE paging block (`paging_off/on`, `map_page`, `mapping`, `emapping`, `page_of`) | x86 CR0/CR3/CR4 2MB-window paging | tested in place (OF claims mapped RAM) |
| `correct_tsc()` | Intel SpeedStep MSR TSC fixup | none (PPC TB invariant) |

Everything is preserved (commented or `#if 0`), not deleted, per the discipline.

## mem_size() — how/where called, memory displayed
`mem_size()` is **not** ours (memsize.c, Wave 3 parallel agent: OF discovery +
claim, populates `v->pmap[]`/`v->msegs`/`v->test_pages`). `init.c` calls it at
the exact upstream call site (after the — now commented — firmware probe) and
displays the total verbatim: `cprint(LINE_CPU+3, 0, "Memory  : ")` then
`aprint(LINE_CPU+3, 10, v->test_pages)`. No memory discovery is done in init.c.

## display_refresh() PPC addition
Two `display_refresh()` calls marked `/* PPC: */`:
1. End of `display_init()` — the title-bar/blink/reverse-line attr-only pokes go
   straight into `vga_buf` attr bytes without changing the char, so
   `cprint`→`tty_print_line` (which dedups on the char shadow) will **not**
   auto-render them. One refresh paints them.
2. End of `init()` — cheap belt-and-suspenders so the fully-assembled static
   screen is on the framebuffer before the test loop runs. (x86 didn't need
   these; VGA hardware repaints from attr, our shadow buffer doesn't.)

## Version-string question (FLAG — user to decide)
Per the brief I did the literal substitution and kept the version as `v2.00`, so
the title reads **`Memtestppc+ v2.00`**. Decision for the user: (1) keep `v2.00`
(advertises the upstream base, honest about provenance), or (2) use the
memtestppc+ project version (e.g. the current `0.01`/next release). I did not
decide. If switching to a project version, only the title string and the `+`
column may need re-checking against `TITLE_WIDTH` (28) if the new version string
differs in length from `v2.00` (5 chars).

## What init.c exports / who calls it
- **Exports (non-static):** `init()` (called by main.c `do_test`),
  `cpu_type()` (forward-declared `static` per upstream; defined non-static —
  verbatim upstream quirk, GCC treats as static + warns), `extclock`,
  `l1_cache`, `l2_cache`, `tsc_invariable`, `cpu_pvr`, `beepmode`.
- **Not exported anymore:** the PAE paging fns (`map_page`/`mapping`/`emapping`/
  `page_of`/`paging_off`) and `memspeed`/`correct_tsc` are `#if 0`'d out —
  **but they are still declared in `test.h` and CALLED by upstream `main.c`**
  (Wave 5). See Build impact.

## Open questions / TO VERIFY
- **OF CPU-node paths:** the path list is from attic identify_cpu + guesses.
  Verify on real hardware which node exists (iBook G3 750CX ≈ PVR 0x0008; the
  user's G5 970 = 0x0039) and that `clock-frequency`/`d-cache-size`/
  `l2-cache-size` are present and 32-bit BE. If the props are absent the table
  fallbacks fill L1/L2 and MHz simply isn't shown.
- **PVR coverage:** 750CX/750FX/750GX and 7445/7447 variants may carry PVRs not
  in the table → "Unknown PowerPC". Expand once seen on hardware.
- **MHz column:** placed at a fixed col 20 (no upstream `off` name-length
  bookkeeping). May overlap a very long CPU name; check rendered output.
- **"Cached" column** (`COL_CACHE_TOP`) is left blank (cacheable() is x86 leaf).
  A PPC cache-able-size probe using the time base could fill it later.
- **set_cache()** is a lib.c stub (no HID0 control yet) — the `set_cache(1)` here
  is inert (already flagged in port-lib_c.md).

## Build impact
- **New deps:** `ofw.{c,h}` (finddevice/getprop/puts), `display.{c,h}`
  (`fb_init`/`display_refresh`), `ppc.h` via test.h. All already in the tree.
- **Makefile:** add `init.o` to `OBJS`.
- **Undefined-until-later symbols init.c references** (expected at the Wave-3
  link; resolve in later waves): `v` (main.c, Wave 5), `tseq[]` (main.c, Wave 5),
  `find_ticks()` (main.c, Wave 5), `mem_size()` (memsize.c, Wave 3 parallel).
  `set_cache/footer/aprint/cprint/dprint` resolve from lib.c (Wave 2).
- **Wave-5 coordination (important):** init.c `#if 0`'d the PAE paging family
  (`map_page`/`mapping`/`emapping`/`page_of`/`paging_off`), but upstream `main.c`
  (`do_test`/`restart`/`compute_segments`) **calls them**. When main.c is ported
  in Wave 5 those call sites must be commented/replaced (in-place testing), OR
  trivial flat-mapping stubs (`mapping(p)=(p<<12)`, `page_of(a)=(a>>12)`,
  `map_page=0`, `paging_off=nop`) must be provided. Recommend stubbing in main.c
  (or a small reloc.c) so the diff in init.c stays "x86 leaf removed". Flagged
  here so Wave 5 doesn't trip on undefined `map_page` etc.
