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
- **Footer honesty:** ✅ RESOLVED in Wave 6. `check_input` wires `c`→`get_config`
  (works), `^L`→redraw, and SP/CR→scroll-lock toggle (`slock` flips + footer
  shows `LOCKED`). Scroll-lock IS functional: `scroll()` honors `slock` verbatim
  from upstream (`while (slock) check_input();` at lib.c — it blocks before
  scrolling the next error row in once the error region is full, so you can read
  the log; Enter clears `slock` to resume). (Earlier note that scroll() didn't
  gate on slock was wrong — it does.)
- **Relocation (`reloc.c`):** ✅ RESOLVED — examined and parked N/A
  (`examine-reloc_c.md`); we test in place, non-PIC, no self-reloc.
- **Bit-fade `sleep()`:** ✅ RESOLVED in Wave 4 — `sleep()` uses the OF timebase
  (`port-test_c.md`). Note bit-fade (tseq idx 9) only runs when selected
  (DEFTESTS=9), so its ~90-min dwell never occurs in a default pass.

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

### Wave 2 — display/string core — ✅ DONE, QEMU-verified (render path end-to-end)
| screen_buffer.c/.h | ✅ | report `port-screen_buffer.md`. Byte-verbatim (platform-neutral char shadow + tty_print_line/region). |
| lib.c | ✅ | report `port-lib_c.md`. ttyprint→fb_render_cell loop (the one key edit); print/str/mem kept verbatim; check_input→OF stdin poll (ESC→ofw_reset); set_cache PPC stub (uncached-test TO-VERIFY); inter/get_key/getval/ser_map/serial_* commented; serial_echo_init stub keeps clear_screen_buf(). Caller audit: clean for Waves 2-5; Wave-5 main.c must comment the serial_console_setup(cmdline) call. |
| random.c | ✅ | report `port-random_c.md`. Byte-verbatim MWC RNG, pure C; seeding is at call sites (Wave 4/5). Minor: test.h declares `ulong rand()` vs random.c's `unsigned int` — ABI-identical on ILP32 BE; kept standalone to avoid redecl. |

### Wave 3 — screen + detection — ✅ DONE, QEMU-verified (real boot screen)
| init.c | ✅ | report `port-init_c.md`. Verbatim v2.00 TUI; title `Memtest86`→`Memtestppc` (+ moved col15→16); CPU id via `mfpvr`+`ppc_cpu_names[]`, clock/cache from OF /cpus props; x86 cpuid/cpuspeed/memspeed/PAE-paging/chipset/DMI/SPD/pci/floppy all commented; `display_init()` calls `fb_init()` first; `display_refresh()` added (PPC) for attr-only cells. Compile bug found+fixed by lead: a comment containing `*/` (`st_*/end_*`) closed early. |
| memsize.c | ✅ | report `port-memsize_c.md`. `mem_size()` skeleton + post-proc kept; body→OF `/memory` reg + `ofw_claim` 1MB chunks (prints "OFW Map"). Sets pmap/msegs/test_pages/selected_pages/reserved_pages/plim_* (all 4K-page units). Skips low 8MB. |

### Wave 4 — error + tests — ✅ DONE, QEMU-verified (error table white-on-red)
| error.c | ✅ | report `port-error_c.md`. error/ad_err1/ad_err2/common_err + all print modes VERBATIM (PRINTMODE_ADDRESSES table byte-for-byte). **v2.00 error.c has NO SMP and NO CPU column** (those are v5.01-isms) — last col is `Chan` (ECC). do_tick IS here: bars/spinner verbatim, rdtsc elapsed-time → PPC timebase. Red row: 0x47 fill + PPC fb_render_cell loop. DMI/ECC/beep/poll_errors commented. Checkpoint stubs: page_of (>>12), insertaddress (return 0); globals p/p1/p2/p0 from main; beepmode owned by init.c. |
| test.c | ✅ | **Wave-5 found+FIXED a reverse-pass spin**: the `pe - SPINSZ < pe` underflow guard (movinv1 + movinv32) relied on pointer-underflow wraparound (UB); gcc -O1 folded it true → infinite `do_tick` on any segment whose start < SPINSZ (our low 8 MB seg). Replaced with an underflow-safe byte-distance check `(ulong)pe - (ulong)start >= SPINSZ*sizeof(ulong)`. Tests now cycle 0→1→2→3→4 with sane %s + Errors:0 (`wave5-tests-cycling.png`). report `port-test_c.md` (+ fix note). Outer SPINSZ/segment/fwd-rev structure verbatim; each test's x86 asm inner loop commented, upstream-commented plain-C enabled (addr_tst1/bit_fade were already pure C). movinvr seed rdtsc→mftb/mftbu; sleep() rdtsc+clks_msec→Time Base + ofw_get_timebase_freq. block_move had no upstream C — rewrote it: carry-faithful 33-bit `rcll` rotate (NOT the attic's plain rotate), memmove for `rep movsl`, C adjacent-word check. **bit_fade keeps the full ~90-min dwell (STIME=5400) — flagged for the user to maybe shorten.** No cpu-param/calculate_chunk to strip (v2.00 is cpu-free). Fixed a `/* PPC: */`-in-header-comment early-close (same trap init.c hit). TO VERIFY: movinv32 documented-C pattern-math vs the asm's plain roll/ror when sval!=0. |

### Wave 5 — main — ✅ DONE, QEMU-verified (tests RUN: bars advance, Errors: 0)
| main.c | ✅ | report `port-main_c.md`. tseq[] verbatim; do_test + switch(pat) + find_ticks/find_ticks_for_test/compute_segments verbatim. **The one structural change:** x86 self-relocation loop → in-place `for(;;) do_test()` in main() (head.S calls main; do_test returns instead of run_at). windows[] 32-entry PAE table → single flat window {0,0xffffffff} clamped to plim_*; run_at/__run_at/parse_command_line #if 0'd. Flat paging defined here (map_page/mapping/emapping/page_of/paging_off = upstream FLAT==1 branch; init.c's x86 PAE versions stay #if 0'd). adj_mem + insertaddress = Wave-6 stubs. **Default pass runs tests 0-8 only (DEFTESTS=9) — bit_fade (idx 9) NOT run unless selected, so the 90-min dwell does not occur in a normal run.** |
| reloc.c | 🅿 | report `examine-reloc_c.md`. x86 ELF self-relocation runtime (`_dl_start`, R_386_* relocs, %ebx GOT asm). N/A: PPC head.S calls main() directly, OF maps the ELF at link addr, non-PIC build, no self-reloc. NOT imported into src/ (wouldn't compile on PPC; no caller). |

### Wave 6 — config.c PORTED; PC-platform files examined → ⛔/parked — ✅ DONE, QEMU-verified
| config.c | ✅ | report `port-config_c.md`. **Ported (Decision #6):** surfaced Test Selection / Address Range / Error Report Mode / Restart / Refresh Screen + popups + real `adj_mem()` (replaces main.c stub); commented PC-only (Memory Sizing/DMI/ECC/SPD/beep, `controller.h`/`dmi.h` includes). Interactive input now real over **OF stdin** (`get_key`/`getval` enabled + `ser_map`/`ascii_to_keycode`; `wait_keyup`→no-op; in lib.c); `check_input` wires `c`→`get_config` (footer honesty). **QEMU-verified: `c` opens the Settings popup with the correct neutral subset, `0` dismisses it and testing resumes** (`wave6-config-menu.png`). config.h was already ported (Wave 1). |
| patn.c | ✅ | report `port-patn_c.md`. Imported VERBATIM — pure neutral C (BadRAM pattern table); replaces the main.c `insertaddress` stub, makes PRINTMODE_PATTERNS real. 32-bit-longword assumption is correct for elf32-powerpc. `printpatn()` is in error.c (not patn.c). |
| controller.c/.h | ⛔ | report `examine-controller_c.md`. x86 north-bridge driver: PCI config cycles + MSRs + ~120 Intel/AMD/VIA/SiS/nVidia PCI IDs. No live callers. Shelves chipset/ECC/FSB/timings TUI fields. Not built. |
| dmi.c/.h | ⛔ | report `examine-dmi_c.md`. Parses x86 SMBIOS via a fixed low-phys `_SM_` scan; PPC analog is the OF device tree. No live callers. (Defines its own `strlen` — would collide; revive as fresh module.) |
| spd.c | ⛔ | report `examine-spd_c.md`. Reads DIMM SPD over the Intel-ICH SMBus via x86 port I/O + PCI. `show_spd()` never called. |
| pci.c/.h | ⛔ | report `examine-pci_c.md`. PCI config-space via x86 ports 0xCF8/0xCFC; OF owns the PCI tree. Only ref is init.c's commented `pci_init()`. |
| extra.c/.h | ⛔ | report `examine-extra_c.md`. Live x86 **DRAM-timing/overclock editor** (pci_conf_* + raw MMIO BARs + inline asm delay), not the CPU-temp screen expected. Out of scope. |
| linuxbios.c, linuxbios_tables.h | ⛔ | report `examine-linuxbios_c.md`. coreboot table parse (`"LBIO"` scan → E820); replaced by `memsize_ofw()`. `query_linuxbios()` proto referenced only in init.c's commented firmware-probe block. |
| msr.h | ⛔ | report `examine-msr_h.md`. x86 rdmsr/wrmsr/rdtsc + MSR constants; PPC uses SPRs, timing via `ppc.h` mftb. No includes in src/. |
| io.h | ⛔ | report `examine-io_h.md`. x86 port-I/O (inb/outb) — exactly what `ofw.{c,h}` replaces (PORTING-GUIDE §4). |
| serial.h | ⛔ | report `examine-serial_h.md`. 16550 UART map + serial-console glue; output via vga_buf/ttyprint, input via OF stdin. |
| bootsect.S, setup.S | ⛔ | report `examine-bootsect_setup_S.md`. x86 real-mode boot/setup (BIOS int 0x13/0x10, A20, GDT, PE switch); OF `load` + `src/head.S` replace them. |
| *.lds, mt86+_loader(.asm) | ⛔ | report `examine-lds-and-loader.md`. x86 GNU link scripts (`src/linker.ld` replaces) + DOS MZ real-mode loader. `mt86+_loader` is the assembled `.asm` binary. |
| elf.h | 🅿 | report `examine-elf_h.md`. ELF defs; only consumer was the parked `reloc.c`; has `R_386_*` only (no `R_PPC_*`), we don't self-relocate / parse ELF at runtime. |
| stdin.h | ⛔ | does not exist in v2.00 (a v5.01-era file) — recorded for completeness; no row needed elsewhere. |

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
- 2026-05-24: **Wave 2 (display/string core) done and QEMU-verified end-to-end.**
  Three subagents: `lib.c` (port-lib_c.md, the ttyprint→fb_render_cell rewrite +
  check_input→OF + x86 leaf commented), `screen_buffer.{c,h}` (verbatim),
  `random.c` (verbatim). Integrated into the Makefile; only undefined symbol at
  link was `v` (defined in the checkpoint stub; real def comes with main.c, Wave
  5). Built ISO, booted QEMU mac99 256MB: the **verbatim** `cprint`/`dprint`/
  `hprint`/`aprint`/`footer` all render correctly through the unchanged
  `cprint → tty_print_line → ttyprint → fb_render_cell` chain (evidence:
  `wave2-render-check.png`). The print core is proven; later waves can rely on it.
  - **QEMU gotcha (recorded):** never `pkill -f qemu-system-ppc` from an ssh
    command — `-f` matches the full cmdline, including the ssh shell *running the
    pkill* (its cmdline contains the string), so it kills its own session →
    exit 255. Use `pkill -9 -x qemu-system-ppc` (match process name, not cmdline).
- 2026-05-24: **Wave 3 (screen + detection) done and QEMU-verified.** Two
  subagents ported `init.c` + `memsize.c`. Renamed backend `display_init→fb_init`
  so init.c keeps upstream `display_init`. After fixing a comment-nesting compile
  bug in init.c, the ISO boots to a faithful v2.00 boot screen with LIVE PPC data:
  `PowerPC 7400 (G4) 900 MHz`, L1 32K / L2 256K, Memory 236M, RsvdMem 20M, MemMap
  "OFW Map" — recognizably the memtest86+ v2.00 TUI. Evidence:
  `wave3-boot-screen.png`. Wave-5 watch-list below captures the loose ends
  (paging-family call sites, adj_mem stub, title/version question).
- 2026-05-24: **Wave 4 (error + tests) done and QEMU-verified — M1 headline.**
  Two subagents ported `error.c` + `test.c`. The verbatim v2.00 PRINTMODE_ADDRESSES
  error table renders edge-to-edge white-on-red on PPC (4 injected fake errors:
  page+MB address, Good/Bad/Err-Bits, incrementing Count, Errors:4 tally).
  Evidence: `wave4-error-table.png`. Confirmed v2.00 error.c has no SMP/CPU-column
  (col is `Chan`). test.c: faithful asm→C, block_move rotate-through-carry, RNG
  seed→timebase, bit_fade keeps full ~90-min dwell (flagged). Lead fixes during
  integration: added `ofw.h` include to test.c; added p/p1/p2/p0 globals to the
  checkpoint; beepmode is owned by init.c (don't redefine). The full engine now
  links (all 11 objects) — only the real main.c test LOOP remains (Wave 5).
- 2026-05-24: **Wave 5 (main) done and QEMU-verified — the tests RUN in place.**
  Ported the real v2.00 `main.c` (report `port-main_c.md`), replacing the
  throwaway stub. **The one structural change:** x86's self-relocation loop
  (`head.S`→`do_test`, `run_at` re-entry) became an in-place `for(;;) do_test();`
  in `main()` (PPC head.S already calls `main`); `do_test` `return`s where
  upstream relocated. The 32-entry PAE `windows[]` collapsed to a single flat
  window `{0,0xffffffff}` (clamped to `plim_*`); `map_page`/`mapping`/`emapping`/
  `page_of`/`paging_off` are flat in-place versions defined in main.c (init.c's
  x86 PAE copies stay `#if 0`'d). `run_at`/`__run_at`/`parse_command_line` `#if
  0`'d. `adj_mem`+`insertaddress` = Wave-6 stubs. `reloc.c` examined → parked
  N/A (`examine-reloc_c.md`): x86 ELF self-reloc runtime, no PPC caller, non-PIC
  build. Builds + links clean (11 objects, only the known GNU-stack/RWX warnings).
  Booted QEMU mac99 256MB: **the engine runs the test loop over the full claimed
  range** — "Testing: 8192K - 252M 236M", WallTime advancing, `Errors: 0` on
  good RAM. Evidence: `wave5-tests-running.png`. Debug-probed `msegs=2 segs=2
  chunks=8 test_ticks=112` (all correct). **BUT — surfaced a Wave-4 `test.c` bug
  (see RESUME #1):** the progress %s overflow (Test% climbs past 64000% and
  grows) because `do_tick()` is called unboundedly inside `movinv1` — test #2
  (first moving-inversions) never completes, so tests don't cycle and a pass
  never finishes. `test_ticks`/`chunks` (main.c) are correct, so the spin is in
  test.c's moving-inversions chunk loop, latent until Wave 5 first drove real
  memory. Each `do_tick` also does an `ofw_read` (OF call), so the over-call is
  also why a single test crawls for many minutes under QEMU. **→ Root-caused and
  FIXED same session (next log entry).**
- 2026-05-24: **Fixed the `test.c` reverse-pass spin — tests now cycle.**
  Instrumented per-phase do_tick counters: `FILL=8 FWD=8 REV=74085+` (growing) →
  the **reverse pass** spins. Segment bounds: `M0=[0x00800000,0x03ffff00]`
  (8-64 MB), `M1=[0x04800000,0x0fbfff00]` (72-252 MB). Root cause: the reverse
  underflow guard `if (pe - SPINSZ < pe)` (in `movinv1` and `movinv32`) relied on
  **pointer-arithmetic underflow wraparound** — when `pe - SPINSZ` goes below
  address 0 it becomes huge so `< pe` is false and the `else` clamps `pe = start`.
  That is UB; `powerpc-linux-gnu-gcc 12 -O1` folds `pe - SPINSZ < pe` to
  always-true, killing the clamp. For seg0 (start = 8 MB < SPINSZ-stride = 32 MB)
  the reverse walk steps past the bottom, wraps to ~0xFFFFFFFF, and the 32 MB
  stride **skips over `[0, start]`**, so `pe <= start` is never true → infinite
  `do_tick`. Fix: underflow-safe byte-distance check
  `if ((ulong)pe - (ulong)start >= (ulong)SPINSZ * sizeof(ulong))` (both reverse
  guards; forward guards left as-is — they can't wrap for ≤4 GB addresses and
  clamp via `pe >= end`). Rebuilt + booted QEMU: tests **cycle** 0→1→2→3→4,
  Pass% 1→4→7 and Test% advance **monotonically and stay ≤100%**, `Errors: 0`.
  Evidence: `wave5-tests-cycling.png`. (A full pass is slow under QEMU TCG —
  test 4 random-pattern is iter=60 — but it progresses; full-pass timing belongs
  on real hardware.)
- 2026-05-24: **Wave 6 done and QEMU-verified — config menu works, PC files
  classified.** Five subagents read+classified the PC-platform files; all are
  **⛔ N/A** with no live callers (controller/dmi/spd, pci/extra, linuxbios/msr/
  io/serial, bootsect/setup/lds/loader/elf; reports `examine-*.md`). `stdin.h`
  doesn't exist in v2.00. **`patn.c` ported VERBATIM** (pure neutral BadRAM
  pattern C; replaced the main.c `insertaddress` stub). **`config.c` ported**
  (`port-config_c.md`) per Decision #6: surfaced Test Selection / Address Range /
  Error Report Mode / Restart / Refresh Screen + popups + the real `adj_mem()`
  (replaced the main.c stub); commented PC-only branches (Memory Sizing/DMI/ECC/
  SPD/beep + controller.h/dmi.h). Made the menu usable by wiring **OF-stdin
  keyboard input** in lib.c: rewrote `get_key()` (OF stdin byte → `ascii_to_keycode`/
  `ser_map`, both re-enabled), enabled `getval()`, made `wait_keyup()` a no-op
  (OF has no key-up), and wired `check_input` `c`→`get_config` + SP/CR scroll-lock
  + ^L redraw (footer honesty — closes that open question). Built clean (13
  objects). QEMU regression: tests still cycle, `Errors: 0`. **Interactively
  verified via QEMU `sendkey`:** `c` opens the Settings popup showing exactly the
  neutral subset (PC-only items absent), `0` dismisses it and testing resumes
  with the screen restored. Evidence: `wave6-config-menu.png`. (Deep getval
  numeric entry for Address Range not exhaustively driven — best on real HW.)

- 2026-05-24: **First real-hardware run on ibookg32 (iBook G3) — mostly great,
  one bug found + fixed.** Deployed the Wave-6 ELF to `ibookg32:/memtest` (build
  on opti7050 → uranium → ibookg32; boot via OF `boot hd:3,memtest` — note our
  test path is **hd:3** `/memtest`, the root HFS+ partition, not hd:5). User
  reports: the **config menu renders and is functional** with a real keyboard
  (ADB/USB via OF console), and the **footer scroll-lock is responsive** (SPACE →
  `LOCKED`, Enter clears). **Bug:** pressing the **Down arrow rebooted the
  machine**. Cause: arrow/nav keys arrive over the OF console as ANSI escape
  sequences (`ESC '[' <final>`), and `check_input()` treated the leading `ESC`
  byte (0x1b) as the "(ESC)Reboot" command. (QEMU/OpenBIOS never surfaced this —
  I only injected specific keys via `sendkey`.) **Fix (`lib.c`):** in
  `check_input`, after reading `ESC`, peek for a follow-on byte via a new
  `read_pending_byte()` (bounded ~20 ms via the OF timebase, latency-independent);
  if a `[`/`O` sequence follows, drain through its final byte (0x40–0x7e) and
  ignore — only a *bare* ESC reboots. Rebuilt, QEMU-regression OK (tests run,
  Errors:0; lone-ESC still reboots — verified via `sendkey esc`), redeployed to
  `ibookg32:/memtest` (md5 matches). **User to re-confirm on hardware that the
  Down arrow no longer reboots.** Known minor follow-up: in the *config menu*,
  arrow keys still go through `get_key()` (no draining there) and inject mapped
  junk into `getval()` numeric entry — harmless, not the reported bug.

- 2026-05-24: **Second input fix from hardware testing — drain the whole console
  buffer per `check_input()`.** User found: after mashing arrow keys then pressing
  ESC, the reboot took several seconds. Cause: `check_input()` is called once per
  `do_tick()` (once per SPINSZ/32 MB chunk) and processed only **one event per
  call**, so a backlog of buffered keystrokes drained one-per-chunk and the ESC
  queued behind them wasn't reached for many chunks. Fix (`lib.c`): wrap the read
  + dispatch in `while (ofw_read(...) > 0)` to **process every pending byte in
  order each call** (meaningful keys acted on; arrow/nav escape sequences drained
  + ignored). A held key's auto-repeat has gaps so the buffer empties and the loop
  exits — no busy-spin. QEMU-regression OK (tests run Errors:0; lone-ESC still
  reboots). Redeployed to `ibookg32:/memtest`. (Note: this is process-in-order,
  not flush — a queued `c`/scroll-lock before the ESC is still honored.)

- 2026-05-24: **Reverted the buffer-drain loop — back to upstream's
  one-event-per-`check_input` structure (fidelity).** On hardware the drain loop
  made no perceptible difference to the mash-then-ESC latency (the dominant
  latency is the `do_tick` interval — once per 32 MB chunk — not per-event
  draining), and it was a structural departure from upstream (which processes one
  key per call). Since arrow keys are non-functional in v2.00 anyway (no arrow
  handling exists upstream — error review is the scrolling log + scroll-lock, not
  arrow nav), the mash-then-ESC case is moot. **Kept** the ESC-vs-escape-sequence
  disambiguation (necessary OF-platform leaf: the console delivers arrows as
  multi-byte `ESC [` sequences where upstream's PS/2 path had one scancode).
  `check_input` is now the single-read + ESC-disambiguation form (== the
  QEMU-verified state of commit `0311d7a`); rebuilt, re-booted in QEMU (tests run,
  Errors:0), redeployed to `ibookg32:/memtest`. Also corrected a HANDOFF error:
  `scroll()` DOES honor `slock` (scroll-lock works).

- 2026-05-24: **Cosmetic fix — CPU clock showed "MH" instead of "MHz."** User
  spotted it on hardware. Port bug (not upstream): `cpu_type()` (init.c) drew the
  clock with `cprint(LINE_CPU, 20, "      MHz")`, whose trailing 'z' landed at
  col 28 — exactly where `init()` draws the vertical divider `"| "` (COL_MID-2),
  which is drawn *after* `cpu_type()` and overwrote the 'z'. (Upstream positioned
  the MHz string dynamically just past the CPU name via `off`, so it never
  collided.) Fix: moved the clock to col 18 so 'z' ends at col 26 with a space
  before the divider. QEMU-verified: now reads "PowerPC 7400 (G4)   900 MHz".
  artifacts/ refreshed + committed (c48cc7d). The iBook briefly went offline
  (No route to host — it was at the OF prompt / running /memtest, sshd down), then
  came back in Tiger; `artifacts/memtest` (md5 1794535…) redeployed to
  `ibookg32:/memtest` — verified matching. User to re-confirm "MHz" on hardware.

- 2026-05-24: **Cut the v2.00 release.** Hardware-confirmed the MHz fix on the
  iBook. Captured the README hero on QEMU `mac99 -cpu 750` — authentic "PowerPC
  750 (G3) 900 MHz" (the 900 MHz iBook G3 is the fastest G3 ever shipped; not
  g3beige, which is Old World and not our New World boot target) — saved to
  `docs/media/memtestppc-v2.00.png`. Refreshed README (v2.00 intro/hero/status
  table/links) and PLAN (M1 done, v2.00, roadmap). Built a clean ISO, refreshed
  `artifacts/` (memtest md5 1794535…). Committed, pushed origin/main, and cut the
  **v2.00 GitHub release** (`cellularmitosis/memtestppc`) with `memtest` +
  `memtestppc.iso` as assets; README download links point at the v2.00 tag.

- 2026-05-24: **Post-release miss caught: the title `+` does not blink.** User
  noticed the README claimed a "blinking `+`". Upstream sets attr `0xA4` (blink +
  green bg + red fg) and the VGA *hardware* blinks bit-7 cells for free; our
  framebuffer has no such hardware and `fb_render_cell` masks bit 7 off, so the
  `+` renders static red-on-green. A software blink would need a steady timer we
  do not have (only `do_tick`, irregular). Dropped "and blinking `+`" from the
  README, and captured the lesson as a reusable VGA-port gotcha in
  PORTING-GUIDE §3 ("Gotchas when porting VGA text mode to a framebuffer").

## Next steps — RESUME HERE (v2.00 RELEASED; M2/M3 breadth remain)

State on exit: **the whole v2.00 import is complete.** Every upstream file is
classified (ported or ⛔/parked with a report). The engine compiles, links (13
objects), runs the full test loop in place over all claimed memory — tests cycle,
percentages sane, `Errors: 0` — and the (c)configuration menu works (popup +
OF-stdin keys). **Only QEMU-tested; real hardware (ibookg32/G5) NOT yet tested**,
and a full pass not yet watched to completion (QEMU TCG is slow on the heavy
tests — do it on hardware).

1. **Finish-up:** real-hardware test on ibookg32 (partition boot
   `boot hd:5,memtestppc.elf` via the uranium hop) — watch a full pass complete +
   the config menu with a real keyboard + G5 speed spot-check; resolve
   bit_fade `STIME` (90 min) + title/version question; update `README.md` +
   `PLAN.md` (held while src/ was mid-rewrite — it is now a runnable release
   candidate, so they should be refreshed to describe the v2.00 port); version
   decision.
2. **Optional polish (open questions parked above):** reloc.c stays parked; the
   few `test.c` TO-VERIFYs (movinv32 documented-C vs asm at sval≠0; ad_err2 cast).
   Possible nicety: arrow-key navigation of the error log — **not** a v2.00
   feature (v2.00 reviews errors via the scrolling log + scroll-lock, no arrow
   nav anywhere), so adding it would be a deliberate enhancement beyond the
   faithful port, not a bug fix.

How to build/test + the `pkill -x` gotcha: see the progress log above and
PORTING-GUIDE §7. Per-file porting decisions: the `port-*.md` reports in this dir.
Keep this file's file-status table + progress log + watch-lists current.

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

## Wave-5 (main.c) watch-list (surfaced porting init.c/memsize.c)
- **Paging family `#if 0`'d in init.c but still called by upstream main.c:**
  `map_page`, `mapping`, `emapping`, `page_of`, `paging_off` are declared in
  test.h and used by v2.00 main.c's windowing. Single-CPU in-place testing
  doesn't relocate/window, so the Wave-5 main.c port must comment those call
  sites (or provide flat-mapping stubs) or they're undefined at link.
- **`adj_mem()` lives in config.c (Wave 6)** but `mem_size()` calls it. main.c
  (Wave 5) builds before config.c, so it needs an `adj_mem()` stub (no-op is
  safe — mem_size already set `selected_pages`) until config.c lands. The Wave-3
  checkpoint stub provided one.
- **`find_ticks()`, `tseq[]`, `v`** are owned by main.c — they materialize in
  Wave 5 (the checkpoint stub provided stand-ins).
- **Title spacing / version string (OPEN, for the user):** title currently
  renders `Memtestppc+v2.00` (the `+` is cramped against the version because
  "Memtestppc" is 1 char wider than "Memtest86" and TITLE_WIDTH=28). Decide:
  (a) keep `v2.00` vs use the memtestppc+ project version; (b) whether to drop a
  leading space to restore `Memtestppc+ v2.00` spacing. Cosmetic; in init.c title.
- **G5 caveat:** memsize.c's `/memory reg` walk assumes 1-cell addr/size (true on
  G3/G4/QEMU); a G5 with `#address-cells=2` needs a stride fix. Revisit for G5.

## Substrate contract (for downstream waves — do not break)
- `vga_buf[]` (in `display.c`) is `SCREEN_ADR`: 80×25×2 char+attr.
- `display.c` exports `void fb_render_cell(int y, int x)` (reads vga_buf[y][x],
  blits the glyph), `int fb_init(void)` (OF framebuffer hardware bring-up), and
  `void display_refresh(void)`. **Renamed display_init→fb_init in Wave 3** so the
  ported `init.c` keeps its upstream `display_init()` (TUI content draw). init.c's
  `display_init()` calls `fb_init()` first.
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
