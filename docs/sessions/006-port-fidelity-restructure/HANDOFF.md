# Session 006 — port-fidelity restructure (rebase onto v2.00, import-everything)

**This is the living master doc for the session.** A future session (or a
post-compaction continuation) should read: `CLAUDE.md` → project `README.md` →
**this file** → [`PORTING-GUIDE.md`](PORTING-GUIDE.md) (the contract every porting
subagent follows). Supporting research: [`plan.md`](plan.md) (original divergence
analysis — *predates the v2.00 pivot*, see note there) and
[`v1.00-vs-v2.00.md`](v1.00-vs-v2.00.md) (the base-version comparison).

## What this session is doing

Rebuilding `src/` as a **line-faithful, single-CPU PowerPC/Open-Firmware port of
memtest86+ v2.00** that reproduces the **TUI _and behavior_ of memtest86+ as
closely as possible** — a faithful port, not a memtest-alike. This replaces the
old "quick hack" look-alike port (now parked in `attic/src-v0.01/`). Work proceeds **one upstream file at a time**, each handled
by a subagent that imports the file, ports it under the discipline below, and
writes a per-file report into this session dir.

## Decisions locked (with the why, so they can be re-judged)

1. **Base version = memtest86+ v2.00** (`ref/memtest86+-2.00/`, Feb 2008).
   - *Why:* single-CPU (0 SMP refs in main.c vs 100 in v5.01), already has a clean
     `error.c` module (the thing M1 restructures toward), richer-but-still-classic
     test suite (adds random-pattern, random-sequence, bit-fade over v1.00), and
     natively uses the era-correct `SPINSZ=0x800000` (32 MB) — so the SPINSZ debate
     is moot, we just inherit it. v1.00 was the runner-up (simpler/nostalgic) but
     lacks `error.c` and those 3 tests. Full reasoning in `v1.00-vs-v2.00.md`.
   - The old port was effectively v5.01-derived; we are NOT continuing from it.

2. **Discipline: import the *complete* upstream tree; drop nothing by assumption.**
   Every v2.00 source file is brought in and *consciously* classified — never
   silently skipped. For each file: start from a verbatim copy, comment out x86/PC
   leaf code (with a one-line reason), add PPC/OF code inline (clearly marked),
   **never delete upstream code**, and stub/comment call sites so it still links.
   The point is a forcing function: looking at every file so we deliberately decide
   which features to *surface* vs shelve, rather than guessing.
   - *Anti-goals (called out explicitly by the user):* deciding by guess instead of
     reading the code, and inheriting the old hack port's choices as if they were
     reasons. Neither is allowed to drive a decision.

3. **Single generic non-SMP binary for G3/G4/G5.** No SMP. Spot-check it isn't too
   slow without SMP on the user's 2 GB G5. (v2.00 has no SMP anyway.)

4. **TUI: reproduce v2.00's screen exactly**, with the only text change being
   `Memtest86` → `Memtestppc`, plus data adaptations (PPC CPU name from PVR;
   clock/cache/memory from OF). No hybrid across versions.

5. **Rendering architecture — the one real design choice.** Keep `cprint`/`scroll`/
   `clear_scroll` byte-verbatim by swapping the leaf they already call: upstream's
   `ttyprint()` (the x86 serial-console echo) becomes our **framebuffer renderer**.
   `vga_buf[]` is the `0xb8000`/`SCREEN_ADR` analog (char+attr); `ttyprint` reads
   char+attr from it and blits glyphs via the PPC framebuffer/font backend. So the
   only display edits are inside `ttyprint` + low-level `display_init` setup.
   Details in [`PORTING-GUIDE.md`](PORTING-GUIDE.md).

6. **`config.c` features to surface (not drop):** Test Selection, Address Range,
   Restart, Refresh Screen, and the basic error-report modes are legitimate,
   platform-neutral, and worth keeping. The PC-only options (Memory Sizing/BIOS,
   DMI info, SPD data, ECC mode) are N/A. Implementation may land in a later
   milestone, but the decision is "surface these," recorded now.

## Open questions parked for later
- **Footer honesty:** v2.00's verbatim `footer()` advertises `(c)configuration`
  and scroll-lock. If kept verbatim we should eventually wire up `c` + scroll-lock
  (ties to #6) or those keys are advertised-but-dead. Decide when porting
  `init.c`/`lib.c`.
- **Relocation (`reloc.c`):** upstream relocates itself to test the RAM it runs
  from. The OF port claims memory in place instead; decide whether to port/stub
  reloc when we reach it.
- **Bit-fade `sleep()`:** needs a timebase delay (was stubbed in the old port).

## Workflow for this effort (see PORTING-GUIDE.md for the full contract)
- Files are ported in **dependency waves** (substrate → headers → display/string →
  screen+detect → error+tests → main → examine PC-platform files).
- Each file is one task (see TaskList) and yields one report
  `docs/sessions/006-.../port-<file>.md` (or `examine-<file>.md` for files we
  classify but don't actively port).
- Build/verify checkpoints between waves on opti7050 (rsync → `make` → QEMU).
- This HANDOFF's **file-status table** below is the source of truth across
  sessions; keep it current as files land.

## File-status table (v2.00 → port)

Status: ☐ todo · ◐ in progress · ✅ ported+report · 🅿 examined→parked/stub · ⛔ N/A

### Substrate (PPC-native, carried from attic/src-v0.01 — not v2.00 ports) — ✅ DONE, QEMU-verified
| file | status | notes |
|---|---|---|
| ofw.c / ofw.h | ✅ | OF client interface = the io.h/BIOS replacement; carried verbatim |
| head.S | ✅ | PPC entry, OF CIF in r5; carried verbatim |
| linker.ld | ✅ | PPC ELF layout (load 0x00400000); carried verbatim |
| font_vga.h | ✅ | IBM VGA 8x16 font; carried verbatim |
| display.c/.h (fb backend) | ✅ | **stripped to backend only** (print primitives removed — they return from v2.00 lib.c). Exports `fb_render_cell(int y,int x)` (the contract lib.c's ttyprint will loop over) + `display_init`/`display_refresh`. `vga_buf` = SCREEN_ADR lives here. |
| ppc.h | ✅ | mftb/mftbu/dcbf/sync/isync intrinsics (extracted from old memtest.h) |
| main.c | ✅ (throwaway) | Wave-0 smoke-test stub — draws title/body/palette/footer to verify the fb. **Replaced by ported v2.00 main.c in Wave 5.** |
| Makefile | ✅ | OBJS = head, ofw, display, main; grows per wave |

### Wave 1 — headers — ✅ DONE, cross-compiler syntax-checked
| test.h | ✅ | report `port-test_h.md`. Verbatim v2.00; full 29-field struct vars restored; includes ppc.h + display.h; SCREEN_ADR=vga_buf (0xb8000 def commented); commented x86 leaves (getCx86, cache_on/off, reboot, struct cpu_ident). |
| config.h | ✅ | report `port-config_h.md`. PARITY_MEM/APM_OFF/USB_WAR commented; serial/beep flags kept *defined* at inert values (lib.c #if/#error guards need them). |
| stdint.h | ✅ | imported; ILP32 BE PPC matches x86 widths, no changes. |
| defs.h | ✅ | all x86 real-mode boot/GDT/reloc — whole body `#if 0`'d. |
| elf.h | ☐ | not needed yet (examine in Wave 6 if anything references it). |

### Wave 2 — display/string core
| screen_buffer.c/.h | ☐ | char shadow + tty_print_line/region |
| lib.c | ☐ | print primitives + str/mem; ttyprint→fb leaf |
| random.c | ☐ | near-verbatim (pure C) |

### Wave 3 — screen + detection
| init.c | ☐ | header draw verbatim (Memtestppc); PPC cpu/cache/mem |
| memsize.c | ☐ | mostly replaced by OF discovery; examine |

### Wave 4 — error + tests
| error.c | ☐ | common_err + print modes + error table; comment SMP/DMI/ECC |
| test.c | ☐ | test routines; x86 asm→C; drop cpu param |

### Wave 5 — main
| main.c | ☐ | tseq, next_test, find_ticks, loop; comment reloc/window/cmdline |
| reloc.c | ☐ | examine; likely stub (in-place testing) |

### Wave 6 — examine PC-platform files (import → report → classify)
| controller.c/.h | ☐ | x86 chipset/mem-controller; expect ⛔ |
| dmi.c/.h | ☐ | SMBIOS; ⛔ |
| spd.c | ☐ | DIMM SPD over SMBus; ⛔ |
| pci.c/.h | ☐ | PCI cfg via x86 I/O; ⛔ |
| extra.c/.h | ☐ | overclock menu; ⛔ |
| config.c/.h | ☐ | config menu; **surface neutral subset (decision #6)** |
| patn.c | ☐ | BadRAM patterns; examine |
| linuxbios.c, linuxbios_tables.h | ☐ | coreboot tables; ⛔ |
| msr.h, io.h, serial.h, stdin.h | ☐ | x86 MSR/port-IO/serial/kbd; ⛔/adapt |
| bootsect.S, setup.S | ☐ | x86 real-mode boot; ⛔ (OF loads ELF) |
| *.lds, mt86+_loader* | ☐ | x86 link/loader; ⛔ |

## Progress log (newest last)
- 2026-05-24: Session opened. Researched the v5.01 port-fidelity gap (see
  plan.md), then pivoted: compared v1.00/v2.00/v5.01, recovered the lost early
  tarballs (now in `ref/`), and **rebased the whole effort onto v2.00** with an
  import-everything discipline. Old port moved to `attic/src-v0.01/`. Wrote this
  HANDOFF + PORTING-GUIDE.
- 2026-05-24: **Wave 0 (substrate) done and QEMU-verified.** Built on opti7050
  (clean compile + link; only the known benign GNU-stack/RWX-segment warnings),
  ISO booted on QEMU mac99 256MB. Smoke-test stub renders correctly: green title
  bar, light-gray-on-blue body, the 16-color palette row, and the reverse-video
  footer — confirming OF fb discovery, palette, 8x16 font blit, and the
  `vga_buf → fb_render_cell` contract. Evidence: `wave0-smoke-test.png`.
- 2026-05-24: **Wave 1 (headers) done, cross-compiler syntax-checked.** Two
  subagents ported `test.h` (port-test_h.md) and `config.h`+`stdint.h`+`defs.h`
  (port-config_h.md). Reading the real v2.00 source corrected several v5.01-era
  assumptions — see "v2.00 facts that bite later waves" below. `test.h`+`config.h`
  compile clean (`-fsyntax-only`) on `powerpc-linux-gnu-gcc`.

## Next steps
1. **Wave 2:** fan out subagents — `screen_buffer.{c,h}` (#4), `lib.c` (#5, the
   key ttyprint→fb_render_cell wiring + comment the x86 interrupt/`inter`/serial/
   keyboard/cache leaf), `random.c` (#6). Then build-checkpoint (these + the
   substrate should link a small image, perhaps with a tiny driver, or wait for
   Wave 3-5 to get a full main).
2. Then Wave 3 (`init.c`, `memsize.c`), Wave 4 (`error.c`, `test.c`), Wave 5
   (`main.c`, `reloc.c`), Wave 6 (examine PC-platform files).
3. Keep this file's file-status table + progress log + "v2.00 facts" current.

## v2.00 facts that bite later waves (learned porting the headers — NOT v5.01!)
- **Test routines are already cpu-free** in v2.00 — no trailing `int cpu` arg. No
  signature stripping needed (that was a v5.01-ism).
- **Single `void bit_fade(void)`** — NOT the `bit_fade_fill`/`bit_fade_chk` split
  (that split was the old hack's invention). Wave-4 test.c defines `bit_fade()`;
  Wave-5 main.c `do_test` calls it; the tseq bit-fade row maps to it.
- **`struct tseq` = `{short cache; short pat; short iter; short ticks; short errors;
  char *msg;}`** — different fields *and* `pat` numbering from v5.01/the hack. The
  Wave-5 tseq table, `next_test`, `find_ticks`, and `do_test`'s `switch(pat)` all
  follow v2.00's shape. Re-read v2.00 main.c/test.c for the real pat numbers.
- **config.h:** serial/beep flags stay *defined* at inert values (0) — the actual
  serial/beep leaf code is commented when porting lib.c/init.c/error.c, not via
  config.h. `BEEP_END_NO_ERROR`/`CONSERVATIVE_SMP` don't exist in v2.00.
- **defs.h body is `#if 0`'d** — `LOW_TEST_ADR`/`INITSEG`/segment defs are gone, so
  the Wave-5 main.c port MUST comment the x86 reloc/cmdline paths that use them or
  it won't resolve.
- **`struct eregs`** is only forward-declared in test.h; its real def + `inter()`
  (x86 interrupt handler) live in lib.c — Wave-2 lib.c port should comment that
  whole x86 interrupt path (N/A on PPC/OF). `SCREEN_END_ADR` is not provided; add
  to display.h only if a consumer appears.

## Substrate contract (for downstream waves — do not break)
- `vga_buf[]` (in `display.c`) is `SCREEN_ADR`: 80×25×2 char+attr.
- `display.c` exports `void fb_render_cell(int y, int x)` (reads vga_buf[y][x],
  blits the glyph) and `int display_init(void)` / `void display_refresh(void)`.
- Wave-2 `lib.c`: keep `cprint`/`scroll`/etc. verbatim; rewrite **only** `ttyprint`
  to loop `fb_render_cell` over its (y,x,text) run; stub `serial_echo_*`.
- **Attr-only changes need an explicit render.** `cprint` routes through
  `tty_print_line`, which dedups on the *char* shadow (`screen_buf`) — so code that
  poke attribute bytes directly into `vga_buf` WITHOUT changing the char (init.c's
  green title bar / reverse footer; error.c's `0x47` red rows) will NOT auto-render.
  Those sites must call `fb_render_cell(y,x)` (or `display_refresh()`) explicitly
  after poking the attr. (x86 didn't need this — VGA hardware re-paints from attr;
  our shadow buffer doesn't.) The old hack port did exactly this in `error()`.
- PPC intrinsics in `ppc.h`. OF facilities via `ofw.{c,h}`.
- Framebuffer is 800×600 under QEMU OpenBIOS (8/32-bit paths both handled).
