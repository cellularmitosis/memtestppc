# Session 001: Inception & Scaffold

> **Note:** This handoff was reconstructed retroactively (during session 005) from
> the full transcript at
> [docs/convos/claude-conversation-2026-05-23-c9a7979e.md](../../convos/claude-conversation-2026-05-23-c9a7979e.md).
> Sessions 001, 002, and 003 were originally a *single* Claude Code session that
> hit context-compaction three times; at the end it wrote only one handoff and
> labelled it "003". This document and [002-cd-boot](../002-cd-boot/HANDOFF.md)
> backfill the earlier phases of that session so the numbering reflects the work.

## What happened

The project was created from scratch. This phase covers understanding the goal,
choosing an architecture, settling the build toolchain, writing the complete
source scaffold, and getting it to compile to a PowerPC ELF for the first time.

### The goal (from the user's brief)

Build **memtestppc+** — a standalone, bootable memory tester for PowerPC Macs
that faithfully reproduces the classic **memtest86+ v5.01** blue-screen TUI. It
boots from CD-ROM straight into testing with **no operating system**, so the
maximum fraction of RAM is under test. Start at version **0.01** and iterate to
**1.00**. Use the **IBM VGA 8x16 font** (codepage 437) for pixel-perfect
authenticity. **Leverage as much of the original memtest86+ TUI code as
possible** — ideally replacing its BIOS/VGA leaf calls with our own rendering
routines rather than rewriting the display layer. Reference material (full
memtest86+ v5.01 source, screenshots, photos) is in `ref/`.

### Architecture decided: hybrid Open Firmware client

- Open Firmware (IEEE 1275) loads our ELF from the CD.
- We use the OF **client interface** (`finddevice`, `getprop`, `open`, `read`,
  `claim`, `exit`) to discover memory layout, framebuffer, and timebase.
- We render the TUI to the OF framebuffer using an **embedded 8x16 VGA font**.
- **Key trick that preserves the original code:** we keep a virtual VGA text
  buffer `vga_buf[80*25*2]` (char+attribute pairs, exactly like x86 text mode)
  and `#define SCREEN_ADR ((unsigned long)vga_buf)`. The original memtest86+
  `cprint`/`dprint`/`hprint`/`aprint`/`scroll` functions then work almost
  verbatim — they write to the buffer, and we render changed cells to the
  framebuffer with the font.

x86 → PowerPC substitutions:
- `CPUID` → **PVR** (Processor Version Register) for CPU identification.
- `RDTSC` → **Timebase Register** (`mftb`/`mftbu`); frequency from OF device tree.
- E820 BIOS memory map → OF **`/memory`** node `reg` property.
- VGA text mode at `0xB8000` → OF framebuffer + our font renderer.
- **No SMP** — the target G3 iMac is single-core, so all the SMP/relocation/
  windowing machinery in the original `main.c` is dropped.

### Build-toolchain saga (this ate real time — read before re-litigating it)

1. The local Mac has **no** PPC cross-compiler and no Docker.
2. **opti9020** (Debian 13) has `gcc-powerpc-linux-gnu` (14.2.0),
   `qemu-system-ppc` (10.0.8), and `genisoimage`. Installed the cross-compiler
   there first.
3. **User interrupted** to ask that we build with the **native system gcc on
   ibookg32** instead, because that's what a fellow Tiger enthusiast with an
   Xcode 2.5 install would have.
4. **Blocker discovered:** Tiger's gcc emits **Mach-O**, but Open Firmware needs
   **ELF** (or XCOFF). Tiger has no `objcopy`/ELF linker to bridge this.
5. User's response: *"do whatever you think is best. Most users will just
   download the CD image artifact anyway, rather than attempting to build from
   source."* → We went with the cross-compiler. **Building from source on a real
   Tiger Mac is not a supported path; the deliverable is the CD image.**
6. opti9020 then went unreachable mid-session, so we pivoted to **opti7050**
   (Ubuntu 22.10), installed `powerpc-linux-gnu-gcc` **12.2.0**, and that became
   the build host for the rest of the project.

### Font sourcing

The IBM VGA 8x16 OTB from int10h.org needed parsing, so instead we took the
identical bitmap from the **Linux kernel's `lib/fonts/font_8x16.c`** (GPLv2-
compatible), extracted the raw bytes, and generated
`src/font_vga.h` — `static const unsigned char font_vga_8x16[4096]` (256 chars ×
16 rows × 8 px, MSB = leftmost pixel). Verified the 'A' glyph by eye.

### Source scaffold written

All of `src/` was created this phase: `head.S`, `ofw.c`/`ofw.h`,
`display.c`/`display.h`, `font_vga.h`, `memtest.h`, `test.c`, `main.c`,
`linker.ld`, plus the `Makefile`.

- `test.c` is a close port of the original `test.c`: same function skeletons
  (`addr_tst1`, `addr_tst2`, `movinv1`, `movinv32`, `movinvr`, `modtst`,
  `block_move`, `bit_fade_fill`/`chk`), with x86 inline asm replaced by portable
  C and the `cpu` parameter dropped (single-CPU). Marsaglia MWC RNG.
- `linker.ld`: ELF32 big-endian PowerPC, entry `_start`, **load address
  0x00400000 (4MB)** — leaving the low memory for OF.

### Build fixes to reach first successful compile

- **head.S immediate overflow:** `addi r1,r1,-0x10000` exceeds the 16-bit signed
  immediate. Switched the stack setup to `lis r1, 0x003F` (stack at 0x003F0000).
- **`@ha`/`@l` relocations with named registers fail** unless the assembler gets
  `-Wa,-mregnames`. Without it, `lis r3, foo@ha` treats `r3` as a *symbol*, not a
  register. (Numeric `lis 3, foo@ha` also works.) Added `-Wa,-mregnames` to
  CFLAGS.
- **CIF pointer storage:** a plain C global `ofw_cif` couldn't be referenced
  PC-relatively from `head.S`. Renamed to `_ofw_cif_store`, defined in head.S's
  `.data`, declared `extern` in C.
- **C89 cleanups:** moved mid-block declarations to block tops; added
  `#include "memtest.h"` to display.c; added `rand_seed`/`rand_next` prototypes;
  forward-declared `check_key()`.
- **`__udivdi3` undefined at link:** 64-bit division in `-nostdlib` freestanding
  code needs libgcc. Added
  `LIBGCC = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)` and link it.

## Files created

- `src/head.S` — PPC entry; saves OF CIF pointer; sets stack (0x003F0000);
  clears BSS; calls `main()`.
- `src/ofw.c` / `src/ofw.h` — OF client interface wrappers.
- `src/display.c` / `src/display.h` — virtual VGA text buffer + framebuffer font
  renderer; the ported memtest86+ print functions.
- `src/font_vga.h` — IBM VGA 8x16 font (4096 bytes).
- `src/memtest.h` — core defs/structs/layout constants, PPC inline-asm helpers.
- `src/test.c` — the memory test algorithms, ported to portable C.
- `src/main.c` — entry, PVR-based CPU id, OF memory discovery, `draw_screen()`
  TUI layout, the test scheduling loop.
- `src/linker.ld` — ELF32-PPC, load 0x00400000.
- `Makefile` — cross-compile, `-Wa,-mregnames`, libgcc link.

## Build & test environment

- **Cross-compiler / build host:** opti7050 (`ssh opti7050`), Ubuntu 22.10,
  `powerpc-linux-gnu-gcc` 12.2.0. (opti9020 was the first choice but went
  offline; it has a newer toolchain if ever needed again.)
- **CFLAGS:** `-Wall -O1 -ffreestanding -fno-builtin -nostdlib -mcpu=750
  -mbig-endian -fno-pic -fno-pie -mno-toc -fno-stack-protector -Wa,-mregnames -I src`
- Native Tiger builds on ibookg32 are **not** supported (Mach-O vs ELF).

## Current state

Compiles cleanly to a **~26KB ELF32 MSB PowerPC** binary. **It does not boot
yet.** QEMU's `-kernel` path jumps to 0x01000000 and immediately takes an ISI
(Instruction Storage Interrupt) — OpenBIOS runs with the MMU on and never maps
the kernel's address. Resolving that is the entire subject of session 002.

## What to do next

1. **Get it to actually boot** — figure out why `-kernel` ISI-faults and find a
   boot path that respects OF's memory mapping. (→ session 002 solves this via
   CD boot.)
2. Confirm which register OF passes the client-interface pointer in.
3. Once booting, exercise the OF framebuffer / memory / timebase discovery.
