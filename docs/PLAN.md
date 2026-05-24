# memtestppc+ — project plan

## Mission

A standalone, bootable memory tester for PowerPC Macs that faithfully reproduces
the classic **memtest86+ v2.00** blue-screen TUI. Because it runs with **no
operating system**, the maximum fraction of system RAM is under test — the whole
point of memtest86+, which has no PowerPC equivalent (Apple Hardware Test and
Open Firmware's built-in test don't compare).

It boots straight into testing from an Open Firmware-loaded ELF. The aesthetic
target is pixel-faithful: the IBM VGA 8x16 font (codepage 437), the green title
bar with the blinking `+`, white-on-blue text, white-on-red errors. The project
version tracks the memtest86+ base it ports: the first faithful,
real-hardware-verified release is **v2.00** — a tester a PowerPC Mac owner can
boot from CD and trust. (Early bring-up snapshots were tagged 0.01; the line-port
itself was rebased onto memtest86+ v2.00 in session 006.)

## Non-goals

- **No SMP / multi-core.** Target machines are single-CPU G3s. The original's
  SMP, relocation, and windowing machinery is deliberately dropped — that's why
  the port's `main.c` is far simpler than upstream's.
- **No build-from-source on Tiger.** Tiger's gcc emits Mach-O; OF needs ELF.
  We cross-compile, and ship the CD image as the artifact. Most users download
  the ISO; they don't build it.
- **Not x86.** This is a PowerPC/Open Firmware port, not a general refactor of
  memtest86+.

## Strategy / approach

- **Reuse the original TUI code via a virtual VGA text buffer.** We keep an
  80×25 char+attribute buffer (exactly the x86 text-mode format) and render
  changed cells to the OF framebuffer with an embedded 8x16 font. This lets the
  original `cprint`/`dprint`/`hprint`/`scroll`/etc. work almost verbatim — the
  key move that makes a *faithful port* rather than a look-alike feasible.
- **Open Firmware client interface** for everything hardware: memory discovery
  and claiming, framebuffer, timebase, keyboard, reboot.
- **Cross-compile on Linux** (`powerpc-linux-gnu-gcc`), **iterate in QEMU**
  (mac99, fast), **verify on real hardware** (iBook G3 — the source of truth for
  OF MMU behaviour, the 8-bit palette, real CPU detection, real timebase).
- **Port-fidelity discipline** (see CLAUDE.md → Key invariants): keep upstream's
  structure; replace only x86 leaf code. Minimize the diff against v2.00.

The detailed bring-up findings (the r5 CIF discovery, the `-kernel` MMU/ISI
dead-end, the CHRP/HFS+ ISO recipe, the 8-bit `color!` BGR ordering, the
ofw_claim-in-1MB-chunks approach) live in the session HANDOFFs `001`–`004`.

## Roadmap

- **M1 ✅ DONE — port-fidelity restructure (session 006).** Rebuilt `src/` as a
  line-faithful port of **memtest86+ v2.00** (rebased from the earlier v5.01-ish
  hack): every upstream file imported and consciously ported or classified, the
  display/error/test paths brought to upstream structure, and a small, legible
  diff against v2.00. Builds, runs the full test suite in QEMU (Errors: 0), and
  boots + runs on the real iBook G3. The **v2.00 release** is cut from this.
- **M2 (mostly done): hardware-truth pass.** ✅ Physical CD-R boots on the iBook
  (session 005); ✅ runs end-to-end on hardware (session 006). **Remaining:**
  visually verify the white-on-red error row on the iBook's real 8-bit screen
  (never triggered there — no errors seen), and optionally refine the memory map
  (probe OF `/memory/available` rather than unconditionally skipping the low
  8 MB).
- **M3: breadth + release.** ✅ Release/packaging flow stood up (GitHub releases;
  `artifacts/` staging). **Remaining:** test on imacg3 (Rage 128) and G4/G5
  (other CPU PVRs; the G5 `#address-cells=2` >4 GB caveat); watch a full pass
  complete on hardware; G5 single-CPU speed spot-check.

## Open questions / deferred

- `movinv32`: verify the documented-C pattern math vs the upstream asm at
  `sval != 0` (and the `k`-may-be-uninitialized warning) — `src/test.c`. Default
  tseq invocations use `sval` 0/1 where they coincide; confirm against a green run.
- The 8-bit `color!` BGR / index-first argument order is empirical (from the
  iBook's ATI Rage Mobility). May differ on other Macs' OF; might need per-machine
  detection or a different approach for non-ATI GPUs.
- ELF size / strip: could `-s` further if older OF implementations are picky about
  CD-loaded ELF size.
- Scroll-lock shows `LOCKED` and `scroll()` honors it, but it only gates once the
  error region fills (faithful to v2.00); arrow-key navigation of the error log
  is **not** a v2.00 feature (review is via the scrolling log + scroll-lock).
