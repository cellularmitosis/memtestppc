# memtestppc+ — project plan

## Mission

A standalone, bootable memory tester for PowerPC Macs that faithfully reproduces
the classic **memtest86+ v5.01** blue-screen TUI. Because it runs with **no
operating system**, the maximum fraction of system RAM is under test — the whole
point of memtest86+, which has no PowerPC equivalent (Apple Hardware Test and
Open Firmware's built-in test don't compare).

It boots straight into testing from an Open Firmware-loaded ELF. The aesthetic
target is pixel-faithful: the IBM VGA 8x16 font (codepage 437), the green title
bar with the blinking `+`, white-on-blue text, white-on-red errors. Versions run
**0.01 → 1.00**; "done" (1.00) is a faithful, real-hardware-verified tester a
PowerPC Mac owner can boot from CD and trust.

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
  structure; replace only x86 leaf code. Minimize the diff against v5.01.

The detailed bring-up findings (the r5 CIF discovery, the `-kernel` MMU/ISI
dead-end, the CHRP/HFS+ ISO recipe, the 8-bit `color!` BGR ordering, the
ofw_claim-in-1MB-chunks approach) live in the session HANDOFFs `001`–`004`.

## Roadmap

- **M1 (active): port-fidelity restructure.** Bring the codebase closer to
  upstream v5.01's structure — especially the display and error-reporting paths,
  which are currently more rolled-our-own than they should be. Goal: a small,
  legible diff against upstream.
- **M2: hardware-truth pass.** Visually verify the white-on-red error display on
  the iBook's 8-bit screen (re-inject a fake error briefly). Burn and boot a
  *physical* CD on real hardware (only QEMU ISO + iBook partition boot tested so
  far). Refine the memory map (probe OF `/memory/available` instead of skipping
  the first 8 MB unconditionally).
- **M3: breadth + release.** Test on imacg3 and G4/G5 (exercises the 32-bit
  framebuffer path and other CPU PVRs). Stand up a release/packaging flow (no
  GitHub remote or releases exist yet). Drive to **1.00**.

## Open questions / deferred

- `movinv32` warning: `k` may be used uninitialized (`src/test.c`) — initialize
  at declaration.
- Bit-fade test (test 11) `sleep()` is stubbed — needs a timebase-based delay.
- The 8-bit `color!` BGR / index-first argument order is empirical (from the
  iBook's ATI Rage Mobility). May differ on other Macs' OF; might need per-machine
  detection or a different approach for non-ATI GPUs.
- ELF size / strip: could `-s` further if older OF implementations are picky about
  CD-loaded ELF size.
