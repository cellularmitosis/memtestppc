# Session 006 — plan: port-fidelity restructure (M1)

**Status:** research complete, plan drafted, *not yet started on code*. This file
is the working plan; it will fold into `HANDOFF.md` at closeout.

## Goal

Bring `src/` structurally close to upstream **memtest86+ v5.01** so the diff
against upstream is small and legible. Per CLAUDE.md invariant #1 / PLAN.md M1:
preserve upstream's **function skeletons, variable names, file boundaries, and
layout constants**; replace only x86-specific *leaf* code with PPC/OF
equivalents; prefer commented-out original over deletion where the replacement is
non-obvious.

This is a *restructure*, not a rewrite: behavior should stay green (QEMU + iBook)
at every stage. The payoff is that upstream bugfixes become pullable and someone
who knows memtest86+ can navigate our tree.

## How this plan was researched

Read in full: the port's `src/{main,display,test,ofw}.c`, `src/{memtest,display,ofw}.h`;
upstream `error.c`, `screen_buffer.{c,h}`, `test.h`, `config.h`; and the canonical
sections of upstream `main.c` (tseq, `next_test`, `set_defaults`, `test_start`,
`test_setup`, `do_test`, `find_chunks`, `find_ticks_for_pass/_test`,
`compute_segments`), `lib.c` (the print primitives + `scroll`/`clear_scroll`),
and `init.c` (`display_init`+`init`). A subagent did a line-level `test.c`
comparison and located the static-header draw.

**Gotcha worth recording:** upstream `init.c` is ISO-8859 encoded (it has a raw
`°C` byte). Plain `grep` treats it as binary and **silently skips all matches** —
that is why the header strings looked "missing." Always `grep -a` the upstream
tree, and use the Read tool (which decodes) rather than `grep`/`sed` when in
doubt.

## Where the static header actually lives (was unclear at session start)

The green title bar, `Pass/Test %` labels, cache lines, `| Time:` clock, vertical
separators, and `---` rules are drawn by upstream **`init()` + `display_init()`
in `init.c`** (`display_init` at init.c:137, `init` at init.c:186; called from
main.c:425). The port's hand-written `draw_screen()` (src/main.c:388) is the
direct analog. The error-row column header is *not* part of this static draw —
upstream draws it lazily on the first error inside `common_err`
(error.c:313-320).

---

## Current-state structural map (port ⇆ upstream)

| Concern | Port location | Upstream home | Verdict |
|---|---|---|---|
| Print primitives `cprint/cplace/dprint/hprint/hprint2/aprint/xprint/scroll/clear_scroll/footer` | `display.c` (mixed with FB backend) | `lib.c` (+ `footer` in lib.c) | Faithful logic, **wrong file**. `dprint` right-justify branch was simplified — re-align to upstream exact. Missing `hprint3`. |
| Framebuffer + font rendering (`fb_*`, `display_render_cell`, palette, `display_init/refresh`) | `display.c` | *(none — this is the PPC leaf replacing VGA text HW + 0xb8000 autorender)* | Correct as a backend; should be its own file, the "io" of this port. |
| `vga_buf[]` = the 80×25 char/attr buffer | `display.c`, `#define SCREEN_ADR vga_buf` | `SCREEN_ADR 0xb8000` (test.h) | **Faithful key move.** Keep. |
| Error path `error/ad_err1/ad_err2` | `main.c` (custom, simplified) | `error.c` (`common_err` + `PRINTMODE_*`) | **Biggest divergence.** Custom row layout & header; no `common_err`, no `err_info` accumulation. |
| `do_tick` | `main.c` (local statics, full bar redraw) | `error.c` (uses `v->tptr/v->pptr`, incremental bars) | Diverges; belongs in `error.c`. |
| Static header draw | `main.c draw_screen()` | `init.c display_init()+init()` | Analog exists; line/col mostly match (see below). |
| CPU detect | `main.c identify_cpu()` | `init.c init()` (calls `get_cpuid` in cpuid.c) | PPC leaf; lives with `init`. |
| Memory discovery / claim | `main.c discover_memory()` | `init.c`/`mem_size()` (memsize.c) | PPC/OF leaf; reasonable. |
| Main loop | `main.c main()` | `main.c test_start()` | Port is a clean single-CPU collapse, but uses standalone globals instead of `v->` fields and a custom tick calc. |
| `find_ticks_for_test`/`calc_pass_ticks` | `main.c` (hardcoded switch) | `main.c find_ticks_for_pass/_test/find_chunks` | Rewritten; re-derive from upstream. |
| `setup_segments` | `main.c` | `compute_segments` | Single-window collapse of `compute_segments`. |
| RNG `rand_seed/rand_next` | `test.c` | `random.c` | **Wrong file** — move to a `random.c` analog. |
| Test routines | `test.c` | `test.c` | Mostly faithful (asm→commented-C). 3 routines need re-aligning (below). |
| `struct vars`, layout `#define`s, `struct tseq/mmap/pmap/err_info` | `memtest.h` | `test.h` | Port dropped `v->` fields it still needs; `tseq.cpu_sel` values changed. |
| OF client interface | `ofw.{c,h}` | *(none — replaces BIOS/`io.h`)* | PPC leaf, fine. |

### Files to add (mirror upstream), files to keep

- **add `error.c`** — `error/ad_err1/ad_err2/common_err/update_err_counts/print_err_counts/do_tick`.
- **add `init.c`** — `display_init()` + `init()` (header draw, CPU detect, memory
  discovery). Pulls `draw_screen/identify_cpu/discover_memory` out of `main.c`.
- **add `lib.c`** — the print primitives + `strlen/memset/memcpy/memmove`.
- **add `random.c`** — `rand_seed/rand_next` (the MWC generator).
- **rename `display.c` → framebuffer backend** (`display.c` is fine as a name; it
  becomes purely the FB/font/palette leaf + `vga_buf` + `display_render_cell`).
- **`main.c`** shrinks to: globals, `tseq[]`, `next_test`, `set_defaults`,
  `test_setup`, the main loop (named `test_start` or kept as `main`), `do_test`,
  `find_ticks_*`, `compute_segments`, `main()` bootstrap.
- **keep** `head.S`, `ofw.{c,h}`, `font_vga.h`, `linker.ld`, `memtest.h`(→ align to `test.h`).
- **out of scope / do not port:** `config.{c,h}` menu, `screen_buffer.c` (serial
  mirror — we have no serial console), `smp.*`, `reloc.c`, `controller.c`,
  `dmi.c`, `spd.c`, `patn.c` (badram), `extra.c`. These are deliberately dropped
  (PLAN.md non-goals). Note them as commented stubs only where a call site would
  otherwise dangle.

---

## Detailed divergence inventory

### A. Error path (highest value)
Port `error()` (main.c:177) invents a layout: header `" Address Expected Actual
Xor Tst Pass"` (drawn statically in `draw_screen`, main.c:440) and a row of
`hprint(adr)@1, good@15, bad@27, xor@39, dprint test@51, pass@55`.

Upstream `common_err` `PRINTMODE_ADDRESSES` (the default printmode, error.c:307)
draws, lazily on first error:
- header (error.c:315): `"Tst  Pass   Failing Address          Good       Bad     Err-Bits  Count CPU"`
- row: `test+1@0, pass@4, page@11, offset@19, " -      . MB"@22, mb@25,
  frac@31, good@36, bad@46, xor@56, ecount@66, cpu@74`.
- de-dups consecutive identical errors (`adr==eadr && xor==exor`).
- `print_err_counts()` (error.c:114) updates the count at `(LINE_INFO,72)` and
  paints the row red by writing attr `0x47` across cols 1..76.

**Plan:** create `error.c` with `common_err` + the `PRINTMODE_ADDRESSES` branch
verbatim (drop `PRINTMODE_SUMMARY/PATTERNS/NONE`, or keep `SUMMARY` later). cpu
col = 0. The session-005 "render each red cell" loop is the faithful PPC version
of `print_err_counts`'s 0x47 fill (our shadow buffer needs explicit render) —
move it into `print_err_counts`. Drop the custom static error header from
`draw_screen`. This makes the error display match the v5.01 screenshot
(`ref/photos/Memtest86+_memory_errors.png`).

### B. do_tick
Adopt upstream `error.c:461` shape: incremental bars via `v->tptr`/`v->pptr`
(only draw new `#`s), `dprint(2,COL_MID+4,pct,3,0)` etc., spinner via
`spin_idx[]`/`cplace`. Replace the x86 `rdtsc` elapsed-time block (error.c:585)
with our timebase `update_time()` leaf. Move `do_tick` into `error.c`.

### C. Print primitives → lib.c
Move `cprint/cplace/dprint/hprint/hprint2/aprint/xprint/scroll/clear_scroll/footer`
+ `strlen/memset/memcpy/memmove` into `lib.c`. Re-align `dprint`'s `right` branch
to upstream's exact loop (lib.c:344-367) — the port simplified it. Add `hprint3`
(used by some upstream paths). `cprint` keeps calling the FB backend
(`display_render_cell`) where upstream calls `tty_print_line`; that's the leaf
swap. `scroll/clear_scroll` already match upstream + our render step.

### D. struct vars / globals → use v->
Port uses standalone globals `v_msg_line, test_ticks, nticks` and never uses the
`v->msg_line/tptr/pptr/pass_ticks/total_ticks` fields that already exist in the
struct. Upstream uses `v->`. Re-align: drop `v_msg_line` for `v->msg_line`, use
`v->tptr/v->pptr/v->pass_ticks/v->total_ticks`. Add back `set_defaults()`
(main.c:118) to initialize `v->erri.*` and the rest — the port inlined a partial
version into `main()`. Restore `err_info` fields used by the error path.

### E. tseq table
Port set every `cpu_sel` to `1` (memtest.h tseq). Upstream uses `-1/32/1` to
encode "round-robin all CPUs / N CPUs / single CPU". For a single-CPU port the
*values* are inert, but fidelity says keep upstream's literals and let the
single-CPU collapse in `next_test`/the loop treat them as "run once". Keep
upstream's `{1,-1,..}` etc. and document that `cpu_sel` is vestigial here.

### F. find_ticks
Re-derive `find_ticks_for_pass/_test` + `find_chunks` from upstream
(main.c:1043-1187) instead of the port's hardcoded `switch`. With single CPU,
`act_cpus==1`, so the `/act_cpus` and `*act_cpus` terms simplify out — keep them
written (as `/1`) per the "comment-out, don't delete" rule, or reduce with a
one-line note. Reconcile `SPINSZ`: port uses `0x400000`, upstream `0x4000000`;
this feeds the tick math, so pick deliberately and document.

### G. test.c re-alignment (from the subagent audit)
Mostly faithful (every asm block → upstream's commented-out C). Three to fix:
1. **addr_tst1** — port dropped upstream's inner `for(i=0;i<50;i++)` repeat on the
   second bank (up test.c:140). Restore.
2. **movinvr** — most diverged: port rolls its own seed (`mftb()`+fixed XOR) and
   embeds the RNG. Upstream seeds from pass/tsc and uses `random.c`. Re-align:
   move RNG to `random.c`, restore the seed scheme (tsc→timebase leaf).
3. **block_move** — port uses plain `<<|>>` rotate + `memcpy`; upstream uses
   rotate-*through-carry* (`rcll`) + `rep movsl`. Verify the carry-rotate
   semantics match before declaring faithful; keep memcpy as the leaf but match
   the pattern rotate.
Also: `movinv32` reuses loop var `i` where upstream used `n` (harmless naming
drift — optional).

### H. draw_screen vs init()/display_init() — known intentional diffs
The port already matches upstream's right-column labels and `| Time:` exactly.
Differences to *decide on* (some are session-005 user choices):
- Title `"Memtestppc+ v0.01"` vs `"Memtest86  5.01"` — **keep** (project identity).
- Two-line `Memory:`/`Testing:` readout (session 005) vs upstream's single
  `Memory:` + L3 line — user-approved divergence; keep but note "Testing" appears
  twice (flagged in 005).
- Lines 7-9: port `State:/Running` + `Pass/Errors` vs upstream
  `Core#/State/Cores...Pass...Errors`. Single-CPU collapse; keep simplified but
  align column positions where free.

---

## Staged execution plan (each stage ends green in QEMU)

Build/verify loop per stage = rsync → `make clean && make memtestppc.iso` on
opti7050 → QEMU mac99 256MB VNC screenshot (see CLAUDE.md → Build & test).
Commit after each green stage so regressions bisect cleanly.

1. **Mechanical file splits, zero behavior change.** Extract print primitives +
   mem* into `lib.c`; extract RNG into `random.c`; move `draw_screen/identify_cpu/
   discover_memory` into `init.c` as `init()`/`display_init()`; leave `display.c`
   as the FB backend. Wire up `Makefile`. Verify identical screen. *(Low risk;
   pure relocation. Do first to de-risk later diffs.)*
2. **struct vars / globals re-align (D, E).** Switch to `v->` fields, add
   `set_defaults()`, restore `err_info`, restore upstream `tseq` literals. Verify
   progress bars + pass counter still advance. *(Medium.)*
3. **find_ticks re-derive (F).** Replace hardcoded switch with upstream
   `find_ticks_for_pass/_test/find_chunks`. Verify bar percentages look sane over
   a full pass. *(Medium — easy to get tick totals wrong; compare bar fill rate
   before/after.)*
4. **error.c restructure (A, B).** New `error.c` with `common_err`
   (`PRINTMODE_ADDRESSES`), `print_err_counts` (incl. the red-fill), `do_tick`.
   Drop the custom static error header. **Verify with a temporary fake-error
   injection** (re-use the session-005 technique; remove before commit) and
   screenshot against `ref/photos/Memtest86+_memory_errors.png`. *(Highest risk +
   highest value; do after the plumbing above is stable.)*
5. **test.c re-alignment (G).** Fix addr_tst1 50× loop, movinvr seed/RNG,
   block_move rotate. Verify a clean full pass still reports 0 errors and timings
   are reasonable. *(Medium.)*
6. **main.c skeleton tidy.** Rename/reshape `main()` toward `test_start()` shape;
   reconcile `setup_segments` ⇆ `compute_segments` naming; comment-out (not
   delete) the SMP/reloc/window scaffolding with a one-line "single-CPU port:
   N/A" so the skeleton still reads like upstream.
7. **Hardware verify on ibookg32.** Deploy ELF via the uranium hop (partition
   boot `boot hd:5,memtestppc.elf`); confirm header, bars, and — critically — the
   restored error rows on the real 8-bit display. Re-burn CD only if the user
   wants a new release artifact.

Stages 1-3 are safe plumbing; 4 is the headline. If time is short, 1+4 alone
deliver most of the fidelity win.

## Decisions needed from the user (prose, since these shape scope)

1. **How far on fidelity vs. the deliberate simplifications?** Do you want the
   commented-out SMP/reloc/window scaffolding *kept* in `main.c` (max
   skeleton-fidelity, more visual noise) or *removed* (cleaner, smaller file but a
   bigger structural diff from upstream)? Invariant #1 leans toward keeping it
   commented; confirm.
2. **Error display:** adopt upstream's full `PRINTMODE_ADDRESSES` row
   (page/offset/MB + good/bad/err-bits/count/cpu) even though it's denser than the
   port's current simple row? (I recommend yes — it's the whole point of M1 and
   matches the reference photo.)
3. **Keep the session-005 TUI choices** (the `Memtestppc+ v0.01` title and the
   two-line Memory/Testing readout) through the restructure, or revert toward
   upstream's exact `Memtest86 5.01` + single `Memory:`+`L3` layout? I assume
   keep; confirm.
4. **`SPINSZ`** — keep the port's `0x400000` or restore upstream `0x4000000`?
   Affects chunking + tick math; I lean toward matching upstream and re-tuning
   only if a pass feels too long on real hardware.

## Out of scope for this session
Config menu, serial console, SMP/relocation, DMI/SPD chipset probing, badram
pattern output, the carried-over `sleep()` stub for the bit-fade test (test 11),
and the empirical 8-bit BGR palette order (tracked in memory + PLAN.md). The
`movinv32` uninitialized-`k` warning can be fixed opportunistically during the
test.c stage.
