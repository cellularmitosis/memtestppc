# port-error.c — memtest86+ v2.00 `error.c` → memtestppc+ (Wave 4)

## File & purpose
Upstream `ref/memtest86+-2.00/error.c` (548 lines) is the **error reporting +
on-screen error table** module. It owns:
- `error()`, `ad_err1()`, `ad_err2()` — thin entry points the test routines
  (test.c) call when a miscompare is found; they funnel into `common_err()`.
- `update_err_counts()` / `print_err_counts()` — bump `v->ecount` /
  `tseq[].errors`, redraw the count fields, and **paint the failing row red**.
- `common_err()` — the heart: four print modes
  (`PRINTMODE_SUMMARY` / `ADDRESSES` / `PATTERNS` / `NONE`). `ADDRESSES` is the
  default and draws the classic scrolling
  `Tst Pass Failing-Address Good Bad Err-Bits Count Chan` table.
- `print_ecc_err()`, `parity_err()` — ECC / parity reporters.
- `printpatn()` — BadRAM `badram=0x..,0x..` boot-option dump (PATTERNS mode).
- **`do_tick()`** — progress bars (test + pass), confidence score (SUMMARY),
  and the **WallTime HH:MM:SS** display. (Verified: `do_tick` lives in v2.00
  `error.c`, NOT `main.c` — `grep do_tick ref/.../main.c` finds nothing.)

## Disposition — **ported (adapted)**.
The error table is the M1 headline and is reproduced **verbatim** (every
`dprint`/`hprint`/`hprint2`/`cprint` coordinate, both header strings, all four
print modes, the SUMMARY confidence math). Only platform leaves changed:
DMI/ECC/parity/beep (commented N/A), the red-row attr render (PPC explicit
re-blit), and `do_tick`'s rdtsc→PPC-timebase elapsed-time block.

## The verbatim error table (the deliverable)
`common_err()`'s `PRINTMODE_ADDRESSES` branch is byte-for-byte upstream:
- Header (drawn once, `hdr_flag`): `LINE_HEADER` col 0
  `"Tst  Pass   Failing Address          Good       Bad     Err-Bits  Count Chan"`
  and `LINE_HEADER+1` the `--- ----` rule.
- Duplicate suppression (`adr==eadr && xor==exor → return`).
- `check_input(); scroll();` then the row at `v->msg_line`:
  `dprint(,0,test,3,0)` `dprint(,4,pass,5,0)` `hprint(,11,page)`
  `hprint2(,19,offset,3)` `cprint(,22," -      . MB")` `dprint(,25,mb,5,0)`
  `dprint(,31,((page&0xF)*10)/16,1,0)`; data branch
  `hprint(,36,good) hprint(,46,bad) hprint(,56,xor) dprint(,66,ecount,5,0)`;
  ECC (type 3) and parity (type 2) sub-branches kept verbatim too.
- `SUMMARY` mode (the confidence-value screen) and `NONE`/`PATTERNS` kept
  verbatim, including all `LINE_HEADER+n` label coordinates and the per-test
  `"Test Errors"` column. **Nothing in the table was simplified.**

## Red error-row attr + explicit render (the attr-only-render rule)
`print_err_counts()` keeps the upstream `0x47` fill **verbatim** (loops
`pp=(char*)(SCREEN_ADR+v->msg_line*160+1)`, writing attr `0x47` across cols
1..76). On x86 the VGA text hardware re-paints from the attr byte; our `vga_buf`
is a shadow buffer and `cprint→tty_print_line` dedups on the **char** shadow, so
an attr-only poke does NOT auto-render. I added immediately after the fill:
```c
/* PPC: ... re-blit each attr-only cell so the red row appears. */
for (i = 1; i <= 76; i++) {
    fb_render_cell(v->msg_line, i);
}
```
This mirrors the fill exactly (row `v->msg_line`, cols 1..76) and matches how
`init.c` handles its green title bar / reverse footer (HANDOFF "Substrate
contract"). `fb_render_cell` comes from `display.h` (included via `test.h`).

## x86 leaf code commented out (each region + replacement)
- **`#include <sys/io.h>` / `#include "dmi.h"`** — x86 port-I/O + SMBIOS DMI
  headers. Commented. Added `#include "ofw.h"` for `ofw_get_timebase_freq()`.
  (`ppc.h`'s `mftb/mftbu` arrive via `test.h`.)
- **`extern ... dmi_err_cnts[] / dmi_initialized`** — commented (DMI display
  gone).
- **`beep(600); beep(1000);`** in `update_err_counts()` — PC-speaker (PIT
  0x43/0x61). Commented; `beepmode` is forced 0 via `config.h` anyway, so the
  branch was already dead. No PPC replacement (no speaker).
- **`add_dmi_err((ulong)adr);`** in `common_err()` — SMBIOS per-slot tally.
  Commented (no DMI).
- **Two DMI display loops** in `SUMMARY` (the "Errors per Memory Slot" header
  population and the per-slot counts). Commented out as block comments; the
  static labels and the per-test `"Test Errors"` column stay verbatim. The now
  unused locals `x`/`j` get a marked `(void)x; (void)j;` (declarations kept).
- **`print_ecc_err()`** — wrapped `#if 0` (consumes `poll_errors()`/
  controller.c, both N/A). The ECC `type==2/3` branches inside the table stay
  so the layout is intact if ECC is ever wired up.
- **`parity_err()`** — left inside its existing `#ifdef PARITY_MEM` (undef'd in
  config.h → compiles out), per discipline.
- **`do_tick()` rdtsc block** — replaced (see below).
- **`poll_errors();`** call in `do_tick()` — commented (controller.c ECC poll).
- **`USB_WAR` block** in `error()` — left inside its `#ifdef` (undef'd).

## PPC/OF additions
- **Red-row render loop** (above).
- **`do_tick()` PPC timebase elapsed time** — replaces the rdtsc block. Reads
  the 64-bit Time Base via `mftbu():mftb()` with the standard upper-half re-read
  (carry guard), latches a `static` start reference on the first tick, caches
  `ofw_get_timebase_freq()` as the ticks/sec divisor, does the 64-bit subtract
  via two 32-bit halves with a borrow, and divides to seconds. The HH:MM:SS
  digits go to the **identical** `LINE_TIME`/`COL_TIME` positions
  (`COL_TIME, +5,+6,+8,+9`) as upstream, so WallTime renders the same. The
  `do{}while(hi!=hi2)` re-read and a `tb_init` guard avoid a torn 64-bit read
  and avoid re-latching. (Mechanics modeled on `attic/.../main.c`
  `read_tb/elapsed_seconds`, not its structure.) Static-local start ref is the
  brief-sanctioned option; I did **not** repurpose `v->startl/starth` (those are
  the x86 rdtsc latch set in upstream main.c — left alone for the Wave-5 port).

## Features surfaced / shelved
- **Surfaced:** the full `PRINTMODE_ADDRESSES` scrolling table (default), the
  `SUMMARY` confidence screen, `NONE`, `PATTERNS`/`printpatn` BadRAM dump, the
  per-test error column, the red failing-row highlight, the progress bars +
  WallTime.
- **Shelved (N/A):** DMI per-slot attribution, ECC reporting, parity reporting,
  beep. All commented with reasons, layout slots preserved.
- **CPU column:** the `Chan` column is fed by ECC only; on data errors it stays
  blank exactly as upstream. (Note: there is no literal `smp_my_cpu_num()`/CPU
  column in v2.00 `error.c` — that was a v5.01 concern. Single-CPU = no SMP
  primitives here; nothing to comment. If the lead/brief expected a CPU column
  reading 0, v2.00's table simply has no such column — see TO VERIFY.)

## Open questions / TO VERIFY
1. **No SMP / no CPU column in v2.00 `error.c`.** The brief said "comment
   spin_lock/barrier, smp_my_cpu_num()→0, CPU column shows 0." v2.00 `error.c`
   has **none of these** (they are v5.01-isms; HANDOFF already flagged v2.00 is
   pre-SMP). So there was nothing to comment and no CPU column to set to 0. The
   `ADDRESSES` table's last column is `Chan` (ECC channel), not a CPU id.
   Flagging so the lead can confirm the table matches the intended TUI.
2. **`tseq[].errors` is `short`** but `dprint(...,tseq[i].errors,8,0)` prints up
   to 8 digits — upstream behavior, kept verbatim (counts wrap at 32767). Not a
   port bug; noting for fidelity.
3. **`syn`/`chan` statics** are only written by the (now `#if 0`'d)
   `print_ecc_err()`, so the ECC table branch reads stale zeros if ever reached
   — but it can't be reached (no ECC poll). Harmless; kept verbatim.
4. **Timebase start reference timing:** elapsed time starts from the **first
   `do_tick`**, not from `main()` entry. Upstream started from the rdtsc latched
   in `init.c`/`main.c`. A few ms skew at most; if exact main-entry start is
   wanted, Wave-5 main.c can latch `v->startl/starth` (PPC tb halves) and this
   file can read them instead of its static. Flagged for the lead.

## Build impact (globals the Wave-4 checkpoint must provide)
`src/error.c` includes `test.h`, `config.h`, `ofw.h` (+ `ppc.h`/`display.h` via
`test.h`). It references these externs/globals — the checkpoint must define or
stub them:
- **`struct vars * const v`** — owned by main.c (Wave 5); checkpoint stub.
- **`struct tseq tseq[]`** — owned by main.c; needs a terminator
  (`.msg == NULL`) so the SUMMARY/`tseq[i].errors` loops stop.
- **`int test_ticks, nticks, beepmode`** — `extern`'d here; defined in
  init.c (`test_ticks`,`nticks` already there) / main.c (`beepmode`). Provide.
- **`v` struct fields used:** `ecount, ecc_ecount, test, pass, msg_line,
  printmode, total_ticks, pass_ticks, tptr, pptr, msegs, pmap[], numpatn,
  patn[], erri.*` (low_addr/high_addr/ebits/tbits/min_bits/max_bits/maxl/eadr/
  exor/cor_err/hdr_flag). All present in `test.h`'s `struct vars`.
- **Functions called (defined elsewhere, must link):** `cprint, dprint, hprint,
  hprint2, scroll, clear_scroll, check_input` (lib.c ✅); `page_of` (init.c —
  HANDOFF notes the paging family is `#if 0`'d there → **needs a flat-mapping
  stub**, e.g. `page_of(p)=((ulong)p)>>12`, for the checkpoint); `insertaddress`
  (patn.c / config — Wave 6 → **stub returning 0** so PATTERNS links);
  `ofw_get_timebase_freq, mftb, mftbu` (substrate ✅); `fb_render_cell` (display
  ✅).
- **No longer referenced (so no stub needed):** `add_dmi_err, beep,
  poll_errors, dmi_err_cnts, dmi_initialized` (all commented). `poll_errors()`
  is still forward-declared at top (harmless prototype, no call).

**Bottom line:** for the checkpoint, the lead needs `v`, `tseq[]` (terminated),
`beepmode`, and stubs for `page_of` (flat `>>12`) and `insertaddress` (`return
0;`). Everything else resolves against already-ported files.
