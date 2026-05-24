# PORTING-GUIDE — the contract for porting memtest86+ v2.00 → memtestppc+

**Every porting subagent MUST read this file first.** It is the stable set of
rules and shared conventions so that independently-run subagents produce a
consistent, faithful, buildable port. The living status/decisions are in
[`HANDOFF.md`](HANDOFF.md); this file is the *how*.

## 0. The mission in one paragraph
memtestppc+ is a standalone, bootable memory tester for PowerPC Macs. It boots
from an Open Firmware-loaded ELF with **no operating system** and reproduces the
**TUI _and behavior_ of memtest86+ as closely as possible** — this is a *faithful
port*, not a memtest-alike look-alike (the old `attic/src-v0.01/` port was the
latter; we are not). We are porting **memtest86+ v2.00**
(`/Users/cell/claude/memtestppc+/ref/memtest86+-2.00/`) — single-CPU, x86 — to
single-CPU PowerPC + Open Firmware. One generic binary targets G3/G4/G5.

## 1. The porting discipline (non-negotiable)
For each upstream file we bring in:
1. **Start from a verbatim copy** of the v2.00 file. Preserve function skeletons,
   variable names, comments, and layout constants.
2. **Comment out** x86/PC-specific *leaf* code rather than deleting it. Leave a
   one-line reason: `/* x86: <what> — N/A on PPC/OF */`. Prefer commenting the
   tightest leaf (an asm block, a port-I/O call), not whole functions, when the
   surrounding structure still applies.
3. **Add PPC/OF replacements inline**, right next to what they replace, marked
   `/* PPC: <what> */`. Where upstream left a plain-C version commented above an
   asm block, invert it: comment the asm, enable the C.
4. **Never delete upstream code.** Commented-out original is the reference that
   keeps our diff legible and upstream-navigable.
5. **Keep it buildable.** If you comment out a function others call, stub it or
   comment the call sites too (`#if 0 … #endif` is fine for large dead regions).
   Do not leave dangling references.
6. **Decide consciously, never by guess or by what the old port did.** If you
   cannot tell what code does, READ it (whole file). Mark genuine unknowns "TO
   VERIFY" in your report; do not assert. **Do not worry about conserving tokens** —
   reading the source in full and making the best-informed decision supersedes any
   concern about saving tokens or output length. Thoroughness wins here.

## 2. What is already decided (do not re-litigate)
- Base = v2.00. TUI reproduced **exactly**, except `Memtest86` → `Memtestppc` and
  data adaptations (CPU name, clock, cache, memory amounts from PVR/OF).
- Single-CPU: drop the trailing `int cpu` / `int me` parameters and any SMP
  branching (v2.00 has little/none). The single-CPU path is the live one.
- `SPINSZ` stays at v2.00's `0x800000` (32 MB) — inherited, do not change.
- Encoding gotcha: some files (esp. `init.c`) are ISO-8859 (a raw `°C` byte). Use
  the Read tool or `grep -a`; plain `grep` treats them as binary and hides matches.

## 3. The rendering architecture (how text reaches the screen)
On x86, `cprint` writes char bytes into the VGA text buffer at `0xb8000` (hardware
auto-paints) and mirrors to a serial console via `tty_print_line` → `ttyprint`.
On PPC there is no text-mode hardware, only a linear framebuffer. So:

- **`vga_buf[]` is our `SCREEN_ADR`** — an 80×25 char+attr array (2 bytes/cell),
  identical in format to `0xb8000`. `#define SCREEN_ADR ((unsigned long)vga_buf)`.
  Upstream `cprint`/`cplace`/`scroll`/`clear_scroll` write into it **verbatim**.
- **`ttyprint(y, x, text)` becomes the framebuffer renderer** (the single leaf we
  swap). Instead of emitting serial escapes, it reads char+attr from `vga_buf` for
  the cells in that run and blits glyphs via the framebuffer/font backend
  (`display.c`). `serial_echo_*` become no-ops/removed.
- Because `cprint` (and `scroll` via `tty_print_region`) already route every
  screen write through `tty_print_line` → `ttyprint`, they stay byte-verbatim and
  rendering "just works."
- The backend (`display.c`, PPC-native, adapted from `attic/src-v0.01/display.c`)
  owns: framebuffer discovery via OF, the 8x16 font, the 16-color VGA palette
  (incl. the 8-bit indexed path with the empirical BGR order), `vga_buf`, and the
  exported render entry that `ttyprint` calls. **Contract:** backend exports
  something like `void fb_render_cell(int y, int x);` (reads `vga_buf[y][x]`); add
  a run helper if convenient. Keep this name stable across subagents.

### Gotchas when porting VGA text mode to a framebuffer
These bite because a linear framebuffer is *dumb* — it has none of the VGA text
hardware's free behaviors. Each must be reproduced in software or consciously
dropped:

- **The blink bit (attr bit 7) does nothing on a framebuffer.** On x86, an attr
  byte with bit 7 set (e.g. the title `+` is `0xA4` = blink + green bg + red fg)
  blinks *for free* — the VGA CRTC toggles bit-7 cells on a hardware timer, zero
  CPU. A framebuffer has no such hardware: `fb_render_cell` blits the glyph once,
  and it extracts the background as `(attr >> 4) & 0x07` — which (correctly, per
  VGA semantics when blink is enabled) **masks bit 7 off**, so the cell renders
  *statically* (red-on-green `+`, not blinking). The blink is silently dropped.
  To actually blink you must animate in software (re-render blink-attr cells on a
  ~0.5 s cadence) — and that needs a steady timer, which this OF/no-interrupt,
  poll-driven design does not have (the only periodic hook is `do_tick()`, whose
  cadence follows test progress). **MISS (session 006):** the README claimed a
  "blinking `+`" (faithful to upstream's `0xA4` intent) while the port renders it
  static; the claim was removed. Lesson: any "blink" in the original is free VGA
  hardware that will *not* survive the port — decide per blink-attr cell whether
  to animate it or accept static, and don't advertise blink you don't render.
- **Attr-only changes need an explicit re-render.** `cprint` dedups on the *char*
  shadow (`screen_buf`), so poking an attribute byte into `vga_buf` *without*
  changing the char (the green title bar, the reverse footer, error.c's `0x47`
  red rows) will NOT auto-render — call `fb_render_cell(y,x)` / `display_refresh()`
  after. (x86 didn't need this: the VGA hardware re-paints from attr; our shadow
  buffer doesn't.)

## 4. The PPC substrate (provided; not x86 ports)
These have no faithful x86 counterpart — they replace the PC platform itself.
Carried/adapted from `attic/src-v0.01/`:
- `ofw.c` / `ofw.h` — Open Firmware client interface (finddevice/getprop/claim/
  open/read/write/exit + fb info + timebase). This is the `io.h`/BIOS replacement.
- `head.S` — PPC entry stub; OF passes the client-interface pointer in **r5**
  (not r3). Sets up stack in BSS, calls `main`.
- `linker.ld` — ELF layout for the OF-loaded image.
- `font_vga.h` — IBM VGA 8x16 glyph table.
- `display.c` / `display.h` — the framebuffer backend (see §3).
- `ppc.h` — `mftb`/`mftbu`/`dcbf`/`sync`/`isync` inlines (were in the old
  `memtest.h`). Include from `test.h` or where needed.

When an upstream file calls a PC facility (BIOS memory probe, port I/O, MSR, PCI
config, serial), the replacement is an `ofw_*` call or a PPC instruction — route
through `ofw.*`/`ppc.h`, do not invent ad-hoc mechanisms.

## 5. Per-file report format
Each subagent writes `docs/sessions/006-port-fidelity-restructure/port-<file>.md`
(use the upstream filename, e.g. `port-lib_c.md`; for files we classify but don't
actively port, `examine-<file>.md`). The report MUST contain:
- **File & purpose** — what the upstream file does (from reading it; cite line
  numbers).
- **Disposition** — ported / adapted / parked-stub / N-A, and why.
- **x86 leaf code commented out** — list each region + the PPC replacement (or why
  none needed).
- **PPC/OF additions** — what you added and how it maps to the original.
- **Features surfaced or shelved** — anything user-facing worth a conscious call
  (esp. relevant for `config.c`, `error.c` print modes, footer keys).
- **Open questions / TO VERIFY** — anything you could not determine from source.
- **Build impact** — new/changed call sites, Makefile changes, what it depends on.

Keep reports tight and evidence-based. Update `HANDOFF.md`'s file-status table
row for your file (or report the new status so the lead can).

## 6. Dependency wave order (don't port out of order)
0. **Substrate** (§4) — must exist first; defines the `ttyprint`→backend contract.
1. **Headers:** `test.h`, `config.h` (+ `stdint.h`/`defs.h`/`elf.h` as needed).
2. **Display/string core:** `screen_buffer.{c,h}`, `lib.c`, `random.c`.
3. **Screen + detection:** `init.c`, `memsize.c`.
4. **Error + tests:** `error.c`, `test.c`.
5. **Main:** `main.c`, `reloc.c`.
6. **Examine PC-platform files:** `controller.*`, `dmi.*`, `spd.c`, `pci.*`,
   `extra.*`, `config.*`, `patn.c`, `linuxbios.*`, `msr.h`, `io.h`, `serial.h`,
   `stdin.h`, `bootsect.S`, `setup.S`, `*.lds`, `mt86+_loader*` — import, report,
   classify (most ⛔ N/A; `config.c` surfaces the neutral subset per HANDOFF #6).

Within a wave, files with no cross-dependency can be ported in parallel. Across
waves, sequence — later waves include/call earlier ones.

## 7. Build & verify (reference; the lead runs checkpoints)
```
rsync -av --exclude '.git' --exclude 'ref/' --exclude '*.o' --exclude '*.elf' \
  --exclude '*.bin' --exclude '*.iso' --exclude 'docs/' --exclude 'testing/' \
  /Users/cell/claude/memtestppc+/ opti7050:/home/cell/memtestppc+/
ssh opti7050 'cd /home/cell/memtestppc+ && make clean && make memtestppc.iso'
# QEMU: mac99, 256MB, -cdrom, VNC :99, screendump via monitor socket (see CLAUDE.md)
```
Cross-compile only (`powerpc-linux-gnu-gcc`); never build on Tiger. A subagent that
can't reach the build host should still produce correct source + report; the lead
integrates and builds at the wave checkpoint.
