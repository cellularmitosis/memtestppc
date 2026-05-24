# Port report — `main.c` (Wave 5)

## File & purpose
`ref/memtest86+-2.00/main.c` (607 lines) is the **test driver**: the `tseq[]`
test table, the global test state (`v`, `p/p1/p2/p0`, `segs`, `bail`,
`test_ticks`, `nticks`, `window`), and the engine functions `do_test()` (run one
window of one test + the `switch(pat)` dispatch), `find_ticks()` /
`find_ticks_for_test()` (progress-bar budgeting), `compute_segments()` (turn the
current window ∩ `v->pmap[]` into the `v->map[]` segments the tests iterate), and
`restart()`. On x86 there is **no `main()`** — `head.S` calls `do_test()`
directly, and the test *loop* is the self-relocation: `do_test()` ends by calling
`run_at()`, which `memmove`s the image to a new address and re-enters via
`startup_32`. That serves two purposes: looping, and vacating the RAM about to be
tested.

## Disposition — **ported (adapted)**. The one real structural change in the port.

The whole skeleton is kept: `tseq[]` byte-verbatim, the `do_test()` body and its
`switch(tseq[v->test].pat)` dispatch verbatim, `find_ticks`/`find_ticks_for_test`/
`compute_segments` verbatim, `restart()` verbatim. What changed is **only the
x86 self-relocation machinery**, because PPC tests in place (Key invariant #4):

| upstream (x86) | PPC port |
|---|---|
| `head.S` → `do_test()`; loop = self-relocation re-entry | `head.S` → `main()`; loop = `for(;;) do_test();` in `main()` |
| `run_at()`/`__run_at()` memmove+`goto *p` to relocate | `#if 0`'d; `do_test()` just `return`s to `main()`'s loop |
| `windows[]` = 32-entry PAE table (0..64 GB, 2 MB-granular) | single flat window `{0, 0xffffffff}`; `compute_segments` clamps to `[plim_lower, plim_upper]` |
| `map_page`/`mapping`/`emapping`/`page_of`/`paging_off` = x86 PAE (`init.c`, `#if 0`'d there) | flat in-place versions defined **here** (the upstream `FLAT==1` branch) |
| `parse_command_line()` (boot-protocol cmdline) | `#if 0`'d (OF loads the ELF; no real-mode boot block) |

### Why the single flat window is correct
Upstream's `windows[]`/`map_page` is an **x86 PAE address-aliasing** scheme: it
maps a 2 MB physical slice into a fixed virtual window and relocates the test
code out of the slice under test. On PPC, OF already mapped all claimed RAM 1:1
(MMU on), and we never relocate — so one window spanning all physical memory is
the whole job. `compute_segments(0)` intersects that window with `v->pmap[]`
(set by `memsize.c`) into `v->map[]`, and the test routines (`test.c`) chunk each
segment internally by `SPINSZ` (32 MB). `window++` wraps to 0 after each test
(`sizeof(windows)/sizeof(windows[0]) == 1`), so `do_test()` runs each test once
over all memory, then `v->test++`. Verified in QEMU: "Testing: 8192K - 252M
236M" — the full claimed range in a single pass.

### Flat paging primitives (the `FLAT==1` branch, moved here)
`init.c` carries the upstream x86 PAE `map_page`/`mapping`/`emapping`/`page_of`/
`paging_off` but wraps them in `#if 0` (they program `%cr3`, alias windows, etc.
— all N/A). The watch-list said Wave-5 must either comment the call sites or
provide flat stubs; I provide the **flat versions** here (they are declared in
`test.h`, so defining them in `main.c` resolves the link):
- `mapping(page) → (void*)(page << 12)` — page number → byte address.
- `emapping(page) → mapping(page-1) + 0xf00` — **verbatim upstream `emapping`**
  over flat `mapping`: the end pointer is 0xf00 into the last (exclusive) page.
- `page_of(addr) → ((ulong)addr) >> 12` — byte address → page number.
- `map_page(page) → 0` (always mapped) / `paging_off() → no-op`.

These are exactly what upstream computes when its compile-time `FLAT` is 1, so
the `v->map[]` boundaries and the `do_test()` range display are bit-identical to
a FLAT x86 build.

## x86 leaf code commented / `#if 0`'d (each region + replacement)
- **`__run_at()` / `run_at()`** — image memmove + `goto *p` relocation. `#if 0`
  (block preserved). Replaced by `main()`'s loop + `do_test()` `return`.
- **`parse_command_line()` + `MK_PTR`/`OLD_CL_*` macros** — Linux old-boot-protocol
  cmdline via real-mode `INITSEG` pointers → `serial_console_setup`. `#if 0`
  (block preserved); the `do_test()` call site is commented. N/A: OF loads the
  ELF, the serial leaf is commented in `lib.c`.
- **`windows[]` 32-entry PAE table** — `#if 0` (preserved); replaced by the
  1-entry flat window above.
- **`#if (LOW_TEST_ADR > 640K) #error` guard** — commented (LOW_TEST_ADR is an
  x86 reloc constant from the `#if 0`'d `defs.h`).
- **`do_test()` firsttime block** — kept `init()`; commented the relocation
  guard (`if (&_start != LOW_TEST_ADR) restart();`) and the window-bound seeding
  (`windows[0].start`/`high_test_adr`/`windows[1].end`), all reloc-only.
- **`do_test()` "Relocated" branch** (`if (&_start > LOW_TEST_ADR) … v->map[0].start
  = mapping(v->plim_lower); cprint " Relocated"`) — commented; we always take the
  "not relocated" path (`cprint "          "`), kept verbatim.
- **`do_test()` end-of-window relocation** (`if (window != 0) { run_at(high/low) }`)
  — commented; with a single window this branch is structurally unreachable
  (window always wraps to 0). The `else`/`skip_test`/`bail_test` path is verbatim.
- **`run_at(LOW_TEST_ADR)` at the end of `do_test()`** — commented; replaced by
  `return;`.
- **`restart()` `run_at(LOW_TEST_ADR)`** — commented; the screen-clear loop is
  kept verbatim. (`restart()` has no caller yet; config-menu "Restart" is Wave 6.)
- **`TEST_TIMES` block** — left under `#ifdef TEST_TIMES` (undef'd at top, as
  upstream), so the rdtsc timing code compiles out. Kept verbatim.
- **`extern void bzero();`** — commented (unused; not in our `lib.c`).

## PPC/OF additions
- `int main(void) { for(;;) do_test(); }` — the test loop (head.S calls `main`).
- Flat `map_page`/`mapping`/`emapping`/`page_of`/`paging_off` (above).
- `adj_mem()` **stub** (no-op) — `config.c` (Wave 6) owns the real address-range
  logic; `memsize.c` already set `selected_pages = test_pages`, which is what
  `adj_mem()` computes with no range restriction. (Watch-list item.)
- `insertaddress()` **stub** (`return 0`) — `patn.c` (Wave 6) owns the BadRAM
  pattern table; `error.c`'s PATTERNS mode calls it, but the default print mode
  is ADDRESSES, so the path isn't taken. Stub keeps the link resolved.

## Single-CPU / SMP
Nothing to strip — v2.00 `main.c` is already SMP-free (no `int cpu`/`int me`, no
barrier/AP scheduling). `v->testsel` (single-test selection) is honored verbatim.

## Default-pass behavior (important, verbatim)
`DEFTESTS == 9` and the pass-complete check is `if (v->test >= 9 || v->testsel >=
0)`. `tseq[]` has 10 real entries (indices 0-9); index 9 is the **90-minute
bit-fade**. So a normal pass runs tests **0-8 only** — bit-fade is *not* run
unless the user selects it (`testsel`). This is upstream v2.00 behavior, kept
verbatim, and it means **the flagged ~90-min bit-fade dwell does not occur in a
default run** (the QEMU checkpoint completes passes without hitting it). The
`v->testsel = -1` default and all other `v->` init live in `init.c` (Wave 3),
called once from `do_test()`'s firsttime block — `main.c` sets no `v->` defaults
itself, matching the upstream split.

## Open questions / TO VERIFY
0. **✅ RESOLVED (a `test.c` bug, not main.c): reverse-pass `do_tick` spin.**
   When the loop first drove real memory, test #2 spun forever and the progress
   %s overflowed. On-screen debug confirmed main.c's accounting is correct
   (`msegs=2 segs=2 chunks=8 test_ticks=112`); per-phase counters showed the
   **reverse pass** spinning. Root cause in `test.c` `movinv1`/`movinv32`: the
   underflow guard `if (pe - SPINSZ < pe)` relied on pointer-underflow
   wraparound (UB), which gcc -O1 folds to always-true, so for a segment whose
   start is below the SPINSZ stride (our 8 MB seg0) `pe` wrapped past the bottom
   and never hit `pe <= start`. Fixed with an underflow-safe byte-distance check.
   Tests now cycle with sane %s + `Errors: 0` (`wave5-tests-cycling.png`). See
   `port-test_c.md` fix note + HANDOFF progress log.
1. **`v->test >= 9` literal vs `DEFTESTS`** — upstream hard-codes `9` here (not
   `DEFTESTS`); kept verbatim. If `DEFTESTS` is ever changed, this literal must
   track it. Noted, not changed.
2. **`compute_segments` `int win` loop in `find_ticks`** — `for(j=0; j<
   sizeof(windows)/sizeof(windows[0]); j++)` compares signed `j` to an unsigned
   `size_t` (==1). Harmless (one iteration); a `-Wsign-compare` may appear but
   did not in the `-Wall` build. Verbatim upstream shape.
3. **G5 (`#address-cells=2`)** — unchanged from the memsize.c caveat: the flat
   `mapping(page) = page<<12` assumes a 32-bit physical address (page <
   0x100000). True for G3/G4/QEMU (≤ a few GB); a >4 GB G5 needs revisiting. Not
   a regression — inherited from the substrate.

## Build impact
- `src/main.c` includes `test.h` + `defs.h` (verbatim upstream includes;
  `ppc.h`/`display.h` arrive via `test.h`). Makefile dependency line updated
  accordingly. `main.o` was already in `OBJS`.
- **Globals defined here** (owned by main.c upstream): `tseq[]`, `firsttime`,
  `cmdline_parsed`, `variables`/`v`, `p/p1/p2/p0`, `segs`, `bail`, `test_ticks`,
  `nticks`, `high_test_adr`, `window`, `windows[]`. `beepmode` is **not** defined
  here (owned by `init.c` — confirmed by grep; redefining would collide).
- **Symbols resolved for the link** that other files needed: `mapping`,
  `emapping`, `page_of`, `map_page`, `paging_off` (called by `compute_segments`/
  `do_test`/`error.c`), `find_ticks` (called by `init.c`), `adj_mem` (called by
  `memsize.c`), `insertaddress` (called by `error.c`). After adding the
  `insertaddress` stub the full image links (11 objects); only the known-benign
  GNU-stack / RWX-segment linker warnings remain.
- **QEMU-verified** (mac99, 256 MB): boots to the v2.00 TUI and **runs the test
  loop** — progress bars advance, "Test #N" cycles, "Testing: 8192K - 252M"
  (full claimed range), WallTime increments, `Errors: 0` on good RAM. Evidence:
  `wave5-tests-running.png`.
