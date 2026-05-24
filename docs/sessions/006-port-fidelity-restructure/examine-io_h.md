# examine: io.h

Wave 6 classification. Upstream: `ref/memtest86+-2.00/io.h` (119 lines).
Classify-only — NOT compiled into the PPC build; `src/` and the Makefile unchanged.

## File & purpose
`io.h` (guard `_ASM_IO_H`) is the x86 **port-I/O** header. Via a tower of token-
pasting macros it generates inline-asm wrappers for the x86 `in`/`out` instruction
family that talk to the 64K I/O-port address space:

- `__OUT1`/`__OUT2`/`__OUT` (lines 33–43) emit the `out`-instruction variants
  (`outb`/`outw`/`outl`, the constant-port `…c` forms, and the slow `…_p` forms).
- `__IN1`/`__IN2`/`__IN` (45–55) emit the matching `in`-instruction variants
  (`inb`/`inw`/`inl` + `…c`/`…_p`).
- `__OUTS` (57–60) emits the `rep outs{b,w,l}` string-output forms.
- The bottom block (62–118) instantiates these for byte/word/long and defines the
  public `outb`/`inb`/`outw`/`inw`/`outl`/`inl` macros, which pick the constant-
  port `…c` variant when `__builtin_constant_p(port) && port < 256` (lines 88–118).
- `SLOW_DOWN_IO` (17–27) is the classic `out %al,$0x80` I/O-delay.

The actual asm is `"in"`/`"out"` with `"a"` (AL/AX/EAX) and `"d"`/`"id"` (DX/port)
constraints — strictly x86 instructions against the x86 port space.

## Disposition: ⛔ N/A on PPC/OF — not ported
PowerPC has **no port-I/O instruction space**: there is no `inb`/`outb`. All device
access on a PPC Mac is memory-mapped I/O and/or mediated by Open Firmware. This is
exactly the header PORTING-GUIDE §4 names as replaced by the OF client interface:
"`ofw.c`/`ofw.h` … This is the `io.h`/BIOS replacement."

## What already-ported substrate replaces it
- **`ofw.{c,h}`** (the OF client interface: finddevice/getprop/claim/open/read/
  write/exit + framebuffer info + timebase) is the platform-I/O substitute. Where
  upstream poked a hardware port, the port either has no PPC analog or is replaced
  by an `ofw_*` call. `lib.c:23–34`, `init.c:36`, `error.c:28`, and `test.c:34` all
  document the swap of `io.h`/`<sys/io.h>` for `ofw.h`.
- The historical consumers of port I/O are all commented/dead:
  - `lib.c` `get_key()` (PS/2 controller, `inb 0x64`/`inb 0x60`) — inside `#if 0`
    (`lib.c:594–617`); replaced by `check_input()`'s OF stdin poll.
  - `lib.c` `put_lp()` (parallel-port line printer, `inb`/`outb`) — inside an
    `#ifdef LP` that is never defined (`lib.c:1296–1317`).
  - `init.c` `cpuspeed()` (PIT gate, `outb`/`inb` 0x61/0x42/0x43) — inside `#if 0`
    (`init.c:739–755`); CPU clock now from the OF CPU node.
  - `test.c` beep() (PIT speaker, `outb_p`/`inb_p` 0x61/0x42/0x43) — inside a `/* */`
    comment block ending `test.c:1709`.
  - `test.h:124–126` the `getCx86(reg)` Cyrix `outb 0x22 / inb 0x23` macro —
    commented out, only caller was init.c's commented cache code.

## Live callers in src/: none
- `io.h` is **not `#include`d** in `src/` — every occurrence is a commented include
  (`lib.c:28`, `init.c:38`) or a comment explaining the replacement.
- Every `inb`/`outb`/`inw`/`outw`/`inl`/`outl`/`inb_p`/`outb_p` token in `src/`
  lives inside a `#if 0`, an `#ifdef LP` (never defined), or a `/* */` comment
  (verified at `lib.c:598/614/1302/1310-1314`, `init.c:745-755`,
  `test.c:1694-1707`). Zero live port-I/O.

No build impact. No surprises — fully replaced by `ofw.{c,h}` as the contract
specifies.
