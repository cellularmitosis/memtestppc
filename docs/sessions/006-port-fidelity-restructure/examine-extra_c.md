# examine-extra_c.md — `extra.c` (1012) + `extra.h` (27)

Wave-6 classification. Read in full; cited by line. Classify-only — **not**
compiled into the PPC build, `src/` untouched.

## File & purpose

The memtest.org "extra stuff" (header credits Eric Nelson & Wee, lines 1–2):
an interactive **DRAM-timing / overclocking editor**. It is NOT CPU temperature
and NOT coreboot — it is a live memory-controller register editor that lets the
user change tCAS/tRCD/tRP/tRAS (and AMD64 sub-timings) on the running chipset,
then `restart()`s the test. Everything funnels through `pci_conf_read/write`
(`#include "pci.h"`, line 12).

- `mem_ctr[]` (25–40): hard-coded **x86 chipset table** keyed by PCI
  vendor/device ID — AMD64 HyperTransport `0x1022/0x1100`, nForce2 `0x10de/01E0`,
  Intel i875/i865/i915/i925/Lakeport/i852 (`0x8086/...`), each mapped to a
  `change_timing_*` writer.
- `find_memctr` (55–84): reads PCI 0/0/0 vendor+device IDs and 0/24/0 (the AMD64
  northbridge) via `pci_conf_read`, matches against `mem_ctr[]`, sets `ctrl`.
- `a64_parameter` (86–110): reads AMD64 DRAM timing-low registers
  (`pci_conf_read(0,24,2,0x88/0x8C/0x90,...)`) and unpacks bitfields.
- `change_timing` (114–124) + per-chipset writers
  `change_timing_i852/i925/Lakeport/i875/nf2/amd64` (573–905) and
  `amd64_tweak` (921–1011): the actual register pokes — `pci_conf_read`,
  bit-twiddle the timing fields, `pci_conf_write` back, `__delay(500)`,
  `restart()`. Some (i925/i875) also poke **MMIO BARs directly** via a raw
  `long *ptr` into the controller's mapped registers (e.g. 624–625, 687,
  741–743, 780) — bare physical-address writes.
- UI: `get_menu` (427–485), `get_option`/`get_option_1` (285–424),
  `amd64_option` (126–283), `get_cas` (487–538), `disclaimer` (540–567) — drive
  the popup via `cprint/dprint/getval/get_key/wait_keyup/popclear` and the
  `POP_X/POP_Y` popup constants. (`disclaimer`'s "delay" is a 500000-iteration
  busy `while` loop, 556–563 — janky but portable.)
- `__delay` (908–919): **inline x86 asm** (`jmp/decl/jns` loop) — explicitly an
  x86 leaf.

`extra.h` (27): just the prototypes for the above.

## Disposition: ⛔ N/A — parked, not ported

Two hard x86 dependencies make this unportable as a faithful feature:
1. **Entirely PCI-config driven** — every timing read/write is `pci_conf_*`
   (x86 port I/O 0xCF8/0xCFC; see `examine-pci_c.md`). N/A on PPC/OF.
2. **Chipset-specific register layouts** — `mem_ctr[]` and every
   `change_timing_*` encode *Intel/AMD/nVidia* DRAM-controller register maps
   (PCI dev 0/fn1, AMD64 dev 24/fn2, MMRBAR offsets). No PPC Mac uses these
   chipsets (Apple UniNorth/U3/Intrepid memory controllers are entirely
   different), so even with OF PCI access the bit-poking is meaningless.

Also x86-leaf: the `__delay` inline asm (911–918). The whole file is therefore an
overclocking tool with no PPC analogue. Park verbatim under `ref/`; do not bring
into `src/`.

## x86 mechanism cited
- PCI config port I/O via `pci_conf_read/write` throughout (63–66, 91–105,
  500, 578, 611, 621, 698, 739, 791–842, 852–900, 926–1008).
- Hard-coded Intel/AMD/nVidia PCI vendor:device IDs (28–39).
- Raw MMIO BAR writes through `long *ptr` (503, 624–625/687, 701–702/728,
  741–743/780).
- Inline x86 assembly `__delay` (911–918).

## Live callers in ported `src/` — NONE
`grep -rni 'extra\|get_menu\|overclock\|change_timing' src/` → nothing. The popup
is reached upstream from a footer keybinding in `main.c`/`config.c`; that hook is
not wired in the ported tree. No live reference anywhere in `src/`.

## Features worth surfacing later — deliberately shelved
DRAM-timing editing is a power-user x86 overclocking feature, **out of scope** for
a faithful PPC memory tester and arguably dangerous. Recommend dropping the menu
entry entirely (don't even surface the key). No PPC equivalent is worth building.

## Open questions / TO VERIFY
- None on disposition. The only conscious call is UI: if `config.c`/`main.c`
  footer ever references the "extra options" hotkey, ensure that entry is
  removed/greyed (it currently can't be — file isn't compiled in).

## Build impact
None. Not added to `src/` or the Makefile; no new/changed call sites.
