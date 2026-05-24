# examine-controller_c.md — controller.c (+ controller.h)

Wave 6 classification. Read-only: NOT copied into `src/`, NOT compiled/linked.

## File & purpose

`controller.c` (2694 lines) is memtest86+'s **x86 chipset / north-bridge memory
controller driver**. It does three things, all by talking directly to PC chipset
registers:

1. **Identify the memory controller** — `find_controller()` (L2638) reads the
   host bridge's PCI vendor/device ID at bus 0 / dev 0 / fn 0
   (`pci_conf_read(... PCI_VENDOR_ID ...)`, L2645-2646) and matches it against a
   ~120-entry table of x86 chipset PCI IDs, `controllers[]` (L2436-2558:
   `{0x1022,0x7006,"AMD 751",...}`, `{0x8086,0x2570,"Intel i848/i865",...}`, etc.).
   AMD K8/K10 are force-detected from `cpu_id` (L2658-2661).
2. **ECC capability/error reporting** — per-chipset `setup_*`/`poll_*` function
   pairs (e.g. `setup_amd64` L169, `poll_amd64` L257, `poll_i875` L765, `poll_iE7520`
   L1003). Each reads/writes that chipset's ECC status registers over PCI config
   space and/or MSRs, then calls `print_ecc_err(page,offset,...)` (defined in
   error.c). `poll_errors()` (L2672) and `set_ecc_polling()` (L2679) are the public
   entry points; `print_memory_controller()` (L2560) paints the ECC capability
   string on `LINE_CPU+4`.
3. **FSB / DRAM clock + memory-timings detection** — `poll_fsb_*` (e.g.
   `poll_fsb_amd64` L1097, `poll_fsb_i925` L1241) and `poll_timings_*` (e.g.
   `poll_timings_i925` L1878, `poll_timings_p35` L1959, `poll_timings_nf2` L2378)
   compute and print the RAM speed (`print_fsb_info` L119) and the
   CAS-RCD-RP-RAS-RC timing string + channel mode (`print_timings_info` L82). These
   are invoked from `print_memory_controller()` (L2632-2633), gated on F1 not held.

`controller.h` declares only the three public symbols: `find_controller`,
`poll_errors`, `set_ecc_polling`.

## Disposition: ⛔ N/A on PPC/OF (do not port)

Every leaf in this file is x86-PC-platform-specific. There is no PPC analogue
because the facilities it drives don't exist on a PowerPC Mac, and OF — not us —
owns the PCI/device tree (PORTING-GUIDE §4). Confirmed not parked into `src/`.

## What x86 mechanism makes it N/A

- **PCI configuration cycles via `pci_conf_read/pci_conf_write`** — used in
  essentially every function. On x86 these issue CF8h/CFCh port-I/O config cycles
  (see `pci.*`, a separate Wave-6 file). PPC PCI config access is host-bridge- and
  firmware-specific; under OF the device tree already enumerates devices, so we'd
  never hand-walk config space this way.
- **`rdmsr`/`wrmsr` inline asm** (macros L22-30; used in `setup_amd64` L197/198,
  `getP4PMmultiplier` L1064, all `poll_fsb_*`). Model-Specific Registers are a
  pure x86 construct — no PPC equivalent.
- **x86 chipset PCI vendor/device IDs and register maps** — the entire
  `controllers[]` table (L2436-2558) and every `setup_*`/`poll_*` body encode
  Intel/AMD/VIA/SiS/nVidia north-bridge register offsets (0x44, 0x4C, 0xC8, MMR
  bases at `dev0+0x120`, etc.). None of these chipsets exist on a Mac; the host
  bridge on G3/G4/G5 is Apple UniNorth/U3/etc.
- **Fixed MMR pointers via raw physical addresses** — e.g. `ptr=(long*)(dev6+0x68)`
  (L697, L1840), `ambase = 0xFE000000` (L2046). These dereference PC physical
  addresses; with OF's MMU on they would fault (CLAUDE.md invariant #4).
- **`extclock` / `cpu_id`** assumptions (L19-20) — `getP4PMmultiplier` (L1064) and
  the FSB math derive the DRAM clock from an x86 bus-clock × CPU multiplier read
  from MSRs. PPC clock discovery is via PVR/OF/timebase, a different model.

## Features surfaced or shelved

All **shelved** (not reproduced on PPC for now). These are real TUI elements in
memtest86+ that we lose:

- **Memory controller / chipset name** on `LINE_CPU+4` (e.g. "Intel i875P") —
  `print_memory_controller()` L2572-2574. On PPC the equivalent (UniNorth/U3 from
  the OF device tree) is a *future* PPC-native nicety, not an x86 port.
- **ECC capability/mode string** ("ECC : Detect / Correct", "Chipkill", "Scrub")
  L2584-2624, plus the `on/ON/off` indicator at `COL_ECC` from `set_ecc_polling()`
  L2687-2690. Mac memory controllers generally aren't ECC; shelved.
- **FSB and RAM clock in MHz + DDR rating** — `print_fsb_info()` L119-140.
- **DRAM timings** "CAS : 2.5-3-3-8 / DDR-2 (Dual)" and channel mode (Single/Dual,
  64/128-bit) — `print_timings_info()` L82-117 and the `poll_timings_*` family.

Conscious call: these are genuinely nice data points, but every one is sourced
from x86 chipset registers we cannot read. A future PPC wave wanting a chipset
line would write a *new* OF-device-tree reader, not port any of this. Note the
TUI lines they target (`LINE_CPU+4`, `LINE_CPU+5`) will simply stay blank — no
layout breakage, since init.c already doesn't call `find_controller()`.

## Live callers in the ported `src/` tree — confirmed none

- `init.c:285` — `/* find_controller(); */` commented out (with rationale at
  L281-284). `cacheable()` (the other rdtsc-gated x86 leaf) likewise commented
  (L295, def at L688).
- `error.c:684` — `/* poll_errors(); */` commented in `do_tick`; its consumer
  `print_ecc_err()` is `#if 0`'d (L437-439 note "its only caller is poll_errors()").
- `error.c:48` and `test.c:49` keep a bare `void poll_errors();` *prototype* but
  there is no live call anywhere (grep for `poll_errors()` finds only comments and
  the commented call). `set_ecc_polling()` has no caller at all.

So controller.c has **zero live references** in the build. Nothing to stub.

## Open questions / TO VERIFY

None material. The file is unambiguously x86-platform glue. The only judgment is
the shelving of the chipset/FSB/timings TUI fields, which is a deliberate
deferral, not an unknown.

## Build impact

None. Not in `src/`, not in the Makefile, no live callers. Pure reference.
