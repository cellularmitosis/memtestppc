# Session 006 — plan: port-fidelity restructure (M1)

> **⚠ SUPERSEDED IN PART (2026-05-24).** This plan was written assuming a **v5.01**
> base. The session then pivoted to a **v2.00 base** with an *import-everything,
> examine-every-file* discipline. For the current approach read
> [`HANDOFF.md`](HANDOFF.md) + [`PORTING-GUIDE.md`](PORTING-GUIDE.md). The parts of
> this doc that remain valid and useful: the **rendering architecture**
> (ttyprint→framebuffer), the **divergence inventory** (display/error/do_tick/
> struct-vars analysis), and the general staging philosophy. Ignore the v5.01-base
> assumptions, the SMP-commenting discussion (v2.00 has no SMP), and the "files we
> don't copy" framing (we now import everything).

**Status:** research complete; plan revised after user direction on fidelity bar.
Not yet started on code. This file is the working plan; it folds into
`HANDOFF.md` at closeout.

## Goal & discipline (revised — stronger than the original M1 wording)

Make `src/` a **line-faithful port of memtest86+ v5.01**. Per explicit user
direction this session, the discipline is now maximal:

> **Each ported source file STARTS as a verbatim copy of its upstream
> counterpart.** We then *comment out* the code that doesn't apply (x86 leaf code,
> SMP, relocation, DMI/SPD, serial, config menu) and *add* the PPC/OF
> equivalents inline next to what they replace. **We do not delete upstream
> code** — commented-out original stays as the reference and keeps the diff
> legible. Additions are clearly marked (e.g. `/* PPC: ... */`).

> **The on-screen display must be byte-for-byte identical to upstream**, with the
> only intentional text change being **`Memtest86` → `Memtestppc`**, plus the
> obvious data adaptations (PowerPC CPU name, OF-reported clock/cache/memory).
> Everything the session-005 work changed away from upstream gets reverted toward
> upstream.

This supersedes the looser "extract our code into upstream-named files" idea from
the first draft. The payoff: upstream bugfixes are pullable, and the diff reads as
"x86 commented out, PPC added."

## Decisions locked this session
- **(1) Skeleton fidelity:** verbatim copy + comment-don't-delete, as above.
- **(2)/(3) Display:** identical to upstream except `Memtestppc`. **Revert** the
  session-005 changes: the two-line `Memory:`/`Testing:` readout → back to
  upstream's single `Memory:` + `L3 Cache:` line; the slimmed `<ESC> reboot`
  footer → back to upstream's full `footer()` text; the custom static error
  header → removed (upstream draws it lazily in `common_err`). Restore upstream's
  exact `init()`/`display_init()` layout (Core#/State/Cores row, etc.).
- **(4) SPINSZ:** use **`0x800000` (32 MB/chunk)** — the value memtest86+ shipped
  for its *entire 1.x–2.x era*, restored for the PowerPC RAM era. This is a *free*
  knob: `find_chunks()` derives the chunk count from `SPINSZ`, so `find_ticks`
  stays consistent at any value (the only thing that must NOT be hardcoded is the
  tick total, which the port wrongly did). Verified timeline from the archived
  tarballs: **v1.00 (Dec 2003) → v1.10 → v1.11 → v2.00 (2008) all use
  `0x800000` = 32 MB**; it only jumped to `0x4000000` = 256 MB by v5.01 (2013) for
  big-RAM x86. 32 MB was thus the live value during the exact years G3/G4 Macs
  were current — historically correct, not a guess. 256 MB/chunk makes a whole
  sweep on a 256 MB G3 ~1 tick (frozen-looking bar); 32 MB gives ~8 ticks/sweep.
  Comment the define citing the v1.00–2.00 precedent. (Earlier "match v5.01's
  256 MB" call reversed after the user flagged it; archived sources now in
  `~/Downloads/memtest86+-{1.00,1.10,1.11,2.00}`.)

## How this was researched
Read in full: port `src/{main,display,test,ofw}.c` + headers; upstream `error.c`,
`screen_buffer.{c,h}`, `test.h`, `config.h`, `lib.c` print section, `init.c`
`display_init`+`init`, and the canonical `main.c` sections (`tseq`, `next_test`,
`set_defaults`, `test_start`, `test_setup`, `do_test`, `find_chunks`,
`find_ticks_*`, `compute_segments`). A subagent did a line-level `test.c`
comparison and located the static-header draw.

**Tooling gotcha (logged):** upstream `init.c` is ISO-8859 encoded (raw `°C`
byte), so plain `grep` treats it as binary and **silently skips all matches** —
why the header strings looked "missing." Use `grep -a` / the Read tool on the
upstream tree.

---

## The rendering architecture (the one real design decision)

x86 text mode auto-renders the buffer at `0xb8000`; PPC has only a linear
framebuffer, so *something* must blit. The faithful way to keep `cprint` (and
`scroll`/`clear_scroll`) **verbatim** is to hook the blit into the leaf they
already call:

- `vga_buf[]` **is** `SCREEN_ADR` (the `0xb8000` analog: 80×25 char+attr). Already
  true in the port (`display.h`); keep it. Upstream `cprint` writes char bytes
  here unchanged.
- Upstream `cprint` → calls `tty_print_line()` (screen_buffer.c) → `ttyprint()`
  (lib.c), which on x86 echoes to the **serial console**. We have no serial
  console but we *do* need framebuffer output, so **`ttyprint` becomes our
  framebuffer renderer** (the leaf swap): it reads char+attr from `vga_buf` for
  the given cells and draws glyphs via the font/palette backend. `scroll` already
  routes through `tty_print_region`, so it renders for free too.
- Net effect: import `lib.c`, `screen_buffer.c`, `error.c`, `init.c`, `main.c`,
  `random.c`, `test.c` close to verbatim; the *only* display edits are inside
  `ttyprint` (serial bytes → glyph blit) and `display_init`'s low-level setup
  (VGA cursor/serial → OF framebuffer init). The current `display.c` FB code
  (`fb_*`, palette, `display_render_cell`, font) becomes the **backend that
  `ttyprint` calls** — it stops being the place that defines `cprint` et al.

This means `screen_buffer.c` *is* worth importing (repurposed), reversing the
first draft's "out of scope" call on it.

---

## Target file layout (mirror upstream; "src/ ← upstream, ported")

| File | Origin | Port action |
|---|---|---|
| `head.S` | (PPC, exists) | keep — already the PPC entry/`r5` CIF leaf |
| `ofw.{c,h}` | (PPC, exists) | keep — the OF client interface; this is our `io.h`/BIOS replacement (no upstream analog) |
| `lib.c` | upstream `lib.c` | import verbatim; comment out `inter`/interrupt `codes[]`, `set_cache` (x86), `get_key`/keyboard (x86), `serial_*`; **repurpose `ttyprint` → framebuffer blit**; keep all print primitives, `scroll`, `clear_scroll`, `footer`, `str*`/`mem*` |
| `screen_buffer.{c,h}` | upstream | import; `tty_print_line/region` drive the FB renderer; `get/set_scrn_buf` track the char shadow (or fold onto `vga_buf`) |
| `display.c` (FB backend) | (PPC, exists) | demote to backend: `fb_*`, palette, font blit, `vga_buf`, one-time clear. Provides the body `ttyprint` calls. No longer defines `cprint` etc. |
| `font_vga.h` | (PPC, exists) | keep |
| `error.c` | upstream `error.c` | **new** — import; keep `error/ad_err1/ad_err2/common_err/update_err_counts/print_err_counts/do_tick`; comment out SMP (`spin_lock/barr`), `beep`, DMI, ECC, `PARITY_MEM`; swap `rdtsc` elapsed-time → timebase leaf; `smp_my_cpu_num()`→0 |
| `init.c` | upstream `init.c` | **new** — import `display_init`+`init`; keep every header `cprint` verbatim except `Memtest86`→`Memtestppc`; comment out x86 cursor/`outb`/serial; swap `get_cpuid`/cache → PPC PVR + OF props; swap `mem_size` → OF `discover_memory`/claim |
| `random.c` | upstream `random.c` | **new** — import (already pure C); host `rand`/`rand_seed`/`rand_next` (currently mis-placed in port `test.c`) |
| `main.c` | upstream `main.c` | reshape toward upstream: `tseq[]`, `next_test`, `set_defaults`, `test_start` (main loop), `test_setup`, `do_test`, `find_chunks`, `find_ticks_*`, `compute_segments`, `main()` bootstrap. Comment out SMP/reloc/window/barrier/`btrace`/`parse_command_line`; single-CPU collapse left visible as commented scaffolding |
| `test.c` | upstream `test.c` | already ~faithful; re-do as **comment-the-asm / un-comment-the-C** (preserve the asm), fix 3 routines (below), inline single-CPU `calculate_chunk` with the omitted version left commented |
| `test.h` | upstream `test.h` | replace `memtest.h`; restore full `struct vars` (`tptr/pptr/pass_ticks/total_ticks/msg_line/ecc_ecount/erri.*` …), upstream `tseq` literals, layout `#define`s. PPC `mftb`/`dcbf`/`sync` inlines move to a small PPC header or stay appended-and-marked |
| `config.h` | upstream `config.h` | import as-is (compile flags); most features `#undef` for us |

**Deliberately NOT ported** (PLAN.md non-goals; leave dangling call sites as
commented stubs): `smp.*`, `reloc.c`, `controller.c`, `dmi.c`, `spd.c`, `patn.c`
(badram), `extra.c`/`config.c` (menu), `cpuid.c` (x86) — PPC CPU id is done inline
in `init`.

---

## Divergence inventory (what the port changed away from upstream)

### A. Error path — biggest gap (→ `error.c`)
Port `error()` (main.c:177) invented a layout: static header `" Address Expected
Actual Xor Tst Pass"` (main.c:440) + row `hprint(adr)@1, good@15, bad@27,
xor@39, test@51, pass@55`. Upstream `common_err` `PRINTMODE_ADDRESSES` (the
default, error.c:307) draws lazily on first error: header (error.c:315) `"Tst
Pass   Failing Address          Good       Bad     Err-Bits  Count CPU"`; row
`test+1@0, pass@4, page@11, offset@19, " -      . MB"@22, mb@25, frac@31,
good@36, bad@46, xor@56, ecount@66, cpu@74`; de-dups consecutive identical
errors. The session-005 "render each red cell 1..76" loop is the faithful PPC
form of `print_err_counts`'s `0x47` fill (our shadow buffer needs explicit
render) → it belongs inside `print_err_counts`. cpu col = 0. Restoring this makes
the error screen match `ref/photos/Memtest86+_memory_errors.png`.

### B. do_tick (→ `error.c`)
Adopt upstream shape (error.c:461): incremental bars via `v->tptr`/`v->pptr`
(draw only new `#`), `dprint(2,COL_MID+4,pct,3,0)`, spinner via `spin_idx[]`.
Replace the `rdtsc` elapsed-time block with our timebase `update_time()` leaf.

### C. Print primitives (→ `lib.c`)
Currently in `display.c`, logic faithful but: `dprint` right-justify branch was
simplified → restore upstream's exact loop (lib.c:344-367); `hprint3` missing →
add. Otherwise verbatim. (Rendering now via `ttyprint`, per architecture above.)

### D. struct vars / globals (→ `test.h` + `main.c`)
Port uses standalone globals (`v_msg_line`, `test_ticks`, `nticks`) and ignores
the `v->msg_line/tptr/pptr/pass_ticks/total_ticks` fields it still declares.
Copying upstream `main.c`/`test.h` resolves this naturally (everything via `v->`).
Restore `set_defaults()` (main.c:118) for `v->erri.*` init.

### E. tseq table (→ `main.c`)
Port set all `cpu_sel=1`. Restore upstream literals `{1,-1,..}/{1,32,..}/{1,1,..}`;
they're vestigial under single-CPU but kept for fidelity (the loop's single-CPU
collapse treats them as "run once"). Document that `cpu_sel` is inert here.

### F. find_ticks (→ `main.c`)
Replace the port's hardcoded `switch` with upstream `find_ticks_for_pass/_test` +
`find_chunks` (main.c:1043-1187). With `act_cpus==1` the `/act_cpus`/`*act_cpus`
terms reduce to identity — keep them written (commented `/* /1 */`) per the
discipline. Pairs with the restored `SPINSZ=0x4000000`.

### G. test.c re-alignment (subagent audit)
Mostly faithful (asm→commented-C). Fix:
1. **addr_tst1** — restore the inner `for(i=0;i<50;i++)` repeat on bank 2 (up
   test.c:140) the port dropped.
2. **movinvr** — most diverged: port rolls its own seed (`mftb()`+fixed XOR) and
   embeds the RNG. Move RNG → `random.c`; restore upstream seed scheme
   (rdtsc/`v->pass` → timebase leaf).
3. **block_move** — port uses plain `<<|>>` rotate + `memcpy`; upstream uses
   rotate-through-carry (`rcll`). Match the rotate semantics (keep `memcpy` as the
   move leaf).
Plus opportunistically fix the `movinv32` uninitialized-`k` warning (test.c:324).

### H. draw_screen → init()/display_init() (revert session-005)
Restore upstream exactly: single `Memory:` line (5) + `L3 Cache:` line (4);
`Core#:`(7)/`State:`(8)/`Cores: Active/Total … Pass … Errors`(9); full `footer()`
text; remove the custom error header. Only `Memtest86`→`Memtestppc` in the title
plus CPU-name/clock/cache/mem data adaptations. Blinking `+`, green title attr,
`| Time:` clock, vertical separators, `---` rules: keep upstream's exact
positions.

---

## Staged execution (each stage ends green in QEMU; commit per stage)

Build/verify = rsync → `make clean && make memtestppc.iso` on opti7050 → QEMU
mac99 256 MB VNC screenshot (CLAUDE.md → Build & test).

0. **Scaffold the rendering swap first.** Import `lib.c` + `screen_buffer.c`
   verbatim; wire `ttyprint` to the existing FB backend; make `cprint` etc. come
   from `lib.c`. Get the *current* screen rendering through the upstream call path
   unchanged. *(De-risks everything else; if this is green, verbatim `cprint`
   works on PPC.)*
1. **`test.h`** ← upstream: full `struct vars`, layout `#define`s, `tseq` literals,
   PPC inlines appended. Switch globals → `v->`. Verify bars/pass counter advance.
2. **`init.c`** ← upstream `display_init`+`init`: exact header (revert session-005,
   `Memtestppc`), PPC CPU/cache/mem leaves. Screenshot-match upstream layout.
3. **`random.c`** + **`find_ticks`/`SPINSZ`**: move RNG out; restore upstream tick
   math (`find_ticks_for_pass/_test` via `find_chunks`) + set `SPINSZ=0x800000`
   (v2.00 value). Verify bar fills to ~100% over a pass and moves visibly.
4. **`error.c`** ← upstream: `common_err`/`PRINTMODE_ADDRESSES`, `print_err_counts`
   (red fill), `do_tick`. Verify with a temporary fake-error injection (session-005
   technique; remove before commit); screenshot vs the reference error photo.
   *(Headline stage.)*
5. **`test.c`** re-align (G) + warning fix. Verify a clean full pass = 0 errors,
   sane timings.
6. **`main.c`** skeleton: reshape toward `test_start`; comment-out SMP/reloc/window
   scaffolding with one-line "single-CPU port: N/A" markers.
7. **Hardware verify on ibookg32** (partition boot `boot hd:5,memtestppc.elf` via
   the uranium hop): header, bars, restored error rows on the real 8-bit display.
   Re-burn CD only if a new release artifact is wanted.

Stages 0–3 are plumbing; 4 is the headline. If time-boxed, 0+1+4 deliver most of
the fidelity win.

## Open questions / to confirm during build
- Whether to fold `screen_buffer.c`'s char shadow onto `vga_buf` (avoid a second
  buffer) or keep it as upstream — decide when wiring stage 0; folding is less
  faithful but avoids redundancy. Lean: keep `vga_buf` as the single char+attr
  buffer and have `tty_print_line` read attrs from it.
- `config.h` features to `#undef` (`USB_WAR`, `PARITY_MEM`, serial, beep, APM_OFF).
- Where the PPC `mftb/dcbf/sync` inlines live (small `ppc.h` vs appended to
  `test.h`) — cosmetic.

## Out of scope this session
SMP/reloc, config menu, serial console, DMI/SPD, badram patterns, the test-11
`sleep()` stub (bit-fade timing), and the empirical 8-bit BGR palette order
(tracked in memory + PLAN.md).
